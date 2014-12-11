/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
         
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software  
  Foundation, either version 3 of the License, or (at your option) any later  
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.
   
********************************************************************************/

#include "ShaderLibrary.h"
#include "Preferences.h"
#include "TextStream.h"
#include "QsLog.h"
#include "GLSLmath.h"
#include <QDir>
#include <QFileInfo>
#include <QGLFramebufferObject>
#include <cstdlib>
#include <time.h>



using namespace qglviewer;

namespace IQmol {


const QString ShaderLibrary::NoShader = "None";
QString ShaderLibrary::s_currentShader;
QVariantMap ShaderLibrary::s_currentMaterial = QVariantMap();

QGLFramebufferObject* ShaderLibrary::s_normalBuffer = 0;
QGLFramebufferObject* ShaderLibrary::s_filterBuffer = 0;

GLuint   ShaderLibrary::s_rotationTextureId   = 0;
GLuint   ShaderLibrary::s_rotationTextureSize = 64;
GLfloat* ShaderLibrary::s_rotationTextureData = 0;



ShaderLibrary* ShaderLibrary::s_instance = 0;
QMap<QString, unsigned> ShaderLibrary::s_shaders = QMap<QString, unsigned>();


ShaderLibrary& ShaderLibrary::instance()
{
   if (!s_instance) {
      s_instance = new ShaderLibrary();
      s_instance->init();
      s_instance->loadAllShaders();
      s_instance->loadPreferences();
      s_instance->setFilterVariables(QVariantMap()); 
      atexit(ShaderLibrary::destroy);
   }
   return *s_instance;
}


void ShaderLibrary::init()
{
   const GLubyte* vendor   = glGetString(GL_VENDOR);
   const GLubyte* renderer = glGetString(GL_RENDERER);
   const GLubyte* version  = glGetString(GL_VERSION);

   QLOG_INFO() << "OpenGL version " << QString((char*)version);
   QLOG_INFO() << "Vendor   " << QString((char*)vendor);
   QLOG_INFO() << "Renderer " << QString((char*)renderer);

#ifdef IQMOL_SHADERS
   const GLubyte* shading  = glGetString(GL_SHADING_LANGUAGE_VERSION);
   QLOG_INFO() << "GLSL     " << QString((char*)shading);
#endif
}


bool ShaderLibrary::setMaterialParameters(QVariantMap const& map) 
{
   double specular = map.contains("Shininess") ? 
       map.value("Shininess").toDouble() : 0.9;

   GLfloat mat_specular[] = {specular, specular, specular, 1.0};
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

   double shininess = map.contains("Highlights") ? 
       map.value("Highlights").toDouble() : 0.5;

   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0*(1.0-shininess));
   s_currentMaterial = map;

   return true;
}


bool ShaderLibrary::bindShader(QString const& shader)
{
   bool okay(false);
   if (s_shaders.contains(shader)) {
#ifdef IQMOL_SHADERS
      glUseProgram(s_shaders.value(shader));
#endif
      s_currentShader = shader;
      okay = true;
   }
   return okay;
}


void ShaderLibrary::loadAllShaders()
{
   s_shaders.insert(NoShader, 0);
   QVariantMap defaultParameters;
   defaultParameters.insert("Shininess", QVariant(0.5));
   defaultParameters.insert("Highlights", QVariant(0.5));
   setUniformVariables(NoShader, defaultParameters);

#ifdef IQMOL_SHADERS
   QDir dir(Preferences::ShaderDirectory());
   if (!dir.exists() || !dir.isReadable()) {
      QLOG_WARN() << "Could not access shader directory: " + dir.path();
      return;
   }

   QDir::Filters filters(QDir::Files | QDir::Readable);
   QStringList contents(dir.entryList(filters));

   unsigned program;
   QStringList::iterator iter;
   for (iter = contents.begin(); iter != contents.end(); ++iter) {
       QFileInfo vertex(dir, *iter);
       if (vertex.suffix().contains("vert", Qt::CaseInsensitive)) {
          QFileInfo fragment(dir, vertex.completeBaseName() + ".frag"); 
          if (fragment.exists()) {
             QString name(fragment.completeBaseName());
             name = name.replace("_", " ");
             program = createProgram(vertex.filePath(), fragment.filePath());          
             if (program > 0) {
                m_shadersInitialized = true;
                QLOG_DEBUG() << "Shader compilation successful:" << name;
                s_shaders.insert(name, program);
                QVariantMap uniforms(parseUniformVariables(vertex.filePath()));
                uniforms.unite(parseUniformVariables(fragment.filePath()));
                setUniformVariables(name, uniforms);
             }
          }
       }
   }
#endif
}


