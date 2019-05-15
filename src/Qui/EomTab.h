#ifndef QUI_EOMTAB_H
#define QUI_EOMTAB_H

/*!
 *  \file EomTab.h 
 *  
 *  \brief contains class definitions associated with the EOM tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_EomTab.h"


namespace Qui {

class EomTab : public QWidget {

   Q_OBJECT

   public:
      EomTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::EomTab m_ui;
};

} // end namespaces Qui

#endif
