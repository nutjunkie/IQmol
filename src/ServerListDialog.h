#ifndef IQMOL_SERVERLISTDIALOG_H
#define IQMOL_SERVERLISTDIALOG_H
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

#include "ui_ServerListDialog.h"


namespace IQmol {

   class Server;

   /// Dialog that allows the user to configure the servers that are available
   /// for submitting calculations to.
   class ServerListDialog : public QDialog {

      Q_OBJECT

      public: 
         ServerListDialog(QWidget* parent = 0);

      private Q_SLOTS:
         void on_addServerButton_clicked(bool);
         void on_removeServerButton_clicked(bool);
         void on_configureServerButton_clicked(bool);
         void on_upButton_clicked(bool);
         void on_downButton_clicked(bool);
         void on_serverListTable_cellDoubleClicked(int row, int col);

      private:
         void updateServerTable();
         void editServer(Server*);

         Ui::ServerListDialog m_dialog;
   };

} // end namespace IQmol


#endif
