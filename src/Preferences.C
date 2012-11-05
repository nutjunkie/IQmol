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

#include "Preferences.h"
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
static int s_maxRecentFiles = 10;

// On Windows the QSettings 'source' does not get updated when creating a new
// QSettings object, which means settings only get updated when IQmol is
// restarted.  To get around this we carry our own cached settings which we can
// reference during the current invocation.
static QMap<QString, QVariant> s_preferencesCache;

// **********  Browser  ********* //
Browser::Browser(QWidget* parent) : QDialog(parent) 
{
   m_preferencesBrowser.setupUi(this);
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
   FragmentDirectory(m_preferencesBrowser.fragmentDirectoryLineEdit->text());
   LoggingEnabled(m_preferencesBrowser.loggingEnabledCheckBox->checkState() == Qt::Checked);
   bool logFileHidden(m_preferencesBrowser.logFileHiddenCheckBox->checkState() == Qt::Checked);
   LogFileHidden(logFileHidden);

   QFileInfo logFile(m_preferencesBrowser.logFileLineEdit->text());
#ifdef Q_WS_WIN
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

   QChemDatabaseFilePath(m_preferencesBrowser.qchemDatabaseFileLineEdit->text());
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




// **********  Non-member Access Functions  ********* //
// The following functions give access to individual options and provide the
// interface for changing options.  When adding preferences, a new (overloaded)
// function should be created to get and set the option.  This should provide a
// default if the preference does not exist in the preference file, and also
// take care of the necessary conversion from the QVariant.


// Size of the InputDialog window
QSize MainWindowSize() 
{
   QVariant value(Get("MainWindowSize"));
   return value.isNull() ? QSize(800,600) : value.value<QSize>();
}

void MainWindowSize(QSize const& size) 
{
   Set("MainWindowSize", QVariant::fromValue(size));
}


// ---------

// Width of the ModelView window on the left of the MainWindow
bool ShowModelView()
{
   QVariant value(Get("ShowModelView"));
   return value.isNull() ? true : value.value<bool>();
}

void ShowModelView(bool const tf) 
{
   Set("ShowModelView", QVariant::fromValue(tf));
}


// ---------

int UndoLimit() 
{
   QVariant value(Get("UndoLimit"));
   return value.isNull() ? 10 : value.value<int>();
}

void UndoLimit(int const limit)
{
   Set("UndoLimit", QVariant::fromValue(limit));
}


// ---------

int LabelFontSize() 
{
   QVariant value(Get("LabelFontSize"));
   return value.isNull() ? 14 : value.value<int>();
}

void LabelFontSize(int const size)
{
   Set("LabelFontSize", QVariant::fromValue(size));
}

// ---------

// FileDialogs should use this to initialize the filename when opening and
// saving files
QString LastFileAccessed() 
{
   QVariant value(Get("LastFileAccesed"));
   return value.isNull() ? QString() : value.value<QString>();
}

void LastFileAccessed(QString const& filePath) 
{
   Set("LastFileAccesed", QVariant::fromValue(filePath));
}

// ---------

QString LogFilePath() 
{
   QVariant value(Get("LogFilePath"));
   QString logFilePath;

   if (value.isNull()) {
      logFilePath = QDir::homePath();
      if (logFilePath.isEmpty()) logFilePath = ".";
      logFilePath += "/.iqmol.log";
   }else {
      logFilePath = value.value<QString>();
   }
   return logFilePath;
}

void LogFilePath(QString const& filePath) 
{
   Set("LogFilePath", QVariant::fromValue(filePath));
}


// ---------

bool LogFileHidden()
{
   QVariant value(Get("LogFileHidden"));
   return value.isNull() ? true : value.value<bool>();
}

void LogFileHidden(bool const tf) 
{
   Set("LogFileHidden", QVariant::fromValue(tf));
}


// ---------

bool LoggingEnabled()
{
   QVariant value(Get("LoggingEnabled"));
   return value.isNull() ? true : value.value<bool>();
}

void LoggingEnabled(bool const tf) 
{
   Set("LoggingEnabled", QVariant::fromValue(tf));
}


// ---------

QString QChemDatabaseFilePath() 
{
   QVariant value(Get("QChemDatabaseFilePath"));
   QString databaseFilePath;

   if (value.isNull()) {
      QDir dir(QApplication::applicationDirPath());
#ifdef Q_WS_MAC
      dir.cdUp();
      dir.cd("Resources");
      databaseFilePath = dir.absolutePath();
#endif
      databaseFilePath = dir.absolutePath() + "/qchem_option.db";
   }else {
      databaseFilePath = value.value<QString>();
   }
   return databaseFilePath;
}

void QChemDatabaseFilePath(QString const& filePath) 
{
   Set("QChemDatabaseFilePath", QVariant::fromValue(filePath));
}


// ---------

QString FragmentDirectory() 
{
   QString fragDir;
   QVariant value(Get("FragmentDirectory"));

   if (value.isNull()) {
      QDir dir(QApplication::applicationDirPath());
#ifdef Q_WS_MAC
      dir.cdUp();
      dir.cd("share/fragments");
      fragDir = dir.absolutePath();
#else
      fragDir = dir.absolutePath() + "/fragments";
#endif
   }else {
      fragDir = value.value<QString>();
   }
   return fragDir;
}

void FragmentDirectory(QString const& filePath) 
{
   Set("FragmentDirectory", QVariant::fromValue(filePath));
}

// ---------

QString DefaultForceField()
{
   QVariant value(Get("DefaultForceField"));
   return value.isNull() ? QString("UFF") : value.value<QString>();
}

void DefaultForceField(QString const& forceField)
{
   Set("DefaultForceField", QVariant::fromValue(forceField));
}

// ---------

double SymmetryTolerance()
{
   QVariant value(Get("SymmetryTolerance"));
   return value.isNull() ? 0.100  : value.value<double>();
}

void SymmetryTolerance(double const tolerance)
{
   Set("SymmetryTolerance", QVariant::fromValue(tolerance));
}

// ---------

QColor PositiveSurfaceColor() 
{
   QVariant value(Get("PositiveSurfaceColor"));
   return value.isNull() ? QColor("blue") : value.value<QColor>();
}

void PositiveSurfaceColor(QColor const& color) 
{
   Set("PositiveSurfaceColor", QVariant::fromValue(color));
}

// ---------

QColor NegativeSurfaceColor() 
{
   QVariant value(Get("NegativeSurfaceColor"));
   return value.isNull() ? QColor("red") : value.value<QColor>();
}

void NegativeSurfaceColor(QColor const& color) 
{
   Set("NegativeSurfaceColor", QVariant::fromValue(color));
}

// ---------

QList<QColor> SpectrumGradientColors()
{
   static QColor orange("#FFA500");
   QList<QColor> colors;
   colors << Qt::red << orange << Qt::yellow << Qt::green
          << Qt::cyan << Qt::blue << Qt::magenta;
   return colors;
}

QList<QColor> DefaultGradientColors()
{
   QList<QColor> colors;
   colors << Qt::red << Qt::white << Qt::blue;
   return colors;
}

QList<QColor> CustomGradientColors()
{
   QList<QColor> colors(GetList<QColor>("CustomGradientColors"));
   if (colors.isEmpty()) {
      colors << Qt::magenta << Qt::black<< Qt::cyan;
   }
   return colors;
}

void CustomGradientColors(QList<QColor> const& colors)
{
   SetList("CustomGradientColors", colors);
}


// ---------

QColor BackgroundColor() 
{
   QVariant value(Get("BackgroundColor"));
   return value.isNull() ? QColor(32, 92, 168) : value.value<QColor>();
}

void BackgroundColor(QColor const& color) 
{
   Set("BackgroundColor", QVariant::fromValue(color));
}


// ---------

QColor ForegroundColor() 
{
   QVariant value(Get("ForegroundColor"));
   return value.isNull() ? QColor(180, 180, 180) : value.value<QColor>();
}

void ForegroundColor(QColor const& color) 
{
   Set("ForegroundColor", QVariant::fromValue(color));
}

// ---------

void VibrationVectorColor(QColor const& color) 
{
   Set("VibrationVectorColor", QVariant::fromValue(color));
}

QColor VibrationVectorColor() 
{
   QVariant value(Get("VibrationVectorColor"));
   return value.isNull() ? QColor(255, 0, 255) : value.value<QColor>();
}

// ---------

QStringList RecentFiles() 
{
   QVariant value(Get("RecentFiles"));
   return value.isNull() ? QStringList() : value.value<QStringList>();
}

void AddRecentFile(QString const& filePath)
{
   QStringList files(RecentFiles());
   if (!files.contains(filePath)) {
      files.push_front(filePath);
      while (files.size() > s_maxRecentFiles) { files.pop_back(); }
      Set("RecentFiles", QVariant::fromValue(files));
   }
}

void RemoveRecentFile(QString const& filePath)
{
   QStringList files(RecentFiles());
   int index(files.indexOf(filePath)); 
   if (index >= 0) {
      files.removeAt(index);
      Set("RecentFiles", QVariant::fromValue(files));
   }
}

void ClearRecentFiles()
{
   QStringList empty;
   Set("RecentFiles", QVariant::fromValue(empty));
}

// ---------

unsigned int PasswordVaultSeed()
{
   QVariant value(Get("PasswordVaultSeed"));
   return value.isNull() ? 0 : value.value<unsigned int>();
}

void PasswordVaultSeed(unsigned int const seed)
{
   Set("PasswordVaultSeed", seed);
}

// ---------

QString PasswordVaultKey()
{
   QVariant value(Get("PasswordVaultKey"));
   return value.isNull() ? QString() : value.value<QString>();
}

void PasswordVaultKey(QString const& key)
{
   Set("PasswordVaultKey", key);
}

// ---------

void ClearPasswordVaultContents()
{
   SetMap("PasswordVaultContents", QMap<QString,QString>());
}

QMap<QString, QString> PasswordVaultContents()
{
   return GetMap<QString>("PasswordVaultContents");
}

void PasswordVaultContents(QMap<QString,QString> const& contents)
{
   SetMap("PasswordVaultContents", contents);
}

// ---------

QString SSHKnownHostsFile()
{
   QVariant value(Get("SSHKnownHostsFile"));
   if (!value.isNull()) return value.value<QString>();

   QString home(QDir::homePath());
   if (home.isEmpty()) return QString();

   QString fileName(home + "/.ssh/known_hosts");
   QFileInfo info(fileName); 
   if (info.exists()) return fileName;

   return QString();
}

void SSHKnownHostsFile(QString const& file)
{
   Set("SSHKnownHostsFile", file);
}

// ---------

QString SSHPublicIdentityFile()
{
   QVariant value(Get("SSHPublicIdentityFile"));
   if (!value.isNull()) return value.value<QString>();

   QString home(QDir::homePath());
   if (home.isEmpty()) return QString();

   QString fileName(home + "/.ssh/id_rsa.pub");
   QFileInfo info(fileName); 
   if (info.exists()) return fileName;

   fileName = home + "/.ssh/id_dsa.pub";
   info.setFile(fileName);
   if (info.exists()) return fileName;

   fileName = home + "/.ssh/identity.pub";
   info.setFile(fileName);
   if (info.exists()) return fileName;

   return QString();
}

void SSHPublicIdentityFile(QString const& file)
{
   Set("SSHPublicIdentityFile", file);
}

// ---------

QString SSHPrivateIdentityFile()
{
   QVariant value(Get("SSHPrivateIdentityFile"));
   if (!value.isNull()) return value.value<QString>();

   QString home(QDir::homePath());
   if (home.isEmpty()) return QString();

   QString fileName(home + "/.ssh/id_rsa");
   QFileInfo info(fileName); 
   if (info.exists()) return fileName;

   fileName = home + "/.ssh/id_dsa";
   info.setFile(fileName);
   if (info.exists()) return fileName;

   fileName = home + "/.ssh/identity";
   info.setFile(fileName);
   if (info.exists()) return fileName;

   return QString();
}

void SSHPrivateIdentityFile(QString const& file)
{
   Set("SSHPrivateIdentityFile", file);
}

// ---------

QVariantList ServerList()
{
   return GetQVariantList("ServerList");
}

void ServerList(QVariantList const& servers)
{
   SetList("ServerList", servers);
}

// ---------


QVariantList CurrentProcessList()
{
   return GetQVariantList("CurrentProcessList");
}

void CurrentProcessList(QVariantList const& processList)
{
   SetList("CurrentProcessList", processList);
}

// ---------



// Size of the InputDialog window
QSize QuiWindowSize() {
   QVariant value(Get("QuiWindowSize"));
   return value.isNull() ? QSize(960,570) : value.value<QSize>();
}

void QuiWindowSize(QSize const& size) {
   Set("QuiWindowSize", QVariant::fromValue(size));
}  
   

// Size of the Output file display
QSize FileDisplayWindowSize() {
   QVariant value(Get("FileDisplayWindowSize"));
   return value.isNull() ? QSize(550,400) : value.value<QSize>();
}

void FileDisplayWindowSize(QSize const& size) {
   Set("FileDisplayWindowSize", QVariant::fromValue(size));
}  
   

// Font for the InputDialog previewText display
QFont PreviewFont() {
   QVariant value(Get("PreviewTextFont"));
   return value.isNull() ? QFont("Courier New") : value.value<QFont>();
}

void PreviewFont(QFont const& font) {
   Set("PreviewTextFont", QVariant::fromValue(font));
}


// Font for displaying the output files.
QFont FileDisplayFont() {
   QVariant value(Get("FileDisplayFont"));
   return value.isNull() ? QFont("Courier New") : value.value<QFont>();
}

void FileDisplayFont(QFont const& font) {
   Set("FileDisplayFont", QVariant::fromValue(font));
}

QString TemplateDirectory() {
   QVariant value(Get("TemplateDirectory"));
   return value.isNull() ? QString() : value.value<QString>();
}

void TemplateDirectory(QString const& filePath) {
   Set("TemplateDirectory", QVariant::fromValue(filePath));
}



// ----------------------------------------------------------------

//! Retrieves a preference setting from the global QSettings object.
//! Should not be used outside the Preferences namespace.
QVariant Get(QString const& name) 
{
   QVariant val;
   if (s_preferencesCache.contains(name)) {
      val = s_preferencesCache[name];
   }else {
      QSettings settings(QSettings::UserScope, s_organization, s_application);
      val = settings.value(name);
   }
   return val;
}


QList<QVariant> GetQVariantList(QString const& name)
{
   QVariant value(Get(name));
   QVariantList list;
   if (!value.isNull()) list = value.toList();
   return list;
}


template <class T>
QList<T> GetList(QString const& name)
{
   QVariant value(Get(name));
   QList<T> list;

   if (!value.isNull()) {
      QVariantList qvars(value.toList());
      QVariantList::iterator iter;
      for (iter = qvars.begin(); iter != qvars.end(); ++iter) {
          list.append( (*iter).value<T>() );
      }
   }

   return list;
}


template <class T>
QMap<QString, T> GetMap(QString const& name)
{
   QVariant qvar(Get(name));
   QMap<QString,T> map;

   if (!qvar.isNull()) {
      QMap<QString, QVariant> variantMap(qvar.toMap());
      QStringList keys(variantMap.keys());

      QStringList::iterator iter;
      for (iter = keys.begin(); iter != keys.end(); ++iter) {
          qvar = variantMap.value(*iter);
          map.insert( (*iter), qvar.value<T>());
      }
   }

   return map;
}


//! Changes a preference setting in the global QSettings object.
//! Should not be used outside the Preferences namespace.
void Set(QString const& name, QVariant const& value) 
{
   QSettings settings(QSettings::UserScope, s_organization, s_application);
   settings.setValue(name,value);
   s_preferencesCache.insert(name, value);
}

template <class T>
void SetList(QString const& name, QList<T> const& values)
{
   QVariantList variantList;
   for (int i = 0; i < values.size(); ++i) {
       variantList.append(QVariant(values[i]));
   }
   Set(name, QVariant(variantList));
}


/// Converts a QMap into QVariant form for storage.
template <class T>
void SetMap(QString const& name, QMap<QString, T> const& map)
{
   QMap<QString, QVariant> variantMap;
   QStringList keys(map.keys());
   for (int i = 0; i < keys.size(); ++i) {
       variantMap.insert(keys[i], QVariant(map.value(keys[i])));
   }
   Set(name, QVariant(variantMap));
}


} } // end namespace IQmol::Preferences
