#ifndef IQMOL_EDITSERVERDIALOG_H
#define IQMOL_EDITSERVERDIALOG_H
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

#include "ui_ServerDialog.h"


namespace IQmol {

   class Server;
   namespace ServerTask {
      class Base;
   }

   class ServerDialog : public QDialog {

      Q_OBJECT

      public:
         ServerDialog(QWidget* parent, Server* server = 0);
         ~ServerDialog();

      private Q_SLOTS:
         void on_localRadioButton_toggled(bool);
         void on_remoteRadioButton_toggled(bool);
         void on_typeCombo_currentIndexChanged(int);
         void on_configureTypeButton_clicked(bool);
         void on_authentication_currentIndexChanged(QString const&);
         void on_setPassword_clicked(bool);
         void on_configureSsh_clicked(bool);
         void on_testConfiguration_clicked(bool);

         void verify();
         void configurationTested();

      private:
         void testConfiguration();
         void copyFromServer(Server*);
         bool copyToServer(Server*);

         // Sets the server to have the default values 
         // appropriate for the current Host and Type
         void setCurrentDefaults(Server* server);

         Server* m_server;
         Server* m_tmpServer;
         bool m_tested;
         bool m_accepted;
         bool m_blockSignals;
         bool m_closeAfterTest;
         QString m_password;
         Ui::ServerDialog m_dialog;
         ServerTask::Base* m_task;
   };


} // end namespace IQmol


#endif
