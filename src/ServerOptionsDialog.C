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

#include "ServerOptionsDialog.h"
#include "Server.h"
#include "QMsgBox.h"


namespace IQmol {

ServerOptionsDialog::ServerOptionsDialog(QWidget* parent, Server* server) : QDialog(parent), 
   m_server(server)
{
   m_dialog.setupUi(this);

   m_dialog.exeName->setText(m_server->m_executableName);
   m_dialog.submitCommand->setText(m_server->m_submitCommand);
   m_dialog.queryCommand->setText(m_server->m_queryCommand);
   m_dialog.killCommand->setText(m_server->m_killCommand);
   m_dialog.runFileTemplate->setText(m_server->m_runFileTemplate);
   m_dialog.updateInterval->setValue(m_server->m_updateInterval);

   // Originally only Custom servers could edit these, but this
   // restriction has been removed.  // deprecate
   bool readOnly(m_server->m_type != Server::Custom);
   readOnly = false;
   m_dialog.exeName->setReadOnly(readOnly);
   m_dialog.submitCommand->setReadOnly(readOnly);
   m_dialog.queryCommand->setReadOnly(readOnly);
   m_dialog.killCommand->setReadOnly(readOnly);

   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(copyToServer()));
}


void ServerOptionsDialog::copyToServer()
{
   m_server->m_executableName  = m_dialog.exeName->text();
   m_server->m_submitCommand   = m_dialog.submitCommand->text();
   m_server->m_queryCommand    = m_dialog.queryCommand->text();
   m_server->m_killCommand     = m_dialog.killCommand->text();
   m_server->m_runFileTemplate = m_dialog.runFileTemplate->toPlainText();
   m_server->m_updateInterval  = m_dialog.updateInterval->value();
   accept();
}


} // end namespace IQmol
