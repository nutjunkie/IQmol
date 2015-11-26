#ifndef IQMOL_PROCESS_QUEUERESOURCESDIALOG_H
#define IQMOL_PROCESS_QUEUERESOURCESDIALOG_H
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

#include "ui_QueueResourcesDialog.h"
#include "QueueResources.h"
#include <QVariant>


namespace IQmol {
namespace Process2 {

   /// Dialog that allows the user to select which queue to submit a job to on
   /// a PBS or SGE server.
   class QueueResourcesDialog : public QDialog {

      Q_OBJECT

      public: 
         QueueResourcesDialog(QueueResourcesList*, QWidget* parent = 0);

         QString queue()    const { return m_dialog.queue->currentText(); }
         QString walltime() const { return m_dialog.walltime->text(); }
         int memory()       const { return m_dialog.memory->value(); }
         int scratch()      const { return m_dialog.scratch->value(); }
         int ncpus()        const { return m_dialog.ncpus->value(); }

      private Q_SLOTS:
         void on_queue_currentIndexChanged(int);
         void verify();

      private:
         void setQueueResources(QueueResources*);
         void saveAsDefaults();
         Ui::QueueResourcesDialog m_dialog;
         QueueResourcesList* m_queueResourcesList;
         QRegExpValidator m_timeValidator;
   };

} } // end namespace IQmol::Process

#endif
