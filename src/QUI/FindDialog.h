#ifndef QUI_FINDDIALOG_H 
#define QUI_FINDDIALOG_H 
/*!
 *  \class FindDialog
 *  
 *  \brief A very simple dialog class for searching text.
 *  
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "ui_FindDialog.h"
#include <QString>


namespace Qui {

class FindDialog : public QDialog {

   Q_OBJECT

   public:
      FindDialog(QWidget* parent, QString const& text = "", 
         bool caseSensitive = false);
      ~FindDialog() { }

      bool caseSensitive();
      QString searchText();

   Q_SIGNALS:
      void findNext();
      void findPrevious();
      void caseSensitivityChanged(int);
      void searchTextChanged(QString const&);

   private Q_SLOTS:
      void on_nextButton_clicked(bool);
      void on_previousButton_clicked(bool);
      void on_caseCheckBox_stateChanged(int);
      void on_searchText_textEdited(QString const&);
      void found(bool);

   private:
      Ui::FindDialog m_ui;
};


} // end namespace Qui

#endif
