#ifndef IQMOL_PREFERENCESBROWSER_H
#define IQMOL_PREFERENCESBROWSER_H
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

#include <QVariant>
#include <QDialog>
#include "ui_PreferencesBrowser.h"


namespace IQmol {
namespace Preferences {

   /// Allows the user to view and edit the configurable preferences.
   class Browser : public QDialog {

      Q_OBJECT

      public:
         Browser(QWidget* parent);
         ~Browser() { }

      Q_SIGNALS:
         void updated();

      private Q_SLOTS:
         void on_buttonBox_accepted();
         void on_browseFragmentDirectoryButton_clicked(bool);
         void on_browseQChemDatabaseFileButton_clicked(bool);
         void on_browseLogFileButton_clicked(bool);
         void on_resetButton_clicked(bool);

      private:
         Ui::PreferencesBrowser m_preferencesBrowser;

         void setPath(QLineEdit* edit);
         void setFilePath(QLineEdit* edit, bool const mustExist = false);
         void init();
   };

} } // end namespace IQmol::Preferences

#endif
