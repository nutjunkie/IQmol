#ifndef QUI_AIMDTAB_H
#define QUI_AIMDTAB_H

/*!
 *  \file AimdTab.h 
 *  
 *  \brief contains class definitions associated with the AIMDtab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_AimdTab.h"


namespace Qui {

class AimdTab : public QWidget {

   Q_OBJECT

   public:
      AimdTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::AimdTab m_ui;
};

} // end namespaces Qui

#endif
