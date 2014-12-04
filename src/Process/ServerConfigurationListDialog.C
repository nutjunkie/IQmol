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

#include "ServerConfigurationListDialog.h"
#include "ServerRegistry2.h"
#include "Server2.h"
#include "ServerConfigurationDialog.h"
#include "QMsgBox.h"
#include <QHeaderView>
#include <QDebug>


namespace IQmol {
namespace Process2 {

ServerConfigurationListDialog::ServerConfigurationListDialog(QWidget* parent) : QDialog(parent)
{
   m_dialog.setupUi(this);
   QTableWidget* table(m_dialog.serverListTable);

#if QT_VERSION >= 0x050000
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#else
   table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#endif

   table->horizontalHeader()->setStretchLastSection(true);
   table->setColumnWidth(0,100);
   table->setColumnWidth(1,200);
   table->setColumnWidth(2, 70);
   table->setColumnWidth(3,100);

   updateServerTable();
}


void ServerConfigurationListDialog::updateServerTable()
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
       server = serverRegistry.find(serverNames[row]);
       if (server) {
          fields = server->tableFields();
          for (int col = 0; col < fields.size() && col < table->columnCount(); ++col) {
              field = new QTableWidgetItem(fields[col]);
              table->setItem(row, col, field);
          }
       }
   }
}


void ServerConfigurationListDialog::on_addServerButton_clicked(bool)
{
   ServerConfiguration config;
   if (editServerConfiguration(config)) {
      ServerRegistry::instance().addServer(config);
      updateServerTable();
   }
}


void ServerConfigurationListDialog::on_serverListTable_cellDoubleClicked(int row, int)
{
   QTableWidgetItem* item(m_dialog.serverListTable->item(row,0));
   Server* server(ServerRegistry::instance().find(item->text()));
   if (!server) return;

   if (editServerConfiguration(server->configuration()) ) {
      updateServerTable();
      ServerRegistry::save();
   }

qDebug() << "****************************************";
server->configuration().dump();
qDebug() << "****************************************";
}


bool ServerConfigurationListDialog::editServerConfiguration(ServerConfiguration& config)
{
   // Note the scoping of the dialog is important here.  The dtor needs to be
   // called before updateServerTable is called.
   ServerConfigurationDialog dialog(config, this);
   dialog.setWindowModality(Qt::WindowModal);
   return (dialog.exec() == QDialog::Accepted);
}


void ServerConfigurationListDialog::on_removeServerButton_clicked(bool)
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


void ServerConfigurationListDialog::on_configureServerButton_clicked(bool)
{
   QList<QTableWidgetItem*> selected(m_dialog.serverListTable->selectedItems());
   if (!selected.isEmpty()) {
      int row(selected[0]->row());
      on_serverListTable_cellDoubleClicked(row, 0);
   }
}


void ServerConfigurationListDialog::on_upButton_clicked(bool)
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


void ServerConfigurationListDialog::on_downButton_clicked(bool)
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
  

} } // end namespace IQmol::Process
