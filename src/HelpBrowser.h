#ifndef IQMOL_HELPBROWSER_H
#define IQMOL_HELPBROWSER_H
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

#include "ui_HelpBrowser.h"
#include <QDialog>
#include <QString>
#include <QMap>


namespace IQmol {

   /// A basic dialog with a QTextBrowser for displaying help files.
   class HelpBrowser : public QDialog {

      Q_OBJECT

      public:
         HelpBrowser(QWidget* parent);
         ~HelpBrowser() { }

      private Q_SLOTS:
         void on_homeButton_clicked(bool);
         void on_forwardButton_clicked(bool);
         void on_backButton_clicked(bool);
         void on_searchButton_clicked(bool);
         void on_searchLineEdit_returnPressed();
         void checkUrlForSearch(QUrl const&);

      private:
         void loadPage(QString const& file);
         void loadImages();
         void loadHelpFiles();
         void setStyleSheet();
         QString createSearchResults(QString const&);
         QString readFile(QString const& file);

         Ui::HelpBrowser m_helpBrowser;
         QString m_searchResults;
         QMap<QString, QString> m_fileContents;
   };

} // end namespace IQmol

#endif
