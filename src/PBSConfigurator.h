#ifndef IQMOL_PBSCONFIGURATOR_H
#define IQMOL_PBSCONFIGURATOR_H
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

#include "ui_PBSConfigurator.h"
#include "PBSQueue.h"
#include <QVariant>


namespace IQmol {

   class PBSConfigurator : public QDialog {

      Q_OBJECT

      public: 
         PBSConfigurator(QueueList const&, QVariantMap const& defaults, QWidget* parent = 0);
         QString queue() const { return m_dialog.queue->currentText(); }
         QString walltime() const { return m_dialog.walltime->text(); }
         int memory() const { return m_dialog.memory->value(); }
         int jobfs() const { return m_dialog.jobfs->value(); }
         int ncpus() const { return m_dialog.ncpus->value(); }
         bool saveAsDefaults() const { 
            return m_dialog.saveAsDefaults->checkState() == Qt::Checked;
         }

      private Q_SLOTS:
         void on_queue_currentIndexChanged(int);
         void verify();

      private:
         void setQueue(PBSQueue*);
         Ui::PBSConfigurator m_dialog;
         QueueList m_queues;
         QRegExpValidator m_timeValidator;
   };

} // end namespace IQmol


#endif
