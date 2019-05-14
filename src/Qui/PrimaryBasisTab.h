#ifndef QUI_PRIMARYBASISTAB_H
#define QUI_PRIMARYBASISTAB_H

/*!
 *  \file PrimaryBasis.h 
 *  
 *  \brief contains class definitions associated with the Primary Basis 
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_PrimaryBasisTab.h"


namespace Qui {

class PrimaryBasisTab : public QWidget {

   Q_OBJECT

   public:
      PrimaryBasisTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::PrimaryBasisTab m_ui;
};

} // end namespaces Qui

#endif
