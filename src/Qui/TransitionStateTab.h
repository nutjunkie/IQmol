#ifndef QUI_TRANSITIONSTATETAB_H
#define QUI_TRANSITIONSTATETAB_H

/*!
 *  \file TransitionStateTab.h 
 *  
 *  \brief contains class definitions associated with the Transition State
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_TransitionStateTab.h"


namespace Qui {

class TransitionStateTab : public QWidget {

   Q_OBJECT

   public:
      TransitionStateTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::TransitionStateTab m_ui;
};

} // end namespaces Qui

#endif
