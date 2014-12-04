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

#include "QueueOptionsDialog.h"
#include "ServerConfiguration.h"
#include <QDebug>


namespace IQmol {
namespace Process2 {

QueueOptionsDialog::QueueOptionsDialog(ServerConfiguration* configuration, QWidget* parent) : 
   QDialog(parent),  m_configuration(configuration)
{
   m_dialog.setupUi(this);

   m_dialog.submit->setText(
      m_configuration->value(ServerConfiguration::Submit));

   m_dialog.query->setText(
      m_configuration->value(ServerConfiguration::Query));

   m_dialog.queueInfo->setText(
      m_configuration->value(ServerConfiguration::QueueInfo));

   m_dialog.jobFileList->setText(
      m_configuration->value(ServerConfiguration::JobFileList));

   m_dialog.kill->setText(
      m_configuration->value(ServerConfiguration::Kill));

   ServerConfiguration::QueueSystemT queue(m_configuration->queueSystem());

   m_dialog.runFileTemplate->setText(
      m_configuration->value(ServerConfiguration::RunFileTemplate));

   m_dialog.updateInterval->setValue(
      m_configuration->updateInterval());

   if (queue == ServerConfiguration::Web) {
      m_dialog.queueInfoLabel->setText("Download");
      m_dialog.runFileGroupBox->setEnabled(false);
   }

   if (queue == ServerConfiguration::Basic) {
      m_dialog.jobLimit->setValue(
         m_configuration->value(ServerConfiguration::JobLimit).toInt());
   }else {
      m_dialog.jobLimitLabel->setVisible(false);
      m_dialog.jobLimit->setVisible(false);
   }


#ifdef Q_OS_WIN32
      m_dialog.runFileGroupBox->setEnabled(
         m_configuration->connection() != ServerConfiguration::Local);
#endif

   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(copyToServer()));
}


void QueueOptionsDialog::copyToServer()
{
   m_configuration->setValue(ServerConfiguration::Submit,
      m_dialog.submit->text());

   m_configuration->setValue(ServerConfiguration::Query,
      m_dialog.query->text());

   m_configuration->setValue(ServerConfiguration::Kill,
      m_dialog.kill->text());

   m_configuration->setValue(ServerConfiguration::JobFileList,
      m_dialog.jobFileList->text());

   m_configuration->setValue(ServerConfiguration::QueueInfo,
      m_dialog.queueInfo->text());

   m_configuration->setValue(ServerConfiguration::RunFileTemplate,
      m_dialog.runFileTemplate->toPlainText());

   m_configuration->setValue(ServerConfiguration::UpdateInterval,
      m_dialog.updateInterval->value());

   m_configuration->setValue(ServerConfiguration::JobLimit,
      m_dialog.jobLimit->value());

   accept();
}

} } // end namespace IQmol::Process
