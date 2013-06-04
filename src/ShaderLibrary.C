/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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
#include <QDir>
#include <QFileInfo>
#include <cstdlib>


using namespace qglviewer;

namespace IQmol {

const QString ShaderLibrary::NoShader = "None";
QString ShaderLibrary::s_currentShader;
QVariantMap ShaderLibrary::s_currentMaterial = QVariantMap();

ShaderLibrary* ShaderLibrary::s_instance = 0;
QMap<QString, unsigned> ShaderLibrary::s_shaders = QMap<QString, unsigned>();


ShaderLibrary& ShaderLibrary::instance()
{
   if (!s_instance) {
      s_instance = new ShaderLibrary();
      s_instance->init();
      s_instance->loadAllShaders();
      s_instance->loadPreferences();
      atexit(ShaderLibrary::destroy);
   }
   return *s_instance;
}


void ShaderLibrary::init()
{
   const GLubyte* vendor   = glGetString(GL_VENDOR);
   const GLubyte* renderer = glGetString(GL_RENDERER);
   const GLubyte* version  = glGetString(GL_VERSION);
   const GLubyte* shading  = glGetString(GL_SHADING_LANGUAGE_VERSION);

   QLOG_INFO() << "OpenGL version " << QString((char*)version);
   QLOG_INFO() << "Vendor   " << QString((char*)vendor);
   QLOG_INFO() << "Renderer " << QString((char*)renderer);
   QLOG_INFO() << "GLSL     " << QString((char*)shading);
}


void ShaderLibrary::destroy()
{
   QMap<QString, unsigned>::iterator iter;
   for (iter = s_shaders.begin(); iter != s_shaders.end(); ++iter) {
       glDeleteProgram(iter.value());
   }
}


bool ShaderLibrary::install(QString const& shader)
{
   bool okay(false);
   if (s_shaders.contains(shader)) {
      QLOG_DEBUG() << "Installing shader " << shader;
      glUseProgram(s_shaders.value(shader));
      s_currentShader = shader;
      okay = true;
   }
   return okay;
}


bool ShaderLibrary::uninstall()
{
   glUseProgram(0);
   return true;
}


void ShaderLibrary::loadPreferences()
{
    QString defaultShader(Preferences::DefaultShader());
    if (defaultShader.isEmpty()) defaultShader = NoShader;
    QVariantMap defaultShaderParameters(Preferences::DefaultShaderParameters());
    setUniformVariables(defaultShader, defaultShaderParameters);
    install(defaultShader);
}


void ShaderLibrary::loadAllShaders()
{
   s_shaders.insert(NoShader, 0);
   QVariantMap defaultParameters;
   defaultParameters.insert("Shininess", QVariant(0.5));
   defaultParameters.insert("Highlights", QVariant(0.5));
   setUniformVariables(NoShader, defaultParameters);

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
                s_shaders.insert(name, program);
                setUniformVariables(name, parseUniformVariables(vertex.filePath()));
             }
          }
       }
   }

   QLOG_INFO() << s_shaders.size() << " Shaders available";
}


QVariantMap ShaderLibrary::uniformVariableList(QString const& shaderName)
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

          if (!name.startsWith("gl_")) {
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
                   qDebug() << "Unknown GL_Type" << type;
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

   // Check the status of the compile/link
   unsigned buflen(1000);
   GLsizei msgLength;
   char msg[buflen];
   glGetProgramInfoLog(program, buflen, &msgLength, msg);

   if (msgLength != 0) {
      QLOG_WARN() << "Failed to compile GLSL program";
      QLOG_WARN() << QString(msg);

qDebug() <<  "Failed to compile GLSL program";
qDebug() << QString(msg);

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
         qDebug() << "Failed to compile shader " << path;
         qDebug() << QString(msg);
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
QVariantMap ShaderLibrary::parseUniformVariables(QString const& vertexShaderPath)
{
   QVariantMap map;

   QFile file(vertexShaderPath);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      Parser2::TextStream textStream(&file);

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

            tokens = Parser2::TextStream::tokenize(line);
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
                  qDebug() << "Unknown uniform variable in shader" << tokens[1];
               }
            }
         }
      }

      file.close();
   }   
   qDebug() << "Parsed the following uniform variables:";
   for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
        qDebug() << iter.key() << iter.value();
   }
   return map;
}


bool ShaderLibrary::setMaterialParameters(QVariantMap const& map) 
{
   double specular = map.contains("Shininess") ? map.value("Shininess").toDouble() : 0.9;
   GLfloat mat_specular[] = {specular, specular, specular, 1.0};
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

   double shininess = map.contains("Highlights") ? map.value("Highlights").toDouble() : 0.5;
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0*(1.0-shininess));
   s_currentMaterial = map;

   return true;
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
             qDebug() << "Unsupported QVariant type in ShaderLibrary";
             break;
       }
   }

   return true;
}


template <class T>
bool ShaderLibrary::setUniformVariable(QString const& shaderName, QString const& variable,
   T const& value)
{
   if (!s_shaders.contains(shaderName)) return false;
   unsigned program(s_shaders.value(shaderName));
   glUseProgram(program);

   QByteArray raw(variable.toLocal8Bit());
   const char* c_str(raw.data());
   GLint location(glGetUniformLocation(program, c_str));
   if (location < 0) return false;
   setUniformVariable(program, location, value);
   return true;
}


void ShaderLibrary::setUniformVariable(GLuint program, GLint location, bool value)
{
   glUniform1i(location, (GLint)value);
}


void ShaderLibrary::setUniformVariable(GLuint program, GLint location, double value)
{
   glUniform1f(location, (GLfloat)value);
}


void ShaderLibrary::setUniformVariable(GLuint program, GLint location, QColor const& color)
{
   glUniform4f(location, color.redF(), color.greenF(), color.blueF(), 
      color.alphaF()); 
}

} // end namespace IQmol
