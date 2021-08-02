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

#include "InfoConfigurator.h"
#include "InfoLayer.h"
#include "StringFormat.h"
#include <cmath>

#include <QDebug>

namespace IQmol {
namespace Configurator { 

Info::Info(Layer::Info& info) : m_info(info)
{
   m_infoConfigurator.setupUi(this);
   connect(&info, SIGNAL(updated()), this, SLOT(sync()));
}


void Info::sync() 
{
   QString s(" Info");
   QStandardItem* parent(m_info.QStandardItem::parent());
   if (parent) s = parent->text() + s;
   setWindowTitle(s);

   m_infoConfigurator.chargeSpin->setValue(m_info.m_charge);
   m_infoConfigurator.chargeSpin->setMaximum(m_info.m_nuclearCharge);

   int n(m_info.numberOfElectrons());
   m_infoConfigurator.multiplicitySpin->setValue(m_info.m_multiplicity);
   m_infoConfigurator.multiplicitySpin->setMaximum(std::min(10,n+1));

   m_infoConfigurator.atomsLabel->setText( QString::number(m_info.m_numberOfAtoms) );
   m_infoConfigurator.electronsLabel->setText( QString::number(n) );
   m_infoConfigurator.formulaLabel->setText( m_info.formula() );

   s = QString::number(m_info.m_mass, 'f', 3) + " g/mol";
   m_infoConfigurator.massLabel->setText(s);

   switch (m_info.m_energyUnit) {
      case Layer::Info::Hartree:
         s = QString::number(m_info.m_energy, 'f', 6) + " E" + Util::subscript("h");
      break;
      case Layer::Info::KJMol:
         s = QString::number(m_info.m_energy, 'f', 3) + " kJ/mol";
      break;
      case Layer::Info::KCalMol:
         s = QString::number(m_info.m_energy, 'f', 3) + " kcal/mol";
      break;
   }

   m_infoConfigurator.energyLabel->setText(s);
   m_infoConfigurator.energyText->setText(m_info.m_energyText);

   s = QString::number(m_info.m_dipoleValue, 'f', 3) + " D";
   if (m_info.m_dipoleEstimated) s+= " (est.)";
   m_infoConfigurator.dipoleLabel->setText(s);

   m_infoConfigurator.symmetryLabel->setText(m_info.m_pointGroup.toString());
}


void Info::on_chargeSpin_valueChanged(int value) 
{
   m_info.setCharge(value); 
}


void Info::on_multiplicitySpin_valueChanged(int value) 
{
   m_info.setMultiplicity(value); 
}


void Info::on_detectSymmetryButton_clicked(bool) 
{
   m_info.detectSymmetry();
}


} } // end namespace IQmol::Configurator
