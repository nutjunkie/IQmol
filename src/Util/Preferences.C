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

#include "Preferences.h"
#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QMap>
#include <QDir>
#include <QFont>
#include <QColor>

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


// **********  Non-member Access Functions  ********* //
// The following functions give access to individual options and provide the
// interface for changing options.  When adding preferences, a new (overloaded)
// function should be created to get and set the option.  This should provide a
// default if the preference does not exist in the preference file, and also
// take care of the necessary conversion from the QVariant.


// These functions are generic and should only be used within the Preferences
// module and not in the general code.
QVariant Get(QString const& name);
void     Set(QString const& name, QVariant const& value);

template <class T> QList<T> GetList(QString const& name);
template <class T> void     SetList(QString const& name, QList<T> const& list);

template <class T> QMap<QString,T> GetMap(QString const& name);
template <class T> void            SetMap(QString const& name, QMap<QString,T> const& map);


void ResetBrowserPreferences()
{
   // We only reset the preferences that show in the Preferences Browser
   QStringList options;
   options << "DefaultForceField"
           << "UndoLimit"
           << "LabelFontSize"
           << "FragmentDirectory"
           << "QChemDatabaseFilePath"
           << "LogFilePath"
           << "LogFileHidden"
           << "LoggingEnabled"
           << "SurfaceOpacity"
           // And a few others
           << "MainWindowSize"
           << "QuiWindowSize"
           << "ShowModelView"
           << "ShaderDirectory"
           << "DefaultShader"
           << "DefaultShaderParameters"
           << "BackgroundColor"
           << "ForegroundColor"
           << "CurrentProcessList"
           << "JobMonitorList"
           << "FileDisplayWindowSize"
;

   QSettings settings(QSettings::UserScope, s_organization, s_application);
   for (int i = 0; i < options.size(); ++i) {
       s_preferencesCache.remove(options[i]);
       settings.remove(options[i]);
   }
}


