#ifndef QUI_FILEDISPLAY_H 
#define QUI_FILEDISPLAY_H
/*!
 *  \class FileDisplay 
 *  
 *  \brief A very simple window for displaying the contents of a file.
 *  
 *  Note that the target file is re-read and the display is updated
 *  intermittently as specified by the interval argument (in msec).  If the
 *  interval is set to 0 then no update is performed (acutally one is performed
 *  every INT_MAX msecs, which should be at least 24 days).
 *  
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "ui_FileDisplay.h"

class QFile;
class QTimer;
class QResizeEvent;

namespace Qui {

class FindDialog;

class FileDisplay : public QMainWindow {

   Q_OBJECT

   public:
      FileDisplay(QWidget* parent, QString const& fileName, int interval = 2000);
      ~FileDisplay();

   Q_SIGNALS:
      void searchTextFound(bool);

   protected:
      void resizeEvent(QResizeEvent* event);

   private Q_SLOTS:
      void menuOpen();
      void menuClose();
      void menuCopy();
      void menuFind();
      void menuSetFont();
      void menuBigger();
      void menuSmaller();

      void refresh();
      void findNext();
      void findPrevious();
      void caseSensitivityChanged(int state) { m_caseSensitive = state; }
      void searchTextChanged(QString const& text) { m_searchText = text; }

   private:
      Ui::FileDisplay m_ui;
      QFile*  m_file;
      QTimer* m_timer;
      FindDialog* m_findDialog;
      QString m_searchText;
      bool m_caseSensitive;

      void openFile(QString const& fileName);
      void initializeMenus();
      void changeFont(QFont const& font);
};


} // end namespace Qui

#endif
