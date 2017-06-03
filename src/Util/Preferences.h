#ifndef IQMOL_UTIL_PREFERENCES_H
#define IQMOL_UTIL_PREFERENCES_H
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

/*!
 *  \file Preferences.h 
 *
 *  \brief Global function to access and change the user preferences.
 *
 *  The idea is that all preference requests should go through the Preferences
 *  interface so that this is the only place where the QSettings object is
 *  changed.  This means that if a developer wants to check an option they only
 *  need to reference this header file (and the corresponding source file),
 *  which helps prevent duplication of preferences.  Note that defaults are also
 *  set in Preferences.C
 */

namespace IQmol {
namespace Preferences {

   void ResetBrowserPreferences();

   QSize ViewerWindowSize();
   void   ViewerWindowSize(QSize const&);
   
   bool  ShowModelView();
   void  ShowModelView(bool const);

   int   UndoLimit();
   void  UndoLimit(int const);
   
   int   LabelFontSize();
   void  LabelFontSize(int const);
   
   QSize MainWindowSize();
   void  MainWindowSize(QSize const&);
   
   QFont PreviewFont();
   void  PreviewFont(QFont const&);
   
   QFont FileDisplayFont();
   void  FileDisplayFont(QFont const&);
   
   QString LastFileAccessed();
   void    LastFileAccessed(QString const&);
   
   QString FragmentDirectory();
   void    FragmentDirectory(QString const&);

   QString ShaderDirectory();
   void    ShaderDirectory(QString const&);

   QString DefaultShader();
   void    DefaultShader(QString const&);
    
   QVariantMap DefaultShaderParameters();
   void        DefaultShaderParameters(QVariantMap const&);

   QVariantMap DefaultFilterParameters();
   void        DefaultFilterParameters(QVariantMap const&);

   QVariantMap DefaultPovRayParameters();
   void        DefaultPovRayParameters(QVariantMap const&);

   QString QChemDatabaseFilePath();
   void    QChemDatabaseFilePath(QString const&);

   QString LogFilePath();
   void    LogFilePath(QString const&);

   bool    LogFileHidden();
   void    LogFileHidden(bool const);
   
   bool    LoggingEnabled();
   void    LoggingEnabled(bool const);
   
   double  SurfaceOpacity();
   void    SurfaceOpacity(double const);
   
   QString DefaultForceField();
   void    DefaultForceField(QString const&);

   double  SymmetryTolerance();
   void    SymmetryTolerance(double const);
   
   QColor PositiveSurfaceColor();
   void   PositiveSurfaceColor(QColor const&);
   
   QColor NegativeSurfaceColor();
   void   NegativeSurfaceColor(QColor const&);

   QList<QColor> SpectrumGradientColors();
   QList<QColor> DefaultGradientColors();
   QList<QColor> CustomGradientColors();
   void CustomGradientColors(QList<QColor> const&);
   
   QColor BackgroundColor();
   void   BackgroundColor(QColor const&);
   
   QColor ForegroundColor();
   void   ForegroundColor(QColor const&);
   
   QColor VibrationVectorColor();
   void   VibrationVectorColor(QColor const&);

   QStringList RecentFiles();
   void AddRecentFile(QString const&);
   void RemoveRecentFile(QString const&);
   void ClearRecentFiles();

   /// These are probably not what you want.  User the static member functions
   /// of the Server class to get an actual list of Servers.
   QVariantList ServerList();  // deprecate
   void ServerList(QVariantList const&);  // deprecate

   QVariantList ServerConfigurationList();
   void ServerConfigurationList(QVariantList const&);

   QString ServerQueryJobFinished();
   void    ServerQueryJobFinished(QString const& repsonse);

   int  DaysToRememberJobs();
   void DaysToRememberJobs(int const& numberOfDays);
   
   QMap<QString,QString> PasswordVaultContents();
   void PasswordVaultContents(QMap<QString,QString> const&);
   void ClearPasswordVaultContents();

   unsigned int PasswordVaultSeed();
   void PasswordVaultSeed(unsigned int const);

   QString PasswordVaultKey();
   void PasswordVaultKey(QString const&);
   
   QString SSHKnownHostsFile();
   void SSHKnownHostsFile(QString const&);

   QString SSHPublicIdentityFile();
   void SSHPublicIdentityFile(QString const&);

   QString SSHPrivateIdentityFile();
   void SSHPrivateIdentityFile(QString const&);

   QList<QVariant> JobMonitorList();
   void JobMonitorList(QList<QVariant> const&);

   // Deprecate
   QList<QVariant> CurrentProcessList();
   void CurrentProcessList(QList<QVariant> const&);

   QSize   QuiWindowSize();
   void    QuiWindowSize(QSize const&);
   
   QSize   FileDisplayWindowSize();
   void    FileDisplayWindowSize(QSize const&);
   
   QFont   PreviewFont();
   void    PreviewFont(QFont const&);
   
   QFont   FileDisplayFont();
   void    FileDisplayFont(QFont const&);
   
   QString TemplateDirectory();
   void    TemplateDirectory(QString const&);
   
} } // end namespace IQmol::Preferences

#endif
