/*******************************************************************************

  Copyright (C) 2011-2013 Andrew Gilbert

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

VectorConstraint::VectorConstraint(Layer::Constraint& constraint) : Constraint(constraint)
{
   m_constraintConfigurator.setupUi(this);
   setWindowTitle("Set Position");
   QChar unit(QChar(0x00c5));
   m_constraintConfigurator.xValue->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.yValue->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.zValue->setSuffix(QString(" ") + unit);

   init();
}


void VectorConstraint::init()
{
   QString text("Set position of atom ");
   text += QString::number(m_constraint.m_atoms[0]->getIndex()) + ":";
   m_constraintConfigurator.label->setText(text);
}


void VectorConstraint::sync()
{
   Vec position(m_constraint.m_atoms[0]->getPosition());
   m_constraintConfigurator.xValue->setValue(position.x);
   m_constraintConfigurator.yValue->setValue(position.y);
   m_constraintConfigurator.zValue->setValue(position.z);
}


void VectorConstraint::accept()
{
   m_constraint.m_mesg = "XYZ";
   unsigned char axes(0);
   axes = axes | XAxis;
   axes = axes | YAxis;
   axes = axes | ZAxis;
   
   m_constraint.m_mesg += " coordinates of atom " + 
       m_constraint.m_atoms[0]->text() + " set";

   Vec position(m_constraintConfigurator.xValue->value(),
                m_constraintConfigurator.yValue->value(),
                m_constraintConfigurator.zValue->value());

   m_constraint.setTargetValue(axes, position);

   bool constrain(m_constraintConfigurator.constrainCheckBox->checkState() == Qt::Checked);
   m_constraint.m_optimizeConstraint = constrain;

   QDialog::accept();
}



// --------------- ScalarConstraintConfigurator ---------------

ScalarConstraint::ScalarConstraint(Layer::Constraint& constraint) : Constraint(constraint)
{
   m_constraintConfigurator.setupUi(this);
   init();
}


void ScalarConstraint::init()
{
   switch (m_constraint.constraintType()) {
      case Layer::Constraint::Distance:  initDistance();  break;
      case Layer::Constraint::Angle:     initAngle();     break;
      case Layer::Constraint::Torsion:   initTorsion();   break;
      default:  break;
   }
}


void ScalarConstraint::initDistance()
{  
   setWindowTitle("Set Distance");
   QChar unit(QChar(0x00c5));
   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(3);
   m_constraintConfigurator.value->setRange(0.000, 1000.0);

   QString text("Set distance between atoms ");
   text += QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[1]->getIndex());
   m_constraintConfigurator.label->setText(text);
}


void ScalarConstraint::initAngle()
{
   setWindowTitle("Set Angle");
   QChar unit(QChar(0x00b0));
   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(2);
   m_constraintConfigurator.value->setRange(0.00, 180.0);

   QString text("Set angle between atoms ");
   text += QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[1]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[2]->getIndex());
   m_constraintConfigurator.label->setText(text);
}


void ScalarConstraint::initTorsion()
{
   setWindowTitle("Set Torsion");
   QChar unit(QChar(0x00b0));
   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(1);
   m_constraintConfigurator.value->setRange(-179.9, 180.0);

   QString text("Set torsion between atoms ");
   text += QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[1]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[2]->getIndex()) + "-" +
           QString::number(m_constraint.m_atoms[3]->getIndex());
   m_constraintConfigurator.label->setText(text);
}


void ScalarConstraint::sync()
{
   switch (m_constraint.constraintType()) {
      case Layer::Constraint::Distance:  syncDistance();  break;
      case Layer::Constraint::Angle:     syncAngle();     break;
      case Layer::Constraint::Torsion:   syncTorsion();   break;
      default:  break;
   }
}


void ScalarConstraint::syncDistance()
{   
   double current(Layer::Atom::distance(m_constraint.m_atoms[0], 
                                        m_constraint.m_atoms[1]));
   m_constraintConfigurator.value->setValue(current);
}


void ScalarConstraint::syncAngle()
{
   double current(Layer::Atom::angle(m_constraint.m_atoms[0], 
                                     m_constraint.m_atoms[1], 
                                     m_constraint.m_atoms[2]));
   m_constraintConfigurator.value->setValue(current);

}


void ScalarConstraint::syncTorsion()
{
   double current(Layer::Atom::torsion(m_constraint.m_atoms[0], 
                                       m_constraint.m_atoms[1], 
                                       m_constraint.m_atoms[2], 
                                       m_constraint.m_atoms[3]));
   m_constraintConfigurator.value->setValue(current);
}
 

void ScalarConstraint::accept()
{
   double value = m_constraintConfigurator.value->value();
   m_constraint.setTargetValue(value);

   switch (m_constraint.constraintType()) {
      case Layer::Constraint::Distance: 
         m_constraint.m_mesg = "Distance set to " + QString::number(value, 'f', 4);
         break;
      case Layer::Constraint::Angle: 
         m_constraint.m_mesg = "Angle set to " + QString::number(value, 'f', 3);
         break;
      case Layer::Constraint::Torsion: 
         m_constraint.m_mesg = "Torsion set to " + QString::number(value, 'f', 2);
         break;
      default: 
         break;
   }

   bool constrain(m_constraintConfigurator.constrainCheckBox->checkState() == Qt::Checked);
   m_constraint.m_optimizeConstraint = constrain;

   QDialog::accept();
}


} } // end namespace IQmol::Configurator
