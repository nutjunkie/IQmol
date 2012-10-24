/*******************************************************************************

  Copyright (C) 2011 Andrew Gilbert

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

#include "ConstraintConfigurator.h"
#include "ConstraintLayer.h"
#include "QGLViewer/vec.h"


using namespace qglviewer;

namespace IQmol {
namespace Configurator {



// --------------- VectorConstraintConfigurator ---------------

VectorConstraint::VectorConstraint(Layer::Constraint* constraint) : Constraint(constraint)
{
   m_constraintConfigurator.setupUi(this);
   setWindowTitle("Constrain Position");
   m_unit = QChar(0x00c5);
   m_constraintConfigurator.xValue->setSuffix(QString(" ") + m_unit);
   m_constraintConfigurator.yValue->setSuffix(QString(" ") + m_unit);
   m_constraintConfigurator.zValue->setSuffix(QString(" ") + m_unit);

   // Make these invisible for the time being as we don't allow 
   // individual axis constraints.  
   m_constraintConfigurator.xCheckBox->setVisible(false);
   m_constraintConfigurator.yCheckBox->setVisible(false);
   m_constraintConfigurator.zCheckBox->setVisible(false);

   init();
}


void VectorConstraint::init()
{
   Vec position(m_constraint->m_atoms[0]->getPosition());
   m_constraintConfigurator.xValue->setValue(position.x);
   m_constraintConfigurator.yValue->setValue(position.y);
   m_constraintConfigurator.zValue->setValue(position.z);
}


void VectorConstraint::sync()
{
   QString text("Current position of atom");
   text += QString::number(m_constraint->m_atoms[0]->getIndex()) + ":";
   m_constraintConfigurator.label->setText(text);

   Vec position(m_constraint->m_atoms[0]->getPosition());
   text = "(";
   text += QString::number(position.x, 'f', s_precisionDistance) + ",";
   text += QString::number(position.y, 'f', s_precisionDistance) + ",";
   text += QString::number(position.z, 'f', s_precisionDistance) + ")";
   m_constraintConfigurator.positionLabel->setText(text);
}


void VectorConstraint::accept()
{
   m_constraint->m_mesg = "";
   unsigned char axes(0);

   if (m_constraintConfigurator.xCheckBox->checkState() == Qt::Checked) {
      axes = axes | XAxis;
      m_constraint->m_mesg += "X";
   }
   if (m_constraintConfigurator.zCheckBox->checkState() == Qt::Checked) {
      axes = axes | YAxis;
      m_constraint->m_mesg += "Y";
   }
   if (m_constraintConfigurator.yCheckBox->checkState() == Qt::Checked) {
      axes = axes | ZAxis;
      m_constraint->m_mesg += "Z";
   }
   
   m_constraint->m_mesg += " coordinates of atom " + 
       m_constraint->m_atoms[0]->text() + " fixed";

   Vec position(m_constraintConfigurator.xValue->value(),
                m_constraintConfigurator.yValue->value(),
                m_constraintConfigurator.zValue->value());

   m_constraint->setTargetValue(axes, position);
   QDialog::accept();
}



// --------------- ScalarConstraintConfigurator ---------------

ScalarConstraint::ScalarConstraint(Layer::Constraint* constraint) : Constraint(constraint)
{
   m_constraintConfigurator.setupUi(this);
   init();
}


void ScalarConstraint::init()
{
   switch (m_constraint->constraintType()) {
      case Layer::Constraint::Distance:  initDistance();  break;
      case Layer::Constraint::Angle:     initAngle();     break;
      case Layer::Constraint::Torsion:   initTorsion();   break;
      default:  break;
   }
}


void ScalarConstraint::initDistance()
{  
   setWindowTitle("Constrain Distance");
   m_unit = QChar(0x00c5);
   m_constraintConfigurator.value->setSuffix(QString(" ") + m_unit);
   m_constraintConfigurator.value->setDecimals(s_precisionDistance);
   m_constraintConfigurator.value->setRange(0.000, 1000.0);
   m_constraintConfigurator.value->setValue(
      Layer::Atom::distance(m_constraint->m_atoms[0], m_constraint->m_atoms[1]));
}


void ScalarConstraint::initAngle()
{
   setWindowTitle("Constrain Angle");
   m_unit = QChar(0x00b0);
   m_constraintConfigurator.value->setSuffix(QString(" ") + m_unit);
   m_constraintConfigurator.value->setDecimals(s_precisionAngle);
   m_constraintConfigurator.value->setRange(0.00, 180.0);
   m_constraintConfigurator.value->setValue(
      Layer::Atom::angle(m_constraint->m_atoms[0], 
                         m_constraint->m_atoms[1], 
                         m_constraint->m_atoms[2]));
}


void ScalarConstraint::initTorsion()
{
   setWindowTitle("Constrain Torsion");
   m_unit = QChar(0x00b0);
   m_constraintConfigurator.value->setSuffix(QString(" ") + m_unit);
   m_constraintConfigurator.value->setDecimals(s_precisionTorsion);
   m_constraintConfigurator.value->setRange(-179.99, 180.0);
   m_constraintConfigurator.value->setValue(
      Layer::Atom::torsion(m_constraint->m_atoms[0], 
                           m_constraint->m_atoms[1], 
                           m_constraint->m_atoms[2], 
                           m_constraint->m_atoms[3]));
}


void ScalarConstraint::sync()
{
   switch (m_constraint->constraintType()) {
      case Layer::Constraint::Distance:  syncDistance();  break;
      case Layer::Constraint::Angle:     syncAngle();     break;
      case Layer::Constraint::Torsion:   syncTorsion();   break;
      default:  break;
   }
}


void ScalarConstraint::syncDistance()
{   
   double current(Layer::Atom::distance(m_constraint->m_atoms[0], 
                                        m_constraint->m_atoms[1]));
 
   QString text("Distance between atoms ");
   text += QString::number(m_constraint->m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[1]->getIndex()) + ":  ";

   text += QString::number(current, 'f', s_precisionAngle) + " " + m_unit;
   m_constraintConfigurator.label->setText(text);
}


void ScalarConstraint::syncAngle()
{
   double current(Layer::Atom::angle(m_constraint->m_atoms[0], 
                                     m_constraint->m_atoms[1], 
                                     m_constraint->m_atoms[2]));
 
   QString text("Angle between atoms ");
   text += QString::number(m_constraint->m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[1]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[2]->getIndex()) + ":  ";

   text += QString::number(current, 'f', s_precisionAngle) + " " + m_unit;
   m_constraintConfigurator.label->setText(text);
}


void ScalarConstraint::syncTorsion()
{
   double current(Layer::Atom::torsion(m_constraint->m_atoms[0], 
                                       m_constraint->m_atoms[1], 
                                       m_constraint->m_atoms[2], 
                                       m_constraint->m_atoms[3]));

   QString text("Torsion between atoms ");
   text += QString::number(m_constraint->m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[1]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[2]->getIndex()) + "-" +
           QString::number(m_constraint->m_atoms[3]->getIndex()) + ":  ";
                           
   text += QString::number(current, 'f', 1) + " " + m_unit;
   m_constraintConfigurator.label->setText(text);
}
 

void ScalarConstraint::accept()
{
   double value = m_constraintConfigurator.value->value();
   m_constraint->setTargetValue(value);

   switch (m_constraint->constraintType()) {
      case Layer::Constraint::Distance: 
         m_constraint->m_mesg = "Distance set to " + QString::number(value, 'f', 4);
         break;
      case Layer::Constraint::Angle: 
         m_constraint->m_mesg = "Angle set to " + QString::number(value, 'f', 3);
         break;
      case Layer::Constraint::Torsion: 
         m_constraint->m_mesg = "Torsion set to " + QString::number(value, 'f', 2);
         break;
      default: 
         break;
   }
   QDialog::accept();
}


} } // end namespace IQmol::Configurator
