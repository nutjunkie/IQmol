/*!
 *  \file FindDialog.C
 *  
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "FindDialog.h"


namespace Qui {

FindDialog::FindDialog(QWidget* parent, QString const& text, bool caseSensitive) 
  : QDialog(parent)
{
   m_ui.setupUi(this);
   m_ui.notFound->hide();
   m_ui.caseCheckBox->setChecked(caseSensitive);
   m_ui.searchText->setText(text);
}


bool FindDialog::caseSensitive() 
{
   return m_ui.caseCheckBox->isChecked();
}


QString FindDialog::searchText() 
{
   return m_ui.searchText->text();
}


// **********  Slots  ********** //
void FindDialog::on_nextButton_clicked(bool) 
{ 
   findNext(); 
}

void FindDialog::on_previousButton_clicked(bool) 
{ 
   findPrevious(); 
}

void FindDialog::on_caseCheckBox_stateChanged(int state) 
{
   caseSensitivityChanged(state); 
}

void FindDialog::on_searchText_textEdited(QString const& text) 
{
   searchTextChanged(text); 
}

void FindDialog::found(bool found)
{
   m_ui.notFound->setVisible(!found);
}

} // end namespace Qui
