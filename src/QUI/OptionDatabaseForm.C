/*!
 *  \class OptionDatabaseForm is the main input dialog for accessing the
 *  OptionDatase
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include <QApplication>
#include "OptionDatabaseForm.h"
#include "OptionDatabase.h"
#include "Option.h"
#include "OptionEditors.h"

#include <iostream> // tmp


namespace Qui {

OptionDatabaseForm::OptionDatabaseForm(QWidget* parent) : 
   QWidget(parent), m_taint(false) {

   ui.setupUi(this);
   ui.optionList->setSortingEnabled(true);

   QStringList all(OptionDatabase::instance().all());

   for (int i = 0; i < all.size(); ++i) {
       QListWidgetItem* item = new QListWidgetItem(all[i], ui.optionList);
       item->setFlags(Qt::ItemIsEnabled  | Qt::ItemIsSelectable | 
                      Qt::ItemIsEditable | Qt::ItemIsDragEnabled );
   }

   if (all.size() > 0) {
      ui.optionList->setCurrentItem(0);
   }else {
      enableControls(false);
   }

   // Crass, for the moment we just accept everything
   intVal = new QIntValidator(-99999, 99999, this);
   doubleVal = new QDoubleValidator(-99999.9, 99999.9, 4, this);

   setWindowTitle(tr("Option Database Editor"));
}



void OptionDatabaseForm::on_quitButton_clicked(bool) {
   commit(ui.optionList->currentItem()->text());
   QApplication::exit();
}



void OptionDatabaseForm::commit(QString const& name) {
   if (m_taint) {
      Option opt(name);
      formToOption(opt);
      OptionDatabase::instance().insert(opt,false);
      m_taint = false;
   }
}



void OptionDatabaseForm::on_deleteButton_clicked(bool) {
   m_taint = false;
   QList<QListWidgetItem*> items = ui.optionList->selectedItems();
   for (int i = 0; i < items.size(); ++i) {
      OptionDatabase::instance().remove(items[i]->text());
      delete ui.optionList->takeItem(ui.optionList->row(items[i]));
   }
}



void OptionDatabaseForm::on_addButton_clicked(bool) {
   if (ui.optionList->currentItem()) commit(ui.optionList->currentItem()->text());
   QListWidgetItem* item = new QListWidgetItem("NEW_OPTION", ui.optionList);
   item->setFlags(Qt::ItemIsEnabled  | Qt::ItemIsSelectable | 
                  Qt::ItemIsEditable | Qt::ItemIsDragEnabled );
   ui.optionList->setCurrentItem(item);
   editName(item);
   clearControls();
   enableControls(true);
}



void OptionDatabaseForm::on_optionList_itemDoubleClicked(QListWidgetItem* item) {
   //std::cout << "on_optionList_itemDoubleClicked" << std::endl;
   QString oldName(item->text());
   editName(item);
   if (item->text() != oldName) {
      OptionDatabase::instance().remove(oldName,false);
   }
   m_taint = true;
}



void OptionDatabaseForm::editName(QListWidgetItem* item) {
   ui.optionList->editItem(item);
   item->setText(item->text().toUpper());
   m_taint = true;
}


//! This allows us to grab the currently displayed data and send it to the
//! database before overwriting it with the new Option data.
void OptionDatabaseForm::on_optionList_currentItemChanged(QListWidgetItem*, 
   QListWidgetItem* previous) {
   //std::cout << "on_optionList_currentItemChanged" << std::endl;
   if (previous) commit(previous->text());
}



void OptionDatabaseForm::on_optionList_itemSelectionChanged() {
   //std::cout << "on_optionList_itemSelectionChanged" << std::endl;
   QListWidgetItem* item = ui.optionList->currentItem();
      
   if (item) {
      QString name(item->text());
      Option opt;
      if (OptionDatabase::instance().get(name, opt)) {
         enableControls(true);
         optionToForm(opt);
      }
   }
}



void OptionDatabaseForm::on_editButton_clicked(bool) {
   QStringList opts;
   int n = ui.optionsCombo->count();

   for (int i = 0; i < n; ++i) {
       opts.append(ui.optionsCombo->itemText(i)); 
   }

   QString defval;
   if (n > 0 && m_default > 0) defval = opts[m_default];

   int impl = ui.widgetCombo->currentIndex();
   QDialog* d;

   if (impl == Option::Impl_Spin) {
      d = new OptionNumberEditor(this, &opts, intVal);
   }else if (impl == Option::Impl_DSpin) {
      d = new OptionNumberEditor(this, &opts, doubleVal);
   }else {
      d = new OptionListEditor(this, &opts);
   }

   if (d->exec() == QDialog::Accepted) {
      ui.optionsCombo->clear();
      ui.optionsCombo->addItems(opts);
      m_taint = true;
      if (impl == Option::Impl_Spin || impl == Option::Impl_DSpin) {
         m_default = 2;
      }else {
         m_default = opts.indexOf(defval);
      }
      if (m_default < 0) m_default = 0;
      ui.optionsCombo->setCurrentIndex(m_default);
   }
}



void OptionDatabaseForm::on_typeCombo_currentIndexChanged(int type) { 

   if (type == Option::Type_Logical) {
      m_backup.clear(); 
      for (int i = 0; i < ui.optionsCombo->count(); ++i) {
          m_backup.append(ui.optionsCombo->itemText(i)); 
      }
      ui.optionsCombo->clear();
      ui.optionsCombo->addItem("FALSE");
      ui.optionsCombo->addItem("TRUE");
      ui.editButton->setEnabled(false);

   }else {
      if (m_backup.count() > 0) {
         ui.optionsCombo->clear();
         ui.optionsCombo->addItems(m_backup);
         m_backup.clear(); 
      }
      ui.editButton->setEnabled(true);
   }

   switch (type) {
      case Option::Type_Integer:
         ui.widgetCombo->setCurrentIndex(Option::Impl_Spin);
         break;
      case Option::Type_Logical:
         ui.widgetCombo->setCurrentIndex(Option::Impl_Check);
         break;
      case Option::Type_String:
         ui.widgetCombo->setCurrentIndex(Option::Impl_Combo);
         break;
      case Option::Type_Real:
         ui.widgetCombo->setCurrentIndex(Option::Impl_DSpin);
         break;
      case Option::Type_Array:
         ui.widgetCombo->setCurrentIndex(Option::Impl_Text);
         break;
   }

   m_taint = true; 
}



void OptionDatabaseForm::on_descriptionText_textChanged() { 
   m_taint = true; 
}



void OptionDatabaseForm::on_widgetCombo_currentIndexChanged(int) { 
  m_taint = true; 
}


void OptionDatabaseForm::on_optionsCombo_currentIndexChanged(int) { 
   m_default = ui.optionsCombo->currentIndex();
   m_taint = true;
}



// This transfers all the data from the Option to the form, except the name
void OptionDatabaseForm::optionToForm(Option const& option) {
   ui.typeCombo->setCurrentIndex(option.getType());
   ui.widgetCombo->setCurrentIndex(option.getImplementation());
   ui.descriptionText->setHtml(option.getDescription());
   ui.optionsCombo->clear();
   ui.optionsCombo->addItems(option.getOptions());
   m_default = option.getDefaultIndex();
   ui.optionsCombo->setCurrentIndex(m_default);

   enableControls(true);
   if (option.getType() == Option::Type_Logical) ui.editButton->setEnabled(false);
   m_backup.clear(); 
   m_taint = false;
}



// This transfers all the data from the form to the Option, except the name
void OptionDatabaseForm::formToOption(Option& option) {
   option.setType(ui.typeCombo->currentIndex());
   option.setDefault(ui.optionsCombo->currentIndex());
   option.setImplementation(ui.widgetCombo->currentIndex());
std::cout << "setting widget to " << option.getImplementation() << std::endl;

   // toHtml protects the <> tags by turning them into &lt; and &gt;
   // so we need to undo this so that next time it displays correctly.
   QString tmp = ui.descriptionText->toHtml();
   tmp.replace(QString("&lt;"), QString("<"));
   tmp.replace(QString("&gt;"), QString(">"));
   tmp.replace(QString("&amp;"), QString("&"));
   option.setDescription(tmp);

   // Only true/false allowed for logicals, and the options should always be in
   // the order false:true (this is used in other parts of the code).  
   QStringList opts;
   if (option.getType() == Option::Type_Logical) {
         
      QString t(ui.optionsCombo->currentText().toUpper());
      
      if (t.contains("TRUE") || t.contains("1")) {
         option.setDefault(1);   
      }else {
         option.setDefault(0);   
      }

      opts.append("FALSE"); 
      opts.append("TRUE"); 
   }else {
      for (int i = 0; i < ui.optionsCombo->count(); ++i) {
          opts.append(ui.optionsCombo->itemText(i)); 
      }
   }

   option.setOptions(opts);
   m_taint = false;
}



void OptionDatabaseForm::clearControls() {
   ui.descriptionText->clear();
   ui.optionsCombo->clear();
   ui.widgetCombo->setCurrentIndex(0);
}



void OptionDatabaseForm::enableControls(bool enable) {
   ui.typeCombo->setEnabled(enable);
   ui.widgetCombo->setEnabled(enable);
   ui.optionsCombo->setEnabled(enable);
   ui.editButton->setEnabled(enable);
   ui.descriptionText->setEnabled(enable);
}

} // end namespace Qui
