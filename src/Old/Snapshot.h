#ifndef IQMOL_SNAPSHOT_H
#define IQMOL_SNAPSHOT_H
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

#include <QStringList>
#include <QImage>


namespace IQmol {

   class Viewer;

   /// Class that encapsulates the saving of snapshots and saving them to file.
   class Snapshot : public QObject {

      Q_OBJECT

      public:
         enum Format { JPG, PNG, TIFF, PPM, BMP, EPS, PDF, SVG };

         enum Flags { 
            None          = 0x000,  
            AutoIncrement = 0x001,
            Overwrite     = 0x002,
            Movie         = 0x004
         };

         Snapshot(Viewer* viewer, int const flags = 0);

         bool requestFileName();
         void resetCounter() { m_counter = 0; } 
         void makeMovie();

      public Q_SLOTS:
         void capture();

      private:
         void captureVector(QString const& fileName, int const format);
         void capture(QString const& fileName);

         // merge with captureVector
         void writefile(int format, int sort, int options, int nbcol,
               const char *filename, const char *extension);

         QString m_fileBaseName; 
         QString m_fileExtension;
         Format  m_fileFormat;

         Viewer* m_viewer;
         int m_flags;
         int m_counter;
         QImage m_image;
         QStringList m_fileNames;
   };


} // end namespace IQmol


#endif
