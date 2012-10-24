#ifndef QUI_QUIPREFERENCES_H
#define QUI_QUIPREFERENCES_H

/*!
 *  \file Preferences.h 
 *
 *  \brief Contains definitions for classes that deal with program
 *  preferences.  
 *
 *  The idea is that all preference requests should go through the Preferences
 *  interface so that this is the only place where the QSettings object is
 *  changed.  This means that if a developer wants to check an option they only
 *  need to reference this header file (and the corresponding source file),
 *  which helps prevent duplication of preferences.
 *
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "ui_PreferencesBrowser.h"


namespace Qui {
namespace Preferences {


//! \class Browser is a class for displaying and editing the user
//! configurable preferences.
class Browser : public QDialog {

   Q_OBJECT

   public:
      Browser(QWidget* parent);
      ~Browser() { }

   private Q_SLOTS:
      void on_okButton_clicked(bool);
      void on_browseRunQChemButton_clicked(bool);
      void on_browseTemplateDirectoryButton_clicked(bool);

   private:
      Ui::PreferencesBrowser m_ui;

      void setPath(QLineEdit* edit);
      void setFilePath(QLineEdit* edit);
};



// Non-member functions used to access the preferences.  This should be the
// only way preferences are accessed in the program.
QSize   QuiWindowSize();
void    QuiWindowSize(QSize const&);

QSize   FileDisplayWindowSize();
void    FileDisplayWindowSize(QSize const&);

QFont   PreviewFont();
void    PreviewFont(QFont const&);

QFont   FileDisplayFont();
void    FileDisplayFont(QFont const&);

QString LastFileAccessed();
void    LastFileAccessed(QString const&);

QString TemplateDirectory();
void    TemplateDirectory(QString const&);

QString QChemRunScript();
void    QChemRunScript(QString const&);

int     NumberOfProcesses();
void    NumberOfProcesses(int);


// These functions are generic and should only be used within the Preferences
// module and not in the general code.
QVariant Get(QString const& name);
void     Set(QString const& name, QVariant const& value);


} } // end namespace Qui::Preferences

#endif
