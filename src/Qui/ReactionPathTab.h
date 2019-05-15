#ifndef QUI_REACTIONPATHTAB_H
#define QUI_REACTIONPATHTAB_H

/*!
 *  \file ReactionPathTab.h 
 *  
 *  \brief contains class definitions associated with the Reaction Path tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_ReactionPathTab.h"


namespace Qui {

class ReactionPathTab : public QWidget {

   Q_OBJECT

   public:
      ReactionPathTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::ReactionPathTab m_ui;
};

} // end namespaces Qui

#endif
