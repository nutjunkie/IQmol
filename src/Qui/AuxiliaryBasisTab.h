#ifndef QUI_AUXILIARYBASISTAB_H
#define QUI_AUXILIARYBASISTAB_H

/*!
 *  \file AuxiliaryBasis.h 
 *  
 *  \brief contains class definitions associated with the Auxiliary Basis 
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_AuxiliaryBasisTab.h"


namespace Qui {

class AuxiliaryBasisTab : public QWidget {

   Q_OBJECT

   public:
      AuxiliaryBasisTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::AuxiliaryBasisTab m_ui;
};

} // end namespaces Qui

#endif
