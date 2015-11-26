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

#include "FileConfigurator.h"
#include "Preferences.h"
#include "FileLayer.h"
#include <QRegExp>
#include <QFileInfo>
#include <QFont>


namespace IQmol {
namespace Configurator { 

File::File(Layer::File& file) : m_file(file), m_qfile(this)
{
   m_fileConfigurator.setupUi(this);
   connect(&file, SIGNAL(updated()), this, SLOT(sync()));
   connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTail()));
   QFileInfo info(m_file.fileName());
   setWindowTitle(info.fileName());
}


void File::sync() 
{
   m_fileConfigurator.textDisplay->clear();
   changeFont(Preferences::FileDisplayFont());
   m_qfile.setFileName(m_file.fileName());
   if (m_qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_fileConfigurator.textDisplay->setText(QString(m_qfile.readAll()));
      m_qfile.close();
   }
}


void File::tail(int const interval) 
{
   m_refreshTimer.setInterval(interval);
   m_fileConfigurator.textDisplay->clear();
   changeFont(Preferences::FileDisplayFont());

   m_qfile.setFileName(m_file.fileName());
   if (m_qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_fileConfigurator.textDisplay->setText(QString(m_qfile.readAll()));
      m_refreshTimer.start();
   }
}


void File::refreshTail()  
{
   if (m_qfile.isOpen()) {
      QString output(m_qfile.readAll());
      if (!output.isEmpty()) m_fileConfigurator.textDisplay->append(output);
   }
} 


void File::on_textLargerButton_clicked(bool) {
   QFont font(Preferences::FileDisplayFont());
   int size = font.pointSize() + 1;
   font.setPointSize(size);
   changeFont(font);
}


void File::on_textSmallerButton_clicked(bool) {
   QFont font(Preferences::FileDisplayFont());
   int size = font.pointSize() - 1;
   font.setPointSize(size);
   changeFont(font);
}


void File::changeFont(QFont const& font) {
   Preferences::FileDisplayFont(font);
   QString text(m_fileConfigurator.textDisplay->toPlainText());
   m_fileConfigurator.textDisplay->clear();
   m_fileConfigurator.textDisplay->setCurrentFont(font);
   m_fileConfigurator.textDisplay->setText(text);
}


void File::on_searchButton_clicked(bool) {
   QTextDocument::FindFlags flags(0);
   QRegExp rx("[A-Z]");
   // Try to immitate vi's smart search, if the search term has a capital
   // letter then we do a case sensitive search, otherwise it is case
   // insensitive.
   if (rx.indexIn(m_searchText) != -1) {
      flags = QTextDocument::FindCaseSensitively;
   }

   bool found(m_fileConfigurator.textDisplay->find(m_searchText, flags));

   if (!found) { // Have another go from the start
      m_fileConfigurator.textDisplay->moveCursor(QTextCursor::Start);
      m_fileConfigurator.textDisplay->find(m_searchText, flags);
   }
}


void File::on_searchBox_textEdited(QString const& text)
{
   m_searchText = text;
   on_searchButton_clicked(true);
}


void File::on_searchBox_returnPressed()
{
   on_searchButton_clicked(true);
}


void File::closeEvent(QCloseEvent* e)
{
   m_refreshTimer.stop();
   if (m_qfile.isOpen()) m_qfile.close();
   Base::closeEvent(e);
}



} } // end namespace IQmol::Configurator
