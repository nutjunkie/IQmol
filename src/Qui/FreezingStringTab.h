#ifndef QUI_FREEZINGSTRINGTAB_H
#define QUI_FREEZINGSTRINGTAB_H

/*!
 *  \file FreezingStringTab.h 
 *  
 *  \brief contains class definitions associated with the Freezing String tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_FreezingStringTab.h"


namespace Qui {

class FreezingStringTab : public QWidget {

   Q_OBJECT

   public:
      FreezingStringTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::FreezingStringTab m_ui;
};

} // end namespaces Qui

#endif
