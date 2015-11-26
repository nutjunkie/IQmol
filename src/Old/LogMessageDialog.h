#ifndef IQMOL_LOGMESSAGEDIALOG_H
#define IQMOL_LOGMESSAGEDIALOG_H
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

#include "ui_LogMessageDialog.h"
#include "QsLogLevel.h"
#include <QRect>
#include <QCloseEvent>
#include <QFileSystemWatcher>


namespace IQmol {

   class LogMessageDialog : public QWidget {

      Q_OBJECT

      public:
         LogMessageDialog(QWidget* parent);
		 void display();
         bool isActive() const { return m_active; }

      protected Q_SLOTS:
         void closeEvent(QCloseEvent* event);

      private Q_SLOTS:
         void update(QString const& filePath);
         void setFilter(int);

      private:
         QsLogging::Level messageLevel(QString const&);
         Ui::LogMessageDialog m_logMessageDialog; 
         QFileSystemWatcher m_fileWatcher;
         QRect m_geometry;
         qint64 m_lastPosition;
         bool m_active;
         QsLogging::Level m_filter;
   };

} // end namespace IQmol

#endif
