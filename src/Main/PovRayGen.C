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

#include "PovRayGen.h"
#include "QsLog.h"
#include "Numerical.h"
#include <QtDebug>


using namespace qglviewer;

namespace IQmol {

PovRayGen::PovRayGen() 
{
   qDebug() << "Generating PovRay file";
   QString fileName("iqmol.pov");
   m_file.setFileName(fileName);

   if (m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      m_stream.setDevice(&m_file);
      printHeader();
      printAtomMacro();
   }else {
      QLOG_WARN() << "Unable to open PovRay file" << fileName;
   }
}


void PovRayGen::printHeader()
{
   m_stream << "//--------------------------------------------------\n";
   m_stream << "// POV-Ray 3.6/3.7 Scene File generated using IQmol\n";
   m_stream << "//--------------------------------------------------\n";

   m_stream << "\n";
   m_stream << "//------------------------------------------\n";
   m_stream << "#version 3.6; // 3.7\n";
   m_stream << "#include \"colors.inc\"\n";
   m_stream << "#include \"textures.inc\"\n";
   m_stream << "//------------------------------------------\n";
   m_stream << "\n\n";
   m_stream << "global_settings {\n";
   m_stream << "   assumed_gamma 3.0\n";
   m_stream << "}\n\n";
}


void PovRayGen::printAtomMacro()
{
   m_stream << "#macro Atom(pos, col, rad)\n";
   m_stream << "sphere {\n";
   m_stream << "   pos, rad  // take radius from configuration\n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color rgbt col \n";
   m_stream << "      }   \n";
   m_stream << "      finish {\n";
   m_stream << "         phong      0.7 \n";
   m_stream << "         ambient    0.6 \n";
   m_stream << "         diffuse    0.75\n";
   m_stream << "         specular   0.5 \n";
   m_stream << "         reflection 0.3 \n";
   m_stream << "      }   \n";
   m_stream << "  }   \n";
   m_stream << "}\n";
   m_stream << "#end\n\n";
/*
      normal {
         bumps 0.4  // controls depth of bumps
         scale .05  // controls width of bumps
      }
*/
}


PovRayGen::~PovRayGen()
{
   m_file.close();
}


void PovRayGen::setCamera(qglviewer::Camera* camera)
{
   Vec pos(camera->position());
   double angle(camera->fieldOfView());

   m_stream << "camera {\n";
   m_stream << "   location <" << pos.x << ", " << pos.y << ", " << pos.z << ">\n";
   m_stream << "   angle " << Util::round(angle*180/M_PI) <<  "\n";
   m_stream << "}\n";
}


} // end namespace IQmol
