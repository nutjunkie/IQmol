#ifndef IQMOL_SHADERLIBRARY_H
#define IQMOL_SHADERLIBRARY_H
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

#include <QColor>
#include <QSize>
#include <QVariant>
#include <QStringList>
#include "QGLViewer/vec.h"
#include <QDebug>
//#include "glloader.h"


class QGLFramebufferObject;
class mat4x4;

namespace IQmol {

   enum  Texture_t { NormalBuffer, FilterBuffer, RotationTexture };

   struct Texture {
      GLuint    id; 
      QSize     size; 
      Texture_t slot;
      GLfloat*  data; 
   };

   enum GLArrayType {GLfloat1v, GLfloat2v, GLfloat3v, GLfloat4v};

   struct GLFloatArray {
      GLArrayType type;
      GLuint      size;
      GLfloat*    ptr; 
   };

   // ShaderLibrary manages the use of GLSL shaders.
   // Note the only supported uniform variable types are float and 
   class ShaderLibrary {

      public:
         static const QString NoShader;
         static ShaderLibrary& instance();

         QStringList availableShaders() const { return s_shaders.keys(); }
         QString const& currentShader() { return s_currentShader; }

         bool bindShader(QString const& name);
         bool suspend();
         bool resume();
         void resizeScreenBuffers(QSize const& windowSize, double* projectionMatrix);

         void bindNormalMap(GLfloat near0, GLfloat far0);
         void releaseNormalMap();
         void generateFilters();
         void bindTextures(QString const& shader);
         void releaseTextures();
         void clearFrameBuffers();
         
         QVariantMap uniformUserVariableList(QString const& shaderName);
         bool setUniformVariables(QString const& shaderName, QVariantMap const& map);
         bool setTextureVariable(QString const& shader, QString const& name, Texture const&);

         void setFilterVariables(QVariantMap const& map);
         void setFiltersAvailable(bool tf) { m_filtersAvailable = tf; };

         bool filtersAvailable() { return m_filtersAvailable; };
         bool filtersActive() { return m_filtersActive; };

         // This does not filter for NoShader
         template <class T>
         bool setUniformVariable(QString const& shaderName, QString const& variable, T 
            const& value)
         {
            if (!s_shaders.contains(shaderName)) {
               qDebug() << "Shader not found:" << shaderName;
               return false;
            }
            unsigned program(s_shaders.value(shaderName));
            glUseProgram(program);

            QByteArray raw(variable.toLocal8Bit());
            const char* c_str(raw.data());
            GLint location(glGetUniformLocation(program, c_str));
            if (location < 0) {
               // qDebug() << "Shader location not found:" << shaderName << variable;
               return false;
            }
            setUniformVariable(program, location, value);
            return true;
         }


      private:
         static QGLFramebufferObject* s_normalBuffer;
         static QGLFramebufferObject* s_filterBuffer;

         static GLuint   s_rotationTextureId;
         static GLuint   s_rotationTextureSize;
         static GLfloat* s_rotationTextureData;

         static QMap<QString, unsigned> s_shaders;
         static ShaderLibrary* s_instance;
         static QString s_currentShader;
         static void destroy();

         void initializeTextures();

         bool m_filtersAvailable;
         bool m_filtersActive;

         void init();
         void loadAllShaders();
         void loadPreferences();
         unsigned createProgram(QString const& vertexPath, QString const& fragmentPath);
         unsigned loadShader(QString const& path, unsigned const mode);

         QVariantMap parseUniformVariables(QString const& vertexShaderPath);

         static QVariantMap s_currentMaterial;
         bool setMaterialParameters(QVariantMap const& map);
         void setUniformVariable(GLuint program, GLint location, QSize const&);
         void setUniformVariable(GLuint program, GLint location, QSizeF const&);
         void setUniformVariable(GLuint program, GLint location, bool);
         void setUniformVariable(GLuint program, GLint location, double);
         void setUniformVariable(GLuint program, GLint location, GLfloat);
         void setUniformVariable(GLuint program, GLint location, QColor const&);
         void setUniformVariable(GLuint program, GLint location, Texture const&);
         void setUniformVariable(GLuint program, GLint location, GLFloatArray const& value);
         void setUniformVariable(GLuint program, GLint location, mat4x4 const& value);

         ShaderLibrary() : m_filtersAvailable(false), m_filtersActive(false) { }
         explicit ShaderLibrary(ShaderLibrary const&) { }
         ~ShaderLibrary();
   };


} // end namespace IQmol

#endif
