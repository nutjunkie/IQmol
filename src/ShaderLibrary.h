#ifndef IQMOL_SHADERLIBRARY_H
#define IQMOL_SHADERLIBRARY_H
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

#include <QColor>
#include <QVariant>
#include <QStringList>
#include "QGLViewer/vec.h"


namespace IQmol {

   // ShaderLibrary manages the use of GLSL shaders.
   // Note the only supported uniform variable types are float and 
   class ShaderLibrary {

      public:
         static const QString NoShader;
         static ShaderLibrary& instance();
         QStringList availableShaders() const { return s_shaders.keys(); }

         bool install(QString const& name);
         bool uninstall();

         QVariantMap uniformVariableList(QString const& shaderName);
         bool setUniformVariables(QString const& shaderName, QVariantMap const& map);

         void suspendShader() { uninstall(); }
         void resumeShader() { install(s_currentShader); }

      private:
         static QMap<QString, unsigned> s_shaders;
         static ShaderLibrary* s_instance;
         static QString s_currentShader;
         static void destroy();

         void init();
         void loadAllShaders();
         void loadPreferences();
         unsigned createProgram(QString const& vertexPath, QString const& fragmentPath);
         unsigned loadShader(QString const& path, unsigned const mode);

         QVariantMap parseUniformVariables(QString const& vertexShaderPath);

         static QVariantMap s_currentMaterial;
         bool setMaterialParameters(QVariantMap const& map);
         void setUniformVariable(GLuint program, GLint location, bool);
         void setUniformVariable(GLuint program, GLint location, double);
         void setUniformVariable(GLuint program, GLint location, QColor const&);

         // This does not filter for NoShader
         template <class T>
         bool setUniformVariable(QString const& shaderName, QString const& variable,
            T const& value);

         ShaderLibrary() { }
         explicit ShaderLibrary(ShaderLibrary const&) { }
         ~ShaderLibrary() { }
   };


} // end namespace IQmol

#endif
