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

#include "LogMessageDialog.h"
#include "Preferences.h"
#include <QFile>
#include <QDebug>


namespace IQmol {

LogMessageDialog::LogMessageDialog(QWidget* parent) : QWidget(parent), m_fileWatcher(parent),
   m_lastPosition(0), m_active(false), m_filter(QsLogging::InfoLevel)
{ 
   m_logMessageDialog.setupUi(this);
   m_fileWatcher.addPath(Preferences::LogFilePath());

   connect(&m_fileWatcher, SIGNAL(fileChanged(QString const&)),
      this, SLOT(update(QString const&)));
   connect(m_logMessageDialog.filterCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setFilter(int)));
}


void LogMessageDialog::display()
{
   if (!m_geometry.isNull()) setGeometry(m_geometry);
   m_active = true;
   update(Preferences::LogFilePath());
   show();
}


void LogMessageDialog::closeEvent(QCloseEvent* event)
{
   m_active = false;
   m_geometry = geometry();
   event->accept();
}


void LogMessageDialog::update(QString const& filePath)
{
   if (!m_active) return;

   if (!Preferences::LoggingEnabled()) {
      m_logMessageDialog.logTextBrowser->setText("Logging disabled");
      return;
   }

   QFile file(filePath);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      file.seek(m_lastPosition);
      QString text(file.readAll());
      if (!text.isEmpty()) {
         QRegExp rx("^.*@\\s\\d+\\s"); // Gets rid of file names and line numbers
         rx.setMinimal(true);
         QStringList lines(text.split(QRegExp("\\n"), QString::SkipEmptyParts));
         for (int i = 0; i < lines.size(); ++i) {
             text = lines[i].trimmed();
             if (messageLevel(text) >= m_filter) {
                if (m_filter > QsLogging::DebugLevel) text = text.replace(rx, "");
                m_logMessageDialog.logTextBrowser->append(text.prepend(" "));
             }
         }
         m_lastPosition = file.pos();
      }
      file.close(); 
   }else {
      QString msg("Could not read log file: ");
      msg += filePath;
      m_logMessageDialog.logTextBrowser->append(msg);
   }
}


void LogMessageDialog::setFilter(int level)
{
   m_lastPosition = 0; 
   m_logMessageDialog.logTextBrowser->clear();
   m_filter = (QsLogging::Level)level;
   update(Preferences::LogFilePath());
}


QsLogging::Level LogMessageDialog::messageLevel(QString const& msg)
{
   if        (msg.startsWith(QsLogging::TraceString)) {
      return QsLogging::TraceLevel;
   } else if (msg.startsWith(QsLogging::DebugString)) {
      return QsLogging::DebugLevel;
   } else if (msg.startsWith(QsLogging::InfoString)) {
      return QsLogging::InfoLevel;
   } else if (msg.startsWith(QsLogging::WarnString)) {
      return QsLogging::WarnLevel;
   } else if (msg.startsWith(QsLogging::ErrorString)) {
      return QsLogging::ErrorLevel;
   } else if (msg.startsWith(QsLogging::FatalString)) {
      return QsLogging::FatalLevel;
   }
   
   return  QsLogging::InfoLevel;
}
 

} // end namespace IQmol