void ShaderLibrary::loadPreferences()
{
    QString defaultShader(Preferences::DefaultShader());
    if (defaultShader.isEmpty()) defaultShader = NoShader;
    QVariantMap defaultShaderParameters(Preferences::DefaultShaderParameters());
    setUniformVariables(defaultShader, defaultShaderParameters);
    bindShader(defaultShader);
}




#ifndef IQMOL_SHADERS

// Define stubs
void ShaderLibrary::destroy() { }

bool ShaderLibrary::suspend() { return true; }
bool ShaderLibrary::resume() { return true; }

void ShaderLibrary::bindNormalMap(GLfloat near0, GLfloat far0) { }
void ShaderLibrary::releaseNormalMap() { }
void ShaderLibrary::generateFilters() { }
void ShaderLibrary::bindTextures(QString const& shader) { }
void ShaderLibrary::releaseTextures() { }
void ShaderLibrary::clearFrameBuffers() { }

void ShaderLibrary::resizeScreenBuffers(QSize const&, double*) { }

bool ShaderLibrary::setUniformVariables(QString const&, QVariantMap const& map) 
{
   return setMaterialParameters(map);
}


void ShaderLibrary::setFilterVariables(QVariantMap const& map) { }

QVariantMap ShaderLibrary::uniformUserVariableList(QString const&)
{
   return QVariantMap();
}


#else  // IQMOL_SHADERS


void ShaderLibrary::destroy()
{
   if (s_rotationTextureId) glDeleteTextures(1, &s_rotationTextureId);

   QMap<QString, unsigned>::iterator iter;
   for (iter = s_shaders.begin(); iter != s_shaders.end(); ++iter) {
       if (iter.value() != 0) glDeleteProgram(iter.value());
   }

   // These seem to cause a crash 
   //if (s_normalBuffer) delete s_normalBuffer;
   //if (s_filterBuffer) delete s_filterBuffer;

}


ShaderLibrary::~ShaderLibrary()
{
   delete s_rotationTextureData;
}



bool ShaderLibrary::resume()
{
   return bindShader(s_currentShader);
}


bool ShaderLibrary::suspend()
{
   glUseProgram(0);
   return true;
}





QVariantMap ShaderLibrary::uniformUserVariableList(QString const& shaderName)
{
   if (shaderName == NoShader) return s_currentMaterial;
   QVariantMap map;

   if (s_shaders.contains(shaderName)) {
      unsigned program(s_shaders.value(shaderName));

      int total(0);
      glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &total); 

      int length, number;
      char buf[100];
      GLenum type;
      GLfloat values[4];
      GLint boolean;
      GLuint location;
      QColor color;

      for (int i = 0; i < total; ++i)  {
          glGetActiveUniform(program, GLuint(i), sizeof(buf)-1, &length, &number, &type, buf);
          buf[length] = 0;
          QString name(buf);

          if (name.startsWith("user_")) {
             location = glGetUniformLocation(program, buf);

             switch (type) {
                case GL_FLOAT:
                   glGetUniformfv(program, location, values);
                   map.insert(name, values[0]);
                   break;

                case GL_BOOL:
                   glGetUniformiv(program, location, &boolean);
                   map.insert(name, (boolean == 1));
                   break;
                   
                case GL_FLOAT_VEC4:
                   glGetUniformfv(program, location, values);
                   color.setRgbF(values[0], values[1], values[2], values[3]);
                   map.insert(name, color);
                   break;

                default:
                   // Assume anything else is installed by the Viewer
                   QLOG_DEBUG() << "Unknown GL_Type" << type;
                   break;
             }
         }
      }
   }

   return map;
}


