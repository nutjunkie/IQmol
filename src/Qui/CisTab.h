#ifndef QUI_CISTAB_H
#define QUI_CISTAB_H

/*!
 *  \file CISTab.h 
 *  
 *  \brief contains class definitions associated with the CIS/TDDFT tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_CisTab.h"


namespace Qui {

class CisTab : public QWidget {

   Q_OBJECT

   public:
      CisTab(QWidget* parent) : QWidget(parent) {
          m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::CisTab m_ui;
};

} // end namespaces Qui

#endif
