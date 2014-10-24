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

#include "ServerListDialog.h"
#include "ServerRegistry.h"
#include "ServerDialog.h"
#include "Server.h"
#include "QMsgBox.h"
#include <QHeaderView>


namespace IQmol {

ServerListDialog::ServerListDialog(QWidget* parent) : QDialog(parent)
{
   m_dialog.setupUi(this);
   QTableWidget* table(m_dialog.serverListTable);
#if QT_VERSION >= 0x050000
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
   table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
   updateServerTable();
}


void ServerListDialog::updateServerTable()
{
   ServerRegistry& serverRegistry(ServerRegistry::instance());
   QStringList serverNames(serverRegistry.availableServers());

   QTableWidget* table(m_dialog.serverListTable);
   table->clearContents();
   table->setRowCount(serverNames.size());

   QTableWidgetItem* field;
   QStringList fields;
   Server* server;

   for (int row = 0; row < serverNames.size(); ++row) {
       server = serverRegistry.get(serverNames[row]);
       if (server) {
          fields = server->tableFields();
          for (int col = 0; col < fields.size() && col < table->columnCount(); ++col) {
              field = new QTableWidgetItem(fields[col]);
              table->setItem(row, col, field);
          }
       }
   }

   serverRegistry.saveToPreferences();
}


void ServerListDialog::on_addServerButton_clicked(bool)
{
   editServer(0);
   updateServerTable();
}


void ServerListDialog::on_serverListTable_cellDoubleClicked(int row, int)
{
   QTableWidgetItem* item(m_dialog.serverListTable->item(row,0));
   Server* server(ServerRegistry::instance().get(item->text()));
   if (!server) return;

   editServer(server);
   updateServerTable();
}


void ServerListDialog::editServer(Server* server)
{
   // Note the scoping of the dialog is important here.  The dtor needs to be
   // called before updateServerTable is called.
   ServerDialog dialog(this, server);
   dialog.setWindowModality(Qt::WindowModal);
   dialog.exec();
}


void ServerListDialog::on_removeServerButton_clicked(bool)
{
   // This relies on the fact that only one row can be selected at a time.
   QList<QTableWidgetItem*> selected(m_dialog.serverListTable->selectedItems());
   if (!selected.isEmpty()) {
      QString serverName(selected[0]->text());
      QString msg("Are you sure you want to delete the server ");
      msg += serverName;
      if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Ok) {
         ServerRegistry::instance().remove(serverName);
         updateServerTable();
      }
   }
}


void ServerListDialog::on_configureServerButton_clicked(bool)
{
   QList<QTableWidgetItem*> selected(m_dialog.serverListTable->selectedItems());
   if (!selected.isEmpty()) {
      int row(selected[0]->row());
      on_serverListTable_cellDoubleClicked(row, 0);
   }
}


void ServerListDialog::on_upButton_clicked(bool)
{
   QTableWidget* table(m_dialog.serverListTable);
   QList<QTableWidgetItem*> items(table->selectedItems());
   if (items.isEmpty()) return;

   QTableWidgetItem* item(items.first());
   int row(item->row());
   ServerRegistry::instance().moveUp(item->text());
   updateServerTable();

   if (row > 0) table->setCurrentItem(table->item(row-1,0));
}


void ServerListDialog::on_downButton_clicked(bool)
{
   QTableWidget* table(m_dialog.serverListTable);
   QList<QTableWidgetItem*> items(table->selectedItems());
   if (items.isEmpty()) return;

   QTableWidgetItem* item(items.first());
   int row(item->row());
   ServerRegistry::instance().moveDown(item->text());
   updateServerTable();

   if (row < table->rowCount()-1) table->setCurrentItem(table->item(row+1,0));
}
  

} // end namespace IQmol
