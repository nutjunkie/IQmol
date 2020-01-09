#ifndef IQMOL_LAYER_FILE_H
#define IQMOL_LAYER_FILE_H
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

#include "Layer.h"
#include "File.h"
#include "FileConfigurator.h"


namespace IQmol {
namespace Layer {

   /// Simple Layer that represents a data file (e.g. checkpoint file).
   class File : public Base {

      Q_OBJECT

      public:
         explicit File(Data::File const& file);
         explicit File(QString const& filePath);

         void tail(int const interval = 2000);
         QString fileName() const { return m_filePath; }
         QString contents() const;
        
      private:
         QString m_filePath;
         Configurator::File m_configurator;
   };


   class Files : public Base {

      Q_OBJECT;

      public:
         Files() : Base("Files") { }
         QList<File*> getFiles() { return findLayers<File>(Children); }
   };

} // end namespace Layer

typedef QList<Layer::File*> FileList;

} // end namespace IQmol

#endif
