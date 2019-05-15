#ifndef QUI_ATTENUATIONPARAMETERTAB_H
#define QUI_ATTENUATIONPARAMETERTAB_H

/*!
 *  \file AttenuationParameterTab.h 
 *  
 *  \brief contains class definitions associated with the Attenuation Parameter
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_AttenuationParameterTab.h"


namespace Qui {

class AttenuationParameterTab : public QWidget {

   Q_OBJECT

   public:
      AttenuationParameterTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::AttenuationParameterTab m_ui;
};

} // end namespaces Qui

#endif