unsigned ShaderLibrary::createProgram(QString const& vertexPath, QString const& fragmentPath)
{
   unsigned vertexShader(loadShader(vertexPath, GL_VERTEX_SHADER));
   if (vertexShader == 0) return 0;

   unsigned fragmentShader(loadShader(fragmentPath, GL_FRAGMENT_SHADER));
   if (fragmentShader == 0) {
      glDeleteShader(vertexShader);
      return 0;
   }

   unsigned program(glCreateProgram());
   glAttachShader(program, vertexShader);
   glAttachShader(program, fragmentShader);

   glLinkProgram(program);
   glValidateProgram(program);

   // Check for any errors in the compile/link
   GLint status;
   glGetProgramiv(program, GL_VALIDATE_STATUS, &status); 

   if (status ==  GL_FALSE) {
      unsigned buflen(1000);
      GLsizei msgLength;
      char msg[buflen];
      glGetProgramInfoLog(program, buflen, &msgLength, msg);

      QLOG_WARN() << "Failed to compile GLSL program";
      QLOG_WARN() << QString(msg);

      glDeleteProgram(program);
      program = 0;
   }
 
   return program;
}


unsigned ShaderLibrary::loadShader(QString const& path, unsigned const mode)
{
   unsigned shader(0);
   QFile file(path);

   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString contents(file.readAll()); 
      file.close();

      QByteArray raw(contents.toLocal8Bit());
      const char* c_str(raw.data());

      shader = glCreateShader(mode);
      glShaderSource(shader, 1, &c_str, NULL);
      glCompileShader(shader);

      // Check if things compiled okay
      unsigned buflen(1000);
      GLsizei msgLength;
      char msg[buflen];
      glGetShaderInfoLog(shader, buflen, &msgLength, msg);

      if (msgLength != 0) {
         QLOG_WARN() << "Failed to compile shader " << path;
         QLOG_WARN() << QString(msg);
         glDeleteShader(shader);  // required?
         shader = 0;
      }
   }

   return shader;
}


/* This only parses uniform variables between the following lines:

// BEGIN UNIFORM
uniform float x;    // default
uniform vec4 color; // red green blue alpha
// END UNIFORM

uniform can be replaced by 'in' and if no default is given the floats default
to 0.0 and the color to white.  Comments between the BEGIN and END lines are
ignored.
*/
QVariantMap ShaderLibrary::parseUniformVariables(QString const& shaderPath)
{
   QVariantMap map;

   QFile file(shaderPath);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      Parser::TextStream textStream(&file);

      int n;
      bool tf;
      double value;
      QString line;
      QString name;
      QColor  color;
      QStringList tokens;
      textStream.seek("BEGIN UNIFORM");

      while (!textStream.atEnd()) {
         line = textStream.nextLine();      
         if (line.contains("END UNIFORM")) break;

         if (!line.startsWith("//")) {
            line = line.replace(";"," ");
            line = line.replace(","," ");
            line = line.replace("//"," ");

            tokens = Parser::TextStream::tokenize(line);
            n = tokens.size();

            if (n >= 3 && (tokens[0] == "uniform" || tokens[0] == "in"))  {
               name = tokens[2];

               if (tokens[1] == "float") {
                  value = (n >= 4) ? tokens[3].toDouble() : 0.0;
                  map.insert(name, value);
               }else if (tokens[1] == "bool") {
                  tf = (n >= 4) ? tokens[3].toInt() : false;
                  map.insert(name, tf);
               }else if (tokens[1] == "vec4") {
                  color = Qt::white;
                  if (n >= 7) {
                     color.setRgbF(tokens[3].toDouble(), tokens[4].toDouble(), 
                        tokens[5].toDouble(), tokens[6].toDouble());
                  }
                  map.insert(name, color);
               }else {
                  QLOG_DEBUG() << "Unknown uniform variable in shader" << tokens[1];
               }
            }
         }
      }

      file.close();
   }   

   return map;

