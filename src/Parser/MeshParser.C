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

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "MeshParser.h"
#include "Mesh.h"


namespace IQmol {
namespace Parser {

bool Mesh::parseFile(QString const& filePath)
{
   m_filePath = filePath;

   Data::Mesh* mesh(new Data::Mesh);
   Data::OMMesh& data(mesh->data());
   OpenMesh::IO::Options options;

   if (OpenMesh::IO::read_mesh(data, m_filePath.toStdString(), options)) {
      mesh->computeFaceNormals();
      data.update_vertex_normals(); 
      m_dataBank.append(mesh);
   }else {
      delete mesh;
      m_errors.append("Failed to load mesh data from file");
   }

   return m_errors.isEmpty();
}

} } // end namespace IQmol::Parser
