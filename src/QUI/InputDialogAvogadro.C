/*!
 *  \file InputDialogAvogadro.C 
 *  
 *  \brief This file contains extensions to the InputDialog class that are only
 *  relevant when the QUI is compiled as an Avogadro extension
 *  
 *  \author Andrew Gilbert
 *  \date   November 2008
 */

#ifndef AVOGADRO
#error Macro AVOGADRO not defined in InputDialogAvogadro.C
#endif


#include <QTextStream>
#include <QtDebug>
#include "InputDialog.h"
#include "Job.h"
#include <avogadro/glwidget.h>  // Hack, no?  This appears to be a catch-all include


using namespace Avogadro;
using namespace OpenBabel;


namespace Qui {


void InputDialog::setMolecule(Avogadro::Molecule* molecule) {
   // Disconnect the old molecule first...
   if (m_molecule) disconnect(m_molecule, 0, this, 0);

   if (molecule) {
      m_molecule = molecule;
      // Update the preview text whenever primitives are changed
      connect(m_molecule, SIGNAL(primitiveRemoved(Primitive *)),
           this, SLOT(updatePreviewText()));
      connect(m_molecule, SIGNAL(primitiveAdded(Primitive *)),
           this, SLOT(updatePreviewText()));
      connect(m_molecule, SIGNAL(primitiveUpdated(Primitive *)),
           this, SLOT(updatePreviewText()));
      updatePreviewText();
   }
}


} // end namespace Qui
