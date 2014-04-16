#ifndef IQMOL_CONFIGURATOR_FILE_H
#define IQMOL_CONFIGURATOR_FILE_H
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

#include "Configurator.h"
#include "ui_FileConfigurator.h"
#include <QTimer>
#include <QFile>


class QFont;

namespace IQmol {

namespace Layer {
   class File;
}

namespace Configurator {

   /// Dialog which simply displays the contents of a file.  This is not  much
   /// of a configurator, but it feels natural that 'configuring' a file (i.e.
   /// double clicking the layer in the Model View) allows the user to view the 
   /// contents.
   class File: public Base {

      Q_OBJECT

      public:
         explicit File(Layer::File& file);
      
      public Q_SLOTS:
         /// Re-reads the entire contents of the file from disk.
         void sync();
         /// Sets up a tail on the file.  Refresh continues until
         /// the window is closed.
         void tail(int const interval);

      private Q_SLOTS:
         void on_textLargerButton_clicked(bool);
         void on_textSmallerButton_clicked(bool);
         void on_searchButton_clicked(bool);
         void on_searchBox_textEdited(QString const&);
         void on_searchBox_returnPressed();
         void refreshTail();


      private:
         void closeEvent(QCloseEvent* e);
         void changeFont(QFont const& font);
         Ui::FileConfigurator m_fileConfigurator;
         Layer::File& m_file;
         QString m_searchText;

         QTimer m_refreshTimer;
         QFile  m_qfile;  // used for tailing the file
   };

} } // end namespace IQmol::Configurator

#endif
