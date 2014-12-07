/*!
 *  \file OptionListEditor.C
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include <QValidator>
#include "OptionEditors.h"
#include "Option.h"

#include <iostream> // tmp


namespace Qui {

/**********************************************************************
 *
 *   OptionListEditor
 *
 **********************************************************************/

OptionListEditor::OptionListEditor(QWidget* parent, QStringList* list) :
   QDialog(parent), m_list(list) {
   
   ui.setupUi(this);

   for (int i = 0; i < list->size(); ++i) {
       QListWidgetItem* item = new QListWidgetItem( (*list)[i], ui.optionList);
       item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
   }

   setWindowTitle(tr("Edit Option List"));
}



void OptionListEditor::on_addButton_clicked(bool) {
   QListWidgetItem* item = new QListWidgetItem(tr("New Option"));
   item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
   item->setText("New option");
   int row = ui.optionList->currentRow();
   if (row < 0) row = ui.optionList->count();

   ui.optionList->insertItem(row+1,item);
   ui.optionList->setCurrentItem(item);
   ui.optionList->editItem(item);
}



void OptionListEditor::on_deleteButton_clicked(bool) {
   QList<QListWidgetItem*> items = ui.optionList->selectedItems();
   for (int i = 0; i < items.size(); ++i) {
      delete ui.optionList->takeItem(ui.optionList->row(items[i]));
   }
}


void OptionListEditor::on_upButton_clicked(bool) {
   int row(ui.optionList->currentRow());
   QListWidgetItem* item;

   if (row > 0) {
      item = ui.optionList->takeItem(row);
      ui.optionList->insertItem(row-1,item);
      ui.optionList->setCurrentItem(item);
   }
}


void OptionListEditor::on_downButton_clicked(bool) {
   int row(ui.optionList->currentRow());
   int max(ui.optionList->count());
   QListWidgetItem* item;

   if (row < max-1) {
      item = ui.optionList->takeItem(row);
      ui.optionList->insertItem(row+1,item);
      ui.optionList->setCurrentItem(item);
   }
}


void OptionListEditor::on_okButton_clicked(bool) {
   m_list->clear();

   ui.optionList->setCurrentItem(ui.optionList->currentItem());
   
   for (int i = 0; i < ui.optionList->count(); ++i) {
      m_list->append( ui.optionList->item(i)->text());
   }
   done(QDialog::Accepted);
}


void OptionListEditor::on_cancelButton_clicked(bool) {
   done(QDialog::Rejected);
}


void OptionListEditor::on_optionList_itemDoubleClicked(QListWidgetItem* item) {
   ui.optionList->editItem(item);
}



/**********************************************************************
 *
 *   OptionNumberEditor
 *
 **********************************************************************/

OptionNumberEditor::OptionNumberEditor(QWidget* parent, QStringList* list, 
   QValidator* ) : QDialog(parent), m_list(list) {

   ui.setupUi(this);

   for (int i = m_list->count(); i < 4; ++i) {
       m_list->append("");
   }

   ui.minEdit->setText( (*m_list)[0]);
   ui.maxEdit->setText( (*m_list)[1]);
   ui.defaultEdit->setText( (*m_list)[2]);
   ui.stepEdit->setText( (*m_list)[3]);

   //ui.minEdit->setValidator(validator);
   //ui.maxEdit->setValidator(validator);
   //ui.stepEdit->setValidator(validator);
   //ui.defaultEdit->setValidator(validator);
}


void OptionNumberEditor::on_okButton_clicked(bool) {
   m_list->clear();
   m_list->append(ui.minEdit->text());
   m_list->append(ui.maxEdit->text());
   m_list->append(ui.defaultEdit->text());
   m_list->append(ui.stepEdit->text());
   done(QDialog::Accepted);
}


void OptionNumberEditor::on_cancelButton_clicked(bool) {
   done(QDialog::Rejected);
}

} // end namespace Qui
