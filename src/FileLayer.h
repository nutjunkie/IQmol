#ifndef IQMOL_FILELAYER_H
#define IQMOL_FILELAYER_H
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

#include "DataLayer.h"
#include "FileConfigurator.h"
#include <QFileInfo>


namespace IQmol {
namespace Layer {

   /// Simple Layer that represents a data file (e.g. checkpoint file).
   class File : public Data {

      Q_OBJECT

      friend class Configurator::File;

      public:
         explicit File(QString const& fileName) : m_fileName(fileName), m_configurator(this) { 
            QFileInfo fileInfo(m_fileName);
            setText(fileInfo.fileName());
            setConfigurator(&m_configurator);
         }

         void tail(int const interval = 2000) {
            configure();
            m_configurator.tail(interval);
         }

      protected:
         QString fileName() const { return m_fileName; }
        
      private:
         QString m_fileName;
         Configurator::File m_configurator;
   };


   class Files : public Data {

      Q_OBJECT;

      public:
         Files() : Data("Files") { }
         QList<File*> getFiles() { return findLayers<File>(Children); }
   };


} // end namespace Layer

typedef QList<Layer::File*> FileList;

} // end namespace IQmol

#endif
