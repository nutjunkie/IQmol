/**********************************************************************
  QChemExtension - Extension for generating QChem input decks

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "QChemExtension.h"
#include "InputDialog.h"

#include <vector>
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QDebug>


using namespace std;
using namespace OpenBabel;
using namespace Eigen;


namespace Avogadro {

   QChemExtension::QChemExtension(QObject* parent) 
      : Extension(parent), m_qchemInputDialog(0), m_molecule(0) {
 
      QAction* action = new QAction(this);
      action->setText(tr("Q-Chem Input"));
      action->setData("QChem");
      m_actions.append(action);
   }


   QChemExtension::~QChemExtension() {
      delete m_qchemInputDialog;
      m_qchemInputDialog = 0;
   }


   QList<QAction *> QChemExtension::actions() const {
      return m_actions;
   }


   QString QChemExtension::menuPath(QAction*) const {
      return tr("&Extensions");
   }


   QUndoCommand* QChemExtension::performAction(QAction *action, GLWidget *) {
      if (action->data() == "QChem") {
         if (!m_qchemInputDialog) {
qDebug() << "Creating QChemInputDialog";
            m_qchemInputDialog = new Qui::InputDialog();
qDebug() << "Creating QChemInputDialog" << m_qchemInputDialog;
            m_qchemInputDialog->setMolecule(m_molecule);
qDebug() << "+=+ Setting molecule to" << m_molecule;
            m_qchemInputDialog->show();
         }else {
            m_qchemInputDialog->show();
         }
      }

      return 0;
   }


   void QChemExtension::setMolecule(Molecule *molecule) {
      m_molecule = molecule;
qDebug() << "Extension set Molecule" << molecule;
qDebug() << "       InputDialog set" << m_qchemInputDialog;
      if (m_qchemInputDialog)
         m_qchemInputDialog->setMolecule(m_molecule);
   }

} // End namespace Avogadro


Q_EXPORT_PLUGIN2(qchemextension, Avogadro::QChemExtensionFactory)