// Size of the InputDialog window
QSize MainWindowSize() 
{
   QVariant value(Get("MainWindowSize"));
   return value.isNull() ? QSize(950,650) : value.value<QSize>();
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
   QString home(QDir::homePath());
   return value.isNull() ? home : value.value<QString>();
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

double SurfaceOpacity()
{
   QVariant value(Get("SurfaceOpacity"));
   return value.isNull() ? 50 : value.value<double>();
}

void SurfaceOpacity(double const value) 
{
   Set("SurfaceOpacity", QVariant::fromValue(value));
}

// ---------

bool LogFileHidden()
{
   QVariant value(Get("LogFileHidden"));

#ifdef Q_OS_WIN32
   return value.isNull() ? false : value.value<bool>();
#else
   return value.isNull() ? true : value.value<bool>();
#endif
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

   if (value.isNull() || value.toString().isEmpty()) {
      QDir dir;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
      dir.setPath(QApplication::applicationDirPath());
      dir.cdUp();
      dir.cd("share");
#else  
      dir.setPath("/usr/share/iqmol");
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

QString ShaderDirectory() 
{
   QString shaderDir;
   QVariant value(Get("ShaderDirectory"));

   if (value.isNull()) {
      QDir dir;
#if  defined(Q_OS_MAC) || defined(Q_OS_WIN32)
      dir.setPath(QApplication::applicationDirPath());
      dir.cdUp();
      dir.cd("share/shaders");
#else
      dir.setPath("/usr/share/iqmol/shaders");
#endif
      shaderDir = dir.absolutePath();
   }else {
      shaderDir = value.value<QString>();
   }
   return shaderDir;
}

void ShaderDirectory(QString const& filePath) 
{
   Set("ShaderDirectory", QVariant::fromValue(filePath));
}


// ---------


QString DefaultShader()
{
   QVariant value(Get("DefaultShader"));
   return value.isNull() ? "Phong" : value.value<QString>();
}

void DefaultShader(QString const& shader)
{
   Set("DefaultShader", shader);
}


// ---------


QVariantMap DefaultShaderParameters()
{
   QVariantMap map;
   QVariant qvar(Get("DefaultShaderParameters"));
   if (!qvar.isNull()) map = qvar.toMap();
   return map;
}

void DefaultShaderParameters(QVariantMap const& map)
{
   Set("DefaultShaderParameters", QVariant(map));
}


// ---------


QVariantMap DefaultPovRayParameters()
{
   QVariantMap map;
   QVariant qvar(Get("DefaultPovRayParameters"));
   if (!qvar.isNull()) map = qvar.toMap();
   return map;
}

void DefaultPovRayParameters(QVariantMap const& map)
{
   Set("DefaultPovRayParameters", QVariant(map));
}


// ---------


QVariantMap DefaultFilterParameters()
{
   QVariantMap map;
   QVariant qvar(Get("DefaultFilterParameters"));
   if (!qvar.isNull()) map = qvar.toMap();
   return map;
}

void DefaultFilterParameters(QVariantMap const& map)
{
   Set("DefaultFilterParameters", QVariant(map));
}


// ---------


QString FragmentDirectory() 
{
   QString fragDir;
   QVariant value(Get("FragmentDirectory"));

   if (value.isNull() || value.toString().isEmpty()) {
      QDir dir;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
      dir.setPath(QApplication::applicationDirPath());
      dir.cdUp();
      dir.cd("share/fragments");
#else
      dir.setPath("/usr/share/iqmol/fragments");
#endif
      fragDir = dir.absolutePath();
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
   return value.isNull() ? QColor(0,98,255) : value.value<QColor>();
}

void PositiveSurfaceColor(QColor const& color) 
{
   Set("PositiveSurfaceColor", QVariant::fromValue(color));
}

// ---------

QColor NegativeSurfaceColor() 
{
   QVariant value(Get("NegativeSurfaceColor"));
   return value.isNull() ? QColor(226,37,79) : value.value<QColor>();
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
   QVariant value(Get("ServerList"));
   QVariantList list;
   if (!value.isNull()) list = value.toList();
   return list;
}

void ServerList(QVariantList const& servers)
{
   SetList("ServerList", servers);
}


// ---------

QVariantList ServerConfigurationList()
{
   QVariant value(Get("ServerConfigurationList"));
   QVariantList list;
   if (!value.isNull()) list = value.toList();
   return list;
}

void ServerConfigurationList(QVariantList const& servers)
{
   SetList("ServerConfigurationList", servers);
}


// ---------

QString ServerQueryJobFinished() {
   QVariant value(Get("ServerQueryJobFinished"));
   return value.isNull() ? QString("No tasks are running") : value.value<QString>();
}

void ServerQueryJobFinished(QString const& repsonse) {
   Set("ServerQueryJobFinished", QVariant::fromValue(repsonse));
}


// ---------

int DaysToRememberJobs() {
   QVariant value(Get("DaysToRememberJobs"));
   return value.isNull() ? 7 : value.value<int>();
}

void DaysToRememberJobs(int days) {
   Set("DaysToRememberJobs", QVariant::fromValue(days));
}


// ---------

QVariantList JobMonitorList()
{
   QVariant value(Get("JobMonitorList"));
   QVariantList list;
   if (!value.isNull()) list = value.toList();
   return list;
}

void JobMonitorList(QVariantList const& jobList)
{
   SetList("JobMonitorList", jobList);
}


// ---------


QVariantList CurrentProcessList()
{
   QVariant value(Get("CurrentProcessList"));
   QVariantList list;
   if (!value.isNull()) list = value.toList();
   return list;
}

void CurrentProcessList(QVariantList const& processList)
{
   SetList("CurrentProcessList", processList);
}


// ---------


// Size of the InputDialog window
QSize QuiWindowSize() {
   QVariant value(Get("QuiWindowSize"));
   return value.isNull() ? QSize(950,550) : value.value<QSize>();
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
   return value.isNull() ? QFont("Courier New", 10) : value.value<QFont>();
}

void PreviewFont(QFont const& font) {
   Set("PreviewTextFont", QVariant::fromValue(font));
}


// Font for displaying the output files.
QFont FileDisplayFont() {
   QVariant value(Get("FileDisplayFont"));
   return value.isNull() ? QFont("Courier New", 10) : value.value<QFont>();
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