/*
   qDebug() << "Parsed the following uniform variables:";
   for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
        qDebug() << iter.key() << iter.value();
   }
*/
}



bool ShaderLibrary::setTextureVariable(QString const& shader, QString const& variable, 
   Texture const& texture)
{
   QSizeF size(1.0/texture.size.width(), 1.0/texture.size.height());
   setUniformVariable(shader, variable+"_delta", size);
   setUniformVariable(shader, variable, texture);
   return true;
}


void ShaderLibrary::setFilterVariables(QVariantMap const& map)
{
   if (!filtersAvailable()) return;

   bool aa(map.contains("Antialias") && map.value("Antialias").toBool());
   bool bd(map.contains("Border") && map.value("Border").toBool());
   bool ao(map.contains("AmbientOcclusion") && map.value("AmbientOcclusion").toBool());
   m_filtersActive = aa || bd || ao;

   // Let the shaders know whether or not the Mask texture is valid
   QStringList shaders(availableShaders());
   QStringList::const_iterator iter;
   for (iter = shaders.begin(); iter != shaders.end(); ++iter) {
       setUniformVariable(*iter, "FiltersAreValid", m_filtersActive);
   }

   // Ambient Occlusion parameters
   //if (ao) {
   //   setUniformVariables("Filters", map);
   //}
}
   


bool ShaderLibrary::setUniformVariables(QString const& shaderName, QVariantMap const& map)
{
   // If there is no shader, the parameters affect the material
   if (shaderName == NoShader) return setMaterialParameters(map);

   bool ok;
   double val;
   QColor color;
   QString name;

   for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
       name = iter.key();
       switch (iter.value().type()) {

//        case QVariant::QColor:
//             color = iter.value().value<QColor>();
//             if (!color.isValid() || !setUniformVariable(shaderName, name, color)) {
//                return false;
//             }
//           break;

          case QVariant::Bool:
             ok = iter.value().toBool(); 
             if (!setUniformVariable(shaderName, name, ok)) return false;
             break;

          case QVariant::Double:
             val = iter.value().toDouble(&ok);  
             if (!ok || !setUniformVariable(shaderName, name, val)) return false;
             break;

          default:
             QLOG_DEBUG() << "Unsupported QVariant type in ShaderLibrary";
             break;
       }
   }

   return true;
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, QSizeF const& value)
{
   glUniform2f(location, value.width(), value.height());
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, QSize const& value)
{
   glUniform2i(location, value.width(), value.height());
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, bool value)
{
   glUniform1i(location, (GLint)value);
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, double value)
{
   glUniform1f(location, (GLfloat)value);
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, 
   GLFloatArray const& value)
{
   switch (value.type) {
      case GLfloat1v:
         glUniform1fv(location, value.size, value.ptr);
         break;
      case GLfloat2v:
         glUniform2fv(location, value.size, value.ptr);
         break;
      case GLfloat3v:
         glUniform3fv(location, value.size, value.ptr);
         break;
      case GLfloat4v:
         glUniform4fv(location, value.size, value.ptr);
         break;
   }
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, GLfloat value)
{
   glUniform1f(location, value);
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, QColor const& color)
{
   glUniform4f(location, color.redF(), color.greenF(), color.blueF(), 
      color.alphaF()); 
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, mat4x4 const& value)
{
   mat4x4 tmp(value);
   glUniformMatrix4fv(location, 1, GL_FALSE, &tmp);
}


