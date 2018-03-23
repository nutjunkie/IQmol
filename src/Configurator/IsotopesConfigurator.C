/*******************************************************************************

  Copyright (C) 2011-2015 Andrew Gilbert

  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.

  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************/

#include "IsotopesConfigurator.h"
#include "IsotopesLayer.h"
#include "openbabel/data.h"
#include "AtomLayer.h"
#include <QComboBox>
#include <QSpinBox>
#include <QHeaderView>
#include <QtAlgorithms>

#include <QDebug>


namespace IQmol {
namespace Configurator {

Isotopes::Isotopes(Layer::Isotopes& isotopes) : m_isotopes(isotopes) 
{ 
   m_configurator.setupUi(this);

   QTableWidget* table(m_configurator.isotopeTable);
   table->setColumnCount(3);
   table->verticalHeader()->setVisible(false);
   table->horizontalHeaderItem(0)->setText("Element");
   table->horizontalHeaderItem(1)->setText("Isotopic Mass");
   table->horizontalHeaderItem(2)->setText("Indicies");
   table->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
   table->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );
   table->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );
}


void Isotopes::loadTable(QList<Layer::Atom*> const& atoms)
{
   QMap<unsigned, QString> indices;

   AtomList::const_iterator atom;
   for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
       unsigned Z((*atom)->getAtomicNumber());
       unsigned idx((*atom)->getIndex());

       if (indices.contains(Z)) {
          indices[Z] += ", " + QString::number(idx);
       }else {
          indices.insert(Z, QString::number(idx));
       }
   }

   QList<unsigned> atomicNumbers(indices.keys());
   qSort(atomicNumbers);

   QTableWidget* table(m_configurator.isotopeTable);
   QTableWidgetItem* name;
   QTableWidgetItem* index;
   QComboBox* combo;

   OpenBabel::OBElementTable pt;
   unsigned n(atomicNumbers.size());
   table->setRowCount(n);

   for (unsigned i = 0; i < n; ++i) {
       unsigned Z(atomicNumbers[i]);
       combo = new QComboBox(table);
       name  = new QTableWidgetItem(QString::fromStdString(pt.GetName(Z)));
       index = new QTableWidgetItem(indices[Z]);
       table->setItem(      i, 0, name);
       table->setCellWidget(i, 1, combo);
       table->setItem(      i, 2, index);
       loadMasses(Z, combo);
   }
}


void Isotopes::loadMasses(unsigned Z, QComboBox* combo)
{
   OpenBabel::OBElementTable pt;
   OpenBabel::OBIsotopeTable it;


   QList<unsigned> isotopes;
   QString symbol(pt.GetSymbol(Z));

   // The isotopes are ordered by abundance
   switch(Z) {
      case 1:
         isotopes << 1 << 2 << 3;
         break;
      case 6:
         isotopes << 12 << 13;
         break;
      default:
         break;
   }

   combo->clear();
   double m(pt.GetMass(Z));
   combo->addItem(symbol + "(std)", m);

   for (int i = 0; i < isotopes.size(); ++i) {
       QString label(symbol + "(" + QString::number(isotopes[i]) + ")");
       double m(it.GetExactMass(Z,isotopes[i]));
       combo->addItem(label, m);
   }
}


QString Isotopes::toString() const
{
   QString str;
   unsigned count(0);

   QTableWidget* table(m_configurator.isotopeTable);

   for (int row = 0; row < table->rowCount(); ++row) {
       QComboBox* combo(qobject_cast<QComboBox*>(table->cellWidget(row, 1)));
       if (combo) {
          QString s(combo->currentText());
          if (!s.contains("(std)")) { 
             QString mass(combo->currentData().toString());
             QString indices(table->item(row, 2)->text());

             QStringList list(indices.split(", "));
             count += list.size();
             str += list.join("   " + mass + "\n");
             str += "  " + mass + "\n";
          }
       }
   }


   double T(m_configurator.temperatureSpinBox->value());
   double P(m_configurator.pressureSpinBox->value());
   QString temperature(QString::number(T, 'f', 2));
   QString pressure(QString::number(P, 'f', 2));

   str = QString::number(count) + "  " + temperature + "  " + pressure + "\n" + str;
   //qDebug() << "Found mass: " << str;

   return str;
}

} } // end namespace IQmol::Configurator
