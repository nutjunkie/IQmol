#ifndef QUIEXTENSION_H
#define QUIEXTENSION_H
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

#include <QtGui/QAction>
#include <avogadro/glwidget.h>
#include <avogadro/extension.h>


namespace Qui {
   class InputDialog;
}

namespace Avogadro
{
  class QChemExtension : public Extension
  {
  Q_OBJECT

  public:
    QChemExtension(QObject* parent = 0);
    virtual ~QChemExtension();

    virtual QString name() const { return QObject::tr("Q-Chem Input Deck"); }
    virtual QString description() const
    {
      return QObject::tr("Q-Chem input deck generator");
    }

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

  private:
    Qui::InputDialog* m_qchemInputDialog;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
  };

  class QChemExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(QChemExtension,
        tr("Q-Chem Extension"),
        tr("Extension for creating input files for the Q-Chem"
          " quantum chemistry package."))
 
  };

} // End namespace Avogadro

#endif