void ShaderLibrary::setUniformVariable(GLuint /*program*/, GLint location, 
   Texture const& texture)
{
   GLuint w(texture.size.width());
   GLuint h(texture.size.height());

   switch (texture.slot) {
      case NormalBuffer:     glActiveTexture(GL_TEXTURE0);  break;
      case FilterBuffer:     glActiveTexture(GL_TEXTURE1);  break;
      case RotationTexture:  glActiveTexture(GL_TEXTURE2);  break;
   }

   glBindTexture(GL_TEXTURE_2D, texture.id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, texture.data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glUniform1i(location, (GLuint)texture.slot);
}



void ShaderLibrary::resizeScreenBuffers(QSize const& windowSize, double* projectionMatrix)
{
   if (!filtersAvailable()) return;

   if (s_normalBuffer) delete s_normalBuffer;
   if (s_filterBuffer) delete s_filterBuffer;

   s_normalBuffer = new QGLFramebufferObject(windowSize, QGLFramebufferObject::Depth);
   s_filterBuffer = new QGLFramebufferObject(windowSize, QGLFramebufferObject::Depth);

   GLfloat width(s_normalBuffer->size().width());
   GLfloat height(s_normalBuffer->size().height());
   
   mat4x4  projection, projectionInverse, projectionBiasInverse;
   for (int i = 0; i < 15; ++i) { projection[i] = (GLfloat)projectionMatrix[i]; }
   projectionInverse     = PerspectiveProjectionMatrixInverse(projection);
   projectionBiasInverse = projectionInverse * BiasMatrixInverse();

   QSizeF size(width/s_rotationTextureSize, height/s_rotationTextureSize);
   //setUniformVariable("Filters", "Scale_xy", size);
   //setUniformVariable("Filters", "ProjectionInverse", projectionBiasInverse);
}


void ShaderLibrary::initializeTextures()
{
   int nSamples = 16;  // This is set in the shader too

   GLfloat* angles = new GLfloat[2*nSamples];
   GLfloat  angle  = (GLfloat)M_PI_4;
   GLfloat  radius = 0.415f;  // This is the maximum length of the vector

   for (int i = 0; i < nSamples; ++i) {
       angles[2*i+0] = radius * cos(angle) * (i+1.0)/16.0f;
       angles[2*i+1] = radius * sin(angle) * (i+1.0)/16.0f;
       angle += (float)M_PI_2;
       if (((i + 1) % 4) == 0) angle += 0.375*M_PI;
   }

   GLFloatArray array;
   array.type = GLfloat2v;
   array.size = nSamples;
   array.ptr  = angles;

   //setUniformVariable("Filters", "SamplingVectors", array);
   delete angles;


   // generate a 64 x 64 rotation texture for rotating the sampling vectors
   srand(time(0));
   int n(s_rotationTextureSize);
   s_rotationTextureData = new GLfloat[4*n*n];
   angle = (2.0*M_PI) * rand() / RAND_MAX;

   for (int i = 0; i < n*n; ++i) {
       s_rotationTextureData[4*i+0] =  0.5f*cos(angle) + 0.5f;
       s_rotationTextureData[4*i+1] =  0.5f*sin(angle) + 0.5f;
       s_rotationTextureData[4*i+2] = -0.5f*sin(angle) + 0.5f;
       s_rotationTextureData[4*i+3] =  0.5f*cos(angle) + 0.5f;
       angle += (2.0*M_PI) * rand() / RAND_MAX;
   }

   for (int i = 0; i < n*n; ++i) {
       s_rotationTextureData[4*i+0] =  1.0;
       s_rotationTextureData[4*i+1] =  0.0;
       s_rotationTextureData[4*i+2] =  0.0;
       s_rotationTextureData[4*i+3] =  1.0;
   }

   glGenTextures(1, &s_rotationTextureId);

   Texture texture;
   texture.id   = s_rotationTextureId;
   texture.size = QSize(n, n);
   texture.slot = RotationTexture;
   texture.data = s_rotationTextureData;
   //setTextureVariable("Filters", "RotationTexture", texture);
}


void ShaderLibrary::bindNormalMap(GLfloat near0, GLfloat far0)
{
   bindShader("Normal Map");
   s_normalBuffer->bind();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   setUniformVariable("Normal Map", "Near", near0);
   setUniformVariable("Normal Map", "Far", far0);
}


void ShaderLibrary::releaseNormalMap()
{
   s_normalBuffer->release();
}


void ShaderLibrary::generateFilters()
{
   GLuint texId(s_normalBuffer->texture()); 
   QSize  size(s_normalBuffer->size()); 

   Texture texture;
   texture.id   = texId;
   texture.size = size;
   texture.slot = NormalBuffer;
   texture.data = 0;
 
   //setTextureVariable("Filters", "NormalMap", texture);

   s_filterBuffer->bind();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //bindShader("Filters");



 
   texture.id   = s_normalBuffer->texture(); 
   texture.size = size;
   texture.slot = (Texture_t)0;
   texture.data = 0;
   //setTextureVariable("Filters", "NormalMap", texture);

   texture.id   = s_rotationTextureId;
   texture.size = QSize(s_rotationTextureSize, s_rotationTextureSize);
   texture.slot = (Texture_t)1;
   texture.data = s_rotationTextureData;
   setTextureVariable("Filters", "RotationTexture", texture);



   //bindTextures("Filters");
/*
   glActiveTexture(GL_TEXTURE0);  
   glBindTexture(GL_TEXTURE_2D, texId);

   glActiveTexture(GL_TEXTURE2);  
   glBindTexture(GL_TEXTURE_2D, s_rotationTextureId);
*/


   float w(size.width());  float h(size.height());  float z(0.0);
  
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(z, w, z, h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glBegin(GL_QUADS);
      glTexCoord2f(1.0f,0.0f);  glVertex3f(w, z, z);
      glTexCoord2f(0.0f,0.0f);  glVertex3f(z, z, z);
      glTexCoord2f(0.0f,1.0f);  glVertex3f(z, h, z);
      glTexCoord2f(1.0f,1.0f);  glVertex3f(w, h, z);
   glEnd();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   s_filterBuffer->release();
   releaseTextures();
}


void ShaderLibrary::bindTextures(QString const& shader)
{
   if (!filtersAvailable()) return;
   GLuint normalId(s_normalBuffer->texture()); 
   GLuint filterId(s_filterBuffer->texture()); 
   QSize  size(s_normalBuffer->size()); 

   Texture texture;
 
   texture.id   = normalId;
   texture.size = size;
   texture.slot = NormalBuffer;
   texture.data = 0;
   setTextureVariable(shader, "NormalMap", texture);

   texture.id   = filterId;
   texture.size = size;
   texture.slot = FilterBuffer;
   texture.data = 0;
   setTextureVariable(shader, "FilterMap", texture);

   texture.id   = s_rotationTextureId;
   texture.size = QSize(s_rotationTextureSize, s_rotationTextureSize);
   texture.slot = RotationTexture;
   texture.data = s_rotationTextureData;
   setTextureVariable(shader, "RotationTexture", texture);
}


void ShaderLibrary::releaseTextures()
{
   glActiveTexture(GL_TEXTURE0);  
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);

   glActiveTexture(GL_TEXTURE1);  
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);

   glActiveTexture(GL_TEXTURE2);  
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);
}




void ShaderLibrary::clearFrameBuffers()
{
   if (!filtersAvailable()) return;

   s_normalBuffer->bind();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   s_normalBuffer->release();

   s_filterBuffer->bind();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   s_filterBuffer->release();
}

#endif  // IQMOL_SHADERS

} // end namespace IQmol
