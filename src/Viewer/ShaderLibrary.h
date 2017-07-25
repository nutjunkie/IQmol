#ifndef IQMOL_SHADERLIBRARY_H
#define IQMOL_SHADERLIBRARY_H
/*******************************************************************************
         
  Copyright (C) 2011-2015 Andrew Gilbert
      
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

#include <QtGlobal>
#ifdef Q_OS_LINUX
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <QColor>
#include <QSize>
#include <QVariant>
#include <QStringList>
#include "QGLViewer/vec.h"
#include <QDebug>
#include <QGLFunctions>

#ifdef Q_OS_WIN32
#undef IQMOL_SHADERS
#else
#define IQMOL_SHADERS
#endif

#define IQMOL_SHADERS


class QGLFramebufferObject;
class mat4x4;

namespace IQmol {

namespace Data {
   class PovRay;
}

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
   // Note the only supported uniform variable types are float and bool
   class ShaderLibrary {

      public:
         static const QString NoShader;

         ShaderLibrary(QGLContext*);
         ~ShaderLibrary();

         QStringList availableShaders() const { return m_shaders.keys(); }
         QString const& currentShader() { return m_currentShader; }

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

         void setPovRayVariables(QVariantMap const& map) {
            m_povrayVariables = map;
         }
         QVariantMap const& povrayVariables() const { return m_povrayVariables; }

         QStringList povrayTextureNames() const;
         QMap<QString,QString> povrayTextures() const;

         bool filtersAvailable() { return m_filtersAvailable; };
         bool filtersActive() { return m_filtersActive; };
         bool shadersInitialized() const { return m_shadersInitialized; }

         template <class T>
         void broadcast(QString const& variableName, T const& value) {
#ifdef IQMOL_SHADERS
            QByteArray raw(variableName.toLocal8Bit());
            const char* c_str(raw.data());

            QList<unsigned> programs(m_shaders.values());
            for (int i = 0; i < programs.size(); ++i) {
                unsigned program(programs[i]);
                GLint location(m_glFunctions->glGetUniformLocation(program, c_str));
                if (location > 0) setUniformVariable(program, location, value);
            }
#endif
         }

         // This does not filter for NoShader
         template <class T>
         bool setUniformVariable(QString const& shaderName, QString const& variable, T 
            const& value)
         {
#ifdef IQMOL_SHADERS
            if (!m_shaders.contains(shaderName)) {
               qDebug() << "Shader not found:" << shaderName;
               return false;
            }
            unsigned program(m_shaders.value(shaderName));
            m_glFunctions->glUseProgram(program);

            QByteArray raw(variable.toLocal8Bit());
            const char* c_str(raw.data());
            GLint location(m_glFunctions->glGetUniformLocation(program, c_str));
            if (location < 0) {
               qDebug() << "Shader location not found:" << shaderName << variable;
               return false;
            }
            setUniformVariable(program, location, value);
            return true;
#else
            return false;
#endif
         }


      private:
         QGLFramebufferObject* m_normalBuffer;
         QGLFramebufferObject* m_filterBuffer;

         GLuint   m_rotationTextureId;
         GLuint   m_rotationTextureSize;
         GLfloat* m_rotationTextureData;

         QMap<QString, unsigned> m_shaders;
         QString m_currentShader;
         void destroy();

         void initializeTextures();

         bool m_filtersAvailable;
         bool m_filtersActive;
         bool m_shadersInitialized;

         void init();
         void loadPreferences();
         void loadShaders();
         void loadPovRay();
         unsigned createProgram(QString const& vertexPath, QString const& fragmentPath);
         unsigned loadShader(QString const& path, unsigned const mode);

         QVariantMap parseUniformVariables(QString const& vertexShaderPath);

         QVariantMap m_currentMaterial;
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

         explicit ShaderLibrary(ShaderLibrary const&) { }

         QGLFunctions* m_glFunctions;
         QVariantMap m_povrayVariables;

         Data::PovRay* m_povray;
   };


} // end namespace IQmol

#endif
