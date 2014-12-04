#ifndef QUI_OPTIONEDITOR_H
#define QUI_OPTIONEDITOR_H
/*!
 *  \file OptionEditors.h  
 *  
 *  \brief Dialogs used for editing options in the database.
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "ui_OptionNumberEditor.h"
#include "ui_OptionListEditor.h"


namespace Qui {

//!  \class OptionListEditor allows a list of options to be edited.
class OptionListEditor : public QDialog {

   Q_OBJECT

   public:
      OptionListEditor(QWidget* parent, QStringList* list);

   private Q_SLOTS:
      void on_addButton_clicked(bool);
      void on_deleteButton_clicked(bool);
      void on_upButton_clicked(bool);
      void on_downButton_clicked(bool);
      void on_cancelButton_clicked(bool);
      void on_okButton_clicked(bool);
      void on_optionList_itemDoubleClicked(QListWidgetItem* item);

   private:
      Ui::OptionListEditor ui;
      QStringList* m_list;
};



//!  \class OptionNumberEditor edits the parameters for a numericl option
class OptionNumberEditor : public QDialog {

   Q_OBJECT

   public:
      OptionNumberEditor(QWidget* parent, QStringList* list, QValidator* validator);

   private Q_SLOTS:
      void on_okButton_clicked(bool);
      void on_cancelButton_clicked(bool);

   private:
      Ui::OptionNumberEditor ui;
      QStringList* m_list;
};

} // end namespace Qui

#endif
