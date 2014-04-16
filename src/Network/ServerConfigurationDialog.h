#ifndef IQMOL_NETWORK_SERVERCONFIGURATIONDIALOG_H
#define IQMOL_NETWORK_SERVERCONFIGURATIONDIALOG_H
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

#include "ui_ServerConfigurationDialog.h"


namespace IQmol {
namespace Network {

   class ServerConfiguration;

   class ServerConfigurationDialog : public QDialog {

      Q_OBJECT

      public:
         ServerDialog(ServerConfiguration* config, QWidget* parent);
         ~ServerDialog();

      private Q_SLOTS:
         void on_localRadioButton_toggled(bool);
         void on_sshRadioButton_toggled(bool);
         void on_httpRadioButton_toggled(bool);
         void on_configureButton_clicked(bool);
         void on_configureSsh_clicked(bool);
         void on_testConnectButton_clicked(bool);
         void on_loadButton_clicked(bool);
         void verify();

      private:
         void testConnection();
         void copyFrom(ServerConfiguration*);
         bool copyTo(ServerConfiguration*);


         //  
         void setHttpProtocol(bool http);


         // Sets the server to have the default values 
         // appropriate for the current Host and Type
         void setCurrentDefaults(Server* server);

         ServerConfiguration* m_config;
         ServerConfiguration* m_tmpConfig;

         bool m_tested;
         bool m_accepted;
         bool m_blockSignals;
         bool m_closeAfterTest;
         Ui::ServerConfigurationDialog m_dialog;

		 // keeps track of what queue systems are available so that we don't
		 // reset unnecessarily.
         bool m_http;

         ServerConfiguration* m_originalConfiguration;
         ServerConfiguration* m_currentConfiguration;
   };

} } // end namespace IQmol::Network

#endif
