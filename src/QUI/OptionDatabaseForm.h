#ifndef QUI_OPTIONDATABASEFORM_H
#define QUI_OPTIONDATABASEFORM_H
/*!
 *  \class OptionDatabaseForm 
 *  
 *  \brief The main input dialog for accessing the OptionDatase.
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "ui_OptionDatabaseForm.h"


namespace Qui {

class Option;

class OptionDatabaseForm : public QWidget {

   Q_OBJECT

   public:
      OptionDatabaseForm(QWidget* parent = 0);
      ~OptionDatabaseForm() { }
     
   private Q_SLOTS:
      void on_optionList_itemSelectionChanged();
      void on_optionList_itemDoubleClicked(QListWidgetItem* item);
      void on_optionList_currentItemChanged(QListWidgetItem*, QListWidgetItem*);
      void on_addButton_clicked(bool);
      void on_deleteButton_clicked(bool);
      void on_editButton_clicked(bool);
      void on_quitButton_clicked(bool);
      void on_descriptionText_textChanged();
      void on_optionsCombo_currentIndexChanged(int index);
      void on_widgetCombo_currentIndexChanged(int index);
      void on_typeCombo_currentIndexChanged(int index);


   private:
      Ui::OptionDatabaseForm ui;
      bool m_taint;
      int  m_default;
      QStringList m_backup;
      QIntValidator* intVal; 
      QDoubleValidator* doubleVal; 

      void editName(QListWidgetItem* item);
      void commit(QString const& name);
      void clearControls();
      void enableControls(bool enable);
      void formToOption(Option& option);
      void optionToForm(Option const& option);
};

} // end namespace Qui

#endif
