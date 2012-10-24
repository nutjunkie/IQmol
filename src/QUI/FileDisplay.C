/*!
 *  \file FileDisplay.C
 *  
 *  \brief See FileDisplay.h for details.
 *  
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "FileDisplay.h"
#include "FindDialog.h"
#include <QFile>
#include <QTimer>
#include <QMenuBar>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QResizeEvent>
#include <limits>

#include <QtDebug>

#ifdef QCHEM_UI
#include "../Preferences.h"
#define Preferences IQmol::Preferences
#else
#include "Preferences.h"
#endif



namespace Qui {

FileDisplay::FileDisplay(QWidget* parent, QString const& fileName, int interval)  
   : QMainWindow(parent), m_file(0), m_timer(0), m_findDialog(0), m_searchText(""),
     m_caseSensitive(false) {

   m_ui.setupUi(this);
   initializeMenus();
   m_ui.textDisplay->setCurrentFont(Preferences::FileDisplayFont());
   resize(Preferences::FileDisplayWindowSize());
   setStatusBar(0);

   m_timer = new QTimer(this);

   if (interval == 0) {
      m_timer->setInterval(INT_MAX);
   }else { 
      m_timer->setInterval(interval);
   }

   connect(m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
   openFile(fileName);
}


FileDisplay::~FileDisplay() {
   m_timer->stop();
   delete m_timer;
   if (m_file) {
      m_file->close();
      delete m_file;
   }
}



void FileDisplay::initializeMenus() {
   QMenuBar* menubar(menuBar());
   QAction*  action;
   QMenu*    menu;

   menubar->clear();

   // File
   menu = menubar->addMenu(tr("File"));

/*
   // File -> Open
   action = menu->addAction(tr("Open"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuOpen()));
   action->setShortcut(QKeySequence::Open);
*/

   // File -> Close
   action = menu->addAction(tr("Close"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuClose()));
   action->setShortcut(QKeySequence::Close);


   // Edit
   menu = menubar->addMenu(tr("Edit"));

   // Edit -> Copy
   action = menu->addAction(tr("Copy"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuCopy()));
   action->setShortcut(QKeySequence::Copy);

   // Edit -> Find
   action = menu->addAction(tr("Find"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuFind()));
   action->setShortcut(QKeySequence::Find);


   // Font
   menu = menubar->addMenu(tr("Font"));

   // Font -> Set Font
   action = menu->addAction(tr("Set Font"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuSetFont()));
   menu->addSeparator();
   
   // Font -> Bigger
   action = menu->addAction(tr("Bigger"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuBigger()));
   action->setShortcut(Qt::CTRL + Qt::Key_Plus);
   
   // Font -> Smaller
   action = menu->addAction(tr("Smaller"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuSmaller()));
   action->setShortcut(Qt::CTRL + Qt::Key_Minus);
}



// ********** Menu Slots ********** //

void FileDisplay::menuOpen() {
   QString file(QFileDialog::getOpenFileName(this, QString("Open File"),
       Preferences::LastFileAccessed()));
   if (!file.isEmpty()) openFile(file);
}


void FileDisplay::menuClose() {
  close();
  deleteLater();
}


void FileDisplay::menuCopy() {
   m_ui.textDisplay->copy();
}


void FileDisplay::menuFind() {
   if (!m_findDialog) {
      m_findDialog = new FindDialog(this, m_searchText, m_caseSensitive);

      connect(this, SIGNAL(searchTextFound(bool)),
         m_findDialog, SLOT(found(bool)));

      connect(m_findDialog, SIGNAL(findNext()), 
         this, SLOT(findNext()));
      connect(m_findDialog, SIGNAL(findPrevious()), 
         this, SLOT(findPrevious()));
      connect(m_findDialog, SIGNAL(caseSensitivityChanged(int)), 
         this, SLOT(caseSensitivityChanged(int)));
      connect(m_findDialog, SIGNAL(searchTextChanged(QString const&)), 
         this, SLOT(searchTextChanged(QString const&)));
   }

   m_findDialog->show();
   m_findDialog->raise();
   m_findDialog->activateWindow();
}


void FileDisplay::menuSetFont() {
   bool ok(false);
   QFont font(Preferences::FileDisplayFont());
   font = QFontDialog::getFont(&ok, font, this);
   if (ok) changeFont(font);
}


void FileDisplay::menuBigger() {
   QFont font(Preferences::FileDisplayFont());
   int size = font.pointSize() + 1;
   font.setPointSize(size);
   changeFont(font);
}


void FileDisplay::menuSmaller() {
   QFont font(Preferences::FileDisplayFont());
   int size = font.pointSize() - 1;
   font.setPointSize(size);
   changeFont(font);
}



// ********** Slots ********** //

void FileDisplay::refresh()  {
   if (m_file && m_file->isOpen()) {
      QString output(m_file->readAll());
      if (!output.isEmpty()) m_ui.textDisplay->append(output);
   }
}


void FileDisplay::findNext() {
   QTextDocument::FindFlags flags(0);
   if (m_caseSensitive) flags = QTextDocument::FindCaseSensitively;

   bool found(m_ui.textDisplay->find(m_searchText, flags));

   if (!found) {
      // Have another go from the start
      m_ui.textDisplay->moveCursor(QTextCursor::Start);
      found = m_ui.textDisplay->find(m_searchText, flags);
   }

   searchTextFound(found);
}
   

void FileDisplay::findPrevious() {
   QTextDocument::FindFlags flags(QTextDocument::FindBackward);
   if (m_caseSensitive) flags = flags | QTextDocument::FindCaseSensitively;

   bool found(m_ui.textDisplay->find(m_searchText, flags));

   if (!found) {
      // Have another go from the end
      m_ui.textDisplay->moveCursor(QTextCursor::End);
      found = m_ui.textDisplay->find(m_searchText, flags);
   }

   searchTextFound(found);
}



// **********  Non Slot Member Functions ********** //

void FileDisplay::changeFont(QFont const& font) {
   Preferences::FileDisplayFont(font);
   m_timer->stop();
   QString text(m_ui.textDisplay->toPlainText());
   m_ui.textDisplay->clear();
   m_ui.textDisplay->setCurrentFont(font);
   m_ui.textDisplay->setText(text);
   m_timer->start();
}



void FileDisplay::openFile(QString const& fileName) {

   if (m_file) {
      m_file->close();
      delete m_file;
   }

   m_file = new QFile(fileName);

   if (m_file->exists() && m_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_timer->stop();
      setWindowTitle(fileName);
      m_ui.textDisplay->clear();
      refresh();
      m_timer->start();
   }else {
      QString msg("Could not open text file for display\n");
      msg += fileName;
      QMessageBox::warning(0, "File Error ", msg);
   }
}

void FileDisplay::resizeEvent(QResizeEvent* event)  {
   Preferences::FileDisplayWindowSize(event->size());
}

} // end namespace Qui
