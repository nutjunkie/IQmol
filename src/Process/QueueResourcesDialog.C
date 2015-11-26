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

#include "QueueResourcesDialog.h"
#include "Preferences.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include <cmath>

#include <QDebug>


namespace IQmol {
namespace Process2 {

QueueResourcesDialog::QueueResourcesDialog(QueueResourcesList* queueResourcesList, 
   QWidget* parent) : QDialog(parent), m_queueResourcesList(queueResourcesList), 
   m_timeValidator(this)
{
   m_dialog.setupUi(this);
   m_dialog.queue->clear();

   if (m_queueResourcesList->isEmpty()) {
      QMsgBox::warning(this, "IQmol", "No queues found on server");
      close();
      return;
   }

   for (int i = 0; i < m_queueResourcesList->size(); ++i) {
       QString name((*m_queueResourcesList)[i]->m_name);
       m_dialog.queue->addItem(name);
   }

   m_dialog.queue->setCurrentIndex(0);
   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
   connect(m_dialog.buttonBox, SIGNAL(rejected()), this, SLOT(close()));

   QString sixty("(?:[0-5][0-9])");
   m_timeValidator.setRegExp(QRegExp("^\\d+:" + sixty + ":" + sixty));
   m_dialog.walltime->setValidator(&m_timeValidator);
}


void QueueResourcesDialog::verify()
{
   QString time(m_dialog.walltime->text());
   bool ok(false);
   int n(time.toInt(&ok));
   if (ok) time += ":00:00";
   m_dialog.walltime->setText(time);

   if (m_timeValidator.validate(time, n) != QValidator::Acceptable) {
      QMsgBox::warning(this, "IQmol", "Wall time must be in the format h:mm:ss");
      return;
   }

   QueueResources* currentQueue((*m_queueResourcesList)[m_dialog.queue->currentIndex()]);
   QStringList requestedTime(time.split(":", QString::SkipEmptyParts));

   time = currentQueue->m_maxWallTime;
   QStringList maxTime(time.split(":", QString::SkipEmptyParts));

   if (requestedTime.size() < 3 || maxTime.size() < 3) {
      QMsgBox::warning(this, "IQmol", "Validated wall time not in the format h:mm:ss");
      return;
   }

   int t1, t2;
   t1  = requestedTime[0].toInt(&ok) * 60 * 60;
   t1 += requestedTime[1].toInt(&ok) * 60;
   t1 += requestedTime[2].toInt(&ok);
   t2  = maxTime[0].toInt(&ok) * 60 * 60;
   t2 += maxTime[1].toInt(&ok) * 60;
   t2 += maxTime[2].toInt(&ok);

   if (t1 > t2) {
      QMsgBox::warning(this, "IQmol", "Wall time exceeds queue maximum of " + time);
      return;
   }else if (m_dialog.memory->value() > currentQueue->m_maxMemory) {
      QMsgBox::warning(this, "IQmol", "Memory exceeds queue limit of " 
         + QString::number(currentQueue->m_maxMemory));
      return;
   }else if (m_dialog.scratch->value() > currentQueue->m_maxScratch) {
      QMsgBox::warning(this, "IQmol", "Scratch exceeds queue limit of " 
         + QString::number(currentQueue->m_maxScratch));
      return;
   }else if (m_dialog.ncpus->value() > currentQueue->m_maxCpus) {
      QMsgBox::warning(this, "IQmol", "Number of CPUs exceeds queue limit of " 
         + QString::number(currentQueue->m_maxCpus));
      return;
   }

   saveAsDefaults();
   accept();
}


void QueueResourcesDialog::saveAsDefaults()
{
   qDebug() << "Saving QueueResource defaults";
   int index(m_dialog.queue->currentIndex());
   if (0 <= index && index < m_queueResourcesList->size()) {
       QueueResources* queue((*m_queueResourcesList)[index]);
       if (index != 0) {
          // rearrage the queues so the default appears 
          // first next time the list is loaded.
          m_queueResourcesList->removeAt(index);
          m_queueResourcesList->prepend(queue);
       }

       queue->m_defaultWallTime = m_dialog.walltime->text();
       queue->m_defaultMemory   = m_dialog.memory->value();
       queue->m_defaultScratch  = m_dialog.scratch->value();
       queue->m_defaultCpus     = m_dialog.ncpus->value();

   }else {
      QLOG_WARN() << "Unable to save defaults QueueResoures";
   }
}


void QueueResourcesDialog::on_queue_currentIndexChanged(int index)
{
   if (0 <= index && index < m_queueResourcesList->size()) {
      setQueueResources((*m_queueResourcesList)[index]);
   }
}


void QueueResourcesDialog::setQueueResources(QueueResources* queue)
{
   m_dialog.walltime->setText(queue->m_defaultWallTime);
   m_dialog.memory->setRange(queue->m_minMemory, queue->m_maxMemory);
   m_dialog.memory->setValue(queue->m_defaultMemory);
   m_dialog.scratch->setRange(queue->m_minScratch, queue->m_maxScratch);
   m_dialog.scratch->setValue(queue->m_defaultScratch);
   m_dialog.ncpus->setRange(queue->m_minCpus, queue->m_maxCpus);
   m_dialog.ncpus->setValue(queue->m_defaultCpus);
}

} } // end namespace IQmol::Process
