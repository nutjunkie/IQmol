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

#include "Preferences.h"
#include "PreferencesBrowser.h"
#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QSettings>
#include <QFileInfo>
#include <QtGlobal>
#include <QMap>
#include <QDir>

#include <QDebug>


namespace IQmol {
namespace Preferences {

static QString s_organization = "iqmol.org";
static QString s_application  = "IQmol";

// On Windows the QSettings 'source' does not get updated when creating a new
// QSettings object, which means settings only get updated when IQmol is
// restarted.  To get around this we carry our own cached settings which we can
// reference during the current invocation.
static QMap<QString, QVariant> s_preferencesCache;

// **********  Browser  ********* //
Browser::Browser(QWidget* parent) : QDialog(parent) 
{
   m_preferencesBrowser.setupUi(this);
   init();
}


void Browser::init()
{
   // These are the options we allow the user to edit in the Browser.  If you
   // add another option here, then you also need to update the
   // Browser::on_okButtonClicked function below.
   m_preferencesBrowser.fragmentDirectoryLineEdit->setText(FragmentDirectory());
   m_preferencesBrowser.qchemDatabaseFileLineEdit->setText(QChemDatabaseFilePath());
   m_preferencesBrowser.logFileLineEdit->setText(LogFilePath());

   if (LogFileHidden()) {
      m_preferencesBrowser.logFileHiddenCheckBox->setCheckState(Qt::Checked);
   }else {
      m_preferencesBrowser.logFileHiddenCheckBox->setCheckState(Qt::Unchecked);
   }

   if (LoggingEnabled()) {
      m_preferencesBrowser.loggingEnabledCheckBox->setCheckState(Qt::Checked);
   }else {
      m_preferencesBrowser.loggingEnabledCheckBox->setCheckState(Qt::Unchecked);
   }

   int idx(m_preferencesBrowser.forceFieldCombo->findText(DefaultForceField()));
   m_preferencesBrowser.forceFieldCombo->setCurrentIndex(idx);
   m_preferencesBrowser.undoLimit->setValue(UndoLimit());
   m_preferencesBrowser.labelFontSize->setValue(LabelFontSize());
}



void Browser::on_buttonBox_accepted()  
{
   LoggingEnabled(m_preferencesBrowser.loggingEnabledCheckBox->checkState() == Qt::Checked);
   bool logFileHidden(m_preferencesBrowser.logFileHiddenCheckBox->checkState() == Qt::Checked);
   LogFileHidden(logFileHidden);

   QFileInfo logFile(m_preferencesBrowser.logFileLineEdit->text());
#ifdef Q_OS_WIN32
   // On Windows the file hidden attribute is set when the log 
   // file is opened in main.C
#else
   QDir dir(logFile.dir());
   QString name(logFile.fileName());
   if (logFileHidden) {
      if (!name.startsWith(".")) name.push_front(".");
   }else {
      while (name.startsWith(".")) { name.remove(0,1); }
   }
   logFile.setFile(dir,name);
#endif

   LogFilePath(logFile.filePath());
   m_preferencesBrowser.logFileLineEdit->setText(logFile.filePath());
   
   // Don't overwrite the default locations unless the user has changed them
   QString oldLocation(QChemDatabaseFilePath());
   QString newLocation(m_preferencesBrowser.qchemDatabaseFileLineEdit->text());
   if (oldLocation != newLocation) QChemDatabaseFilePath(newLocation);
   
   oldLocation = FragmentDirectory();
   newLocation = m_preferencesBrowser.fragmentDirectoryLineEdit->text();
   if (oldLocation != newLocation) FragmentDirectory(newLocation);

   DefaultForceField(m_preferencesBrowser.forceFieldCombo->currentText());
   UndoLimit(m_preferencesBrowser.undoLimit->value());
   LabelFontSize(m_preferencesBrowser.labelFontSize->value());
   updated();
   accept();
}


void Browser::on_browseFragmentDirectoryButton_clicked(bool) 
{
   setPath(m_preferencesBrowser.fragmentDirectoryLineEdit);
}


void Browser::on_browseLogFileButton_clicked(bool) 
{
   bool mustExist(false);
   setFilePath(m_preferencesBrowser.logFileLineEdit, mustExist);
}

void Browser::on_browseQChemDatabaseFileButton_clicked(bool) 
{
   bool mustExist(true);
   setFilePath(m_preferencesBrowser.qchemDatabaseFileLineEdit, mustExist);
}


void Browser::on_resetButton_clicked(bool)
{
   ResetBrowserPreferences();
   init();
}




//! Convenience function that opens a file browser so the user can specify a
//! directory.
void Browser::setPath(QLineEdit* edit) 
{
   QString s(edit->text());
   QString path(QFileDialog::getExistingDirectory(this, tr("Select Path"), s));
   if (!path.isEmpty()) edit->setText(path);
}


//! Convenience function that opens a file browser so the user can specify a
//! file.
void Browser::setFilePath(QLineEdit* edit, bool const mustExist) 
{
   QString s(edit->text());
   QString path;
   if (mustExist) {
      path = QFileDialog::getOpenFileName(this, tr("Select File"), s);
   }else {
      path = QFileDialog::getSaveFileName(this, tr("Select File"), s);
   }
   if (!path.isEmpty()) edit->setText(path);
}


} } // end namespace IQmol::Preferences
