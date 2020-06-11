#ifndef IQMOL_PARSER_CUBE_H
#define IQMOL_PARSER_CUBE_H
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

#include "Parser.h"


namespace IQmol {

namespace Data {
  class Geometry;
}

namespace Parser {

   /// Parses a cube data file.  I couldn't find an official standard for the
   /// file format, so I have worked off http://paulbourke.net/dataformats/cube.  
   /// 
   /// Assumptions:
   ///   - only one grid per file is supported
   ///   - if the number of voxels is positive, units are in bohr
   ///   - if the number of voxels is negative, units are in angstrom
   ///   - units cannot be mixed
   ///   - voxels can have different lengths along each dimension
   ///   - voxel axes must be aligned with the cartesian axes
   ///   - data loops in the order x, y, z, so the z data varies fastest
   ///   - all data is separated by whitespace
   /// 
   /// https://h5cube-spec.readthedocs.io/en/latest/cubeformat.html

   class Cube : public Base {

      public:
         bool parse(TextStream&);
         bool save(QString const& filePath, Data::Bank&);

      private:
         int parseGridAxes(TextStream& textStream);
		 bool parseCoordinates(TextStream& textStream, unsigned nAtoms);
         void parseGridData(TextStream& textStream);
         QStringList formatCoordinates(Data::Geometry const&);

         int m_nx, m_ny, m_nz;
         double m_scale;
         qglviewer::Vec m_origin;
         qglviewer::Vec m_delta;
   };

} } // end namespace IQmol::Parser

#endif
