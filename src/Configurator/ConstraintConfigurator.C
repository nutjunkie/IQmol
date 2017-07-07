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

#include "ConstraintConfigurator.h"
#include "ConstraintLayer.h"
#include "QGLViewer/vec.h"
#include "Axes.h"


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
   m_constraintConfigurator.deleteButton->setVisible(false);

   init();
}


void VectorConstraint::init()
{
   QString atoms(QString::number(m_constraint.m_atoms[0]->getIndex()));
   m_constraintConfigurator.label->setText("Set position of atom " + atoms);
   m_constraint.setText("Position " + atoms);
}


void VectorConstraint::sync()
{
   Vec position(m_constraint.m_atoms[0]->getPosition());
   m_constraintConfigurator.xValue->setValue(position.x);
   m_constraintConfigurator.yValue->setValue(position.y);
   m_constraintConfigurator.zValue->setValue(position.z);
}


void VectorConstraint::on_deleteButton_clicked(bool)
{
   reject(); 
   m_constraint.invalid();
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

   bool constrain(m_constraintConfigurator.constrainButton->isChecked());
   m_constraint.m_optimizeConstraint = constrain;
   m_constraintConfigurator.deleteButton->setVisible(true);

   QDialog::accept();
}



// --------------- ScalarConstraintConfigurator ---------------

ScalarConstraint::ScalarConstraint(Layer::Constraint& constraint) : Constraint(constraint)
{
   m_constraintConfigurator.setupUi(this);
   on_scanButton_clicked(false);
   m_constraintConfigurator.deleteButton->setVisible(false);
  
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
   setWindowTitle("Configure Distance");
   QChar unit(QChar(0x00c5));

   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(3);
   m_constraintConfigurator.value->setRange(0.000, 1000.0);

   m_constraintConfigurator.valueMax->setDecimals(3);
   m_constraintConfigurator.valueMax->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.valueMax->setRange(0.000, 1000.0);

   QString atoms(QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[1]->getIndex()));

   m_constraintConfigurator.label->setText("Distance between atoms " + atoms);
   m_constraint.setText("Distance " + atoms);
}


void ScalarConstraint::initAngle()
{
   setWindowTitle("Configure Angle");
   QChar unit(QChar(0x00b0));

   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(2);
   m_constraintConfigurator.value->setRange(0.00, 180.0);

   m_constraintConfigurator.valueMax->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.valueMax->setDecimals(2);
   m_constraintConfigurator.valueMax->setRange(0.00, 180.0);

   QString atoms(QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[1]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[2]->getIndex()));

   m_constraintConfigurator.label->setText("Angle between atoms " + atoms);
   m_constraint.setText("Angle " + atoms);
}


void ScalarConstraint::initTorsion()
{
   setWindowTitle("Configure Torsion");
   QChar unit(QChar(0x00b0));

   m_constraintConfigurator.value->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.value->setDecimals(1);
   m_constraintConfigurator.value->setRange(-179.9, 180.0);

   m_constraintConfigurator.valueMax->setSuffix(QString(" ") + unit);
   m_constraintConfigurator.valueMax->setDecimals(1);
   m_constraintConfigurator.valueMax->setRange(-179.9, 180.0);

   QString atoms(QString::number(m_constraint.m_atoms[0]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[1]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[2]->getIndex()) + "-" +
                 QString::number(m_constraint.m_atoms[3]->getIndex()));

   m_constraintConfigurator.label->setText("Torsion between atoms " + atoms);
   m_constraint.setText("Torsion " + atoms);
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
   m_constraintConfigurator.valueMax->setValue(current);
}


void ScalarConstraint::syncAngle()
{
   double current(Layer::Atom::angle(m_constraint.m_atoms[0], 
                                     m_constraint.m_atoms[1], 
                                     m_constraint.m_atoms[2]));
   m_constraintConfigurator.value->setValue(current);
   m_constraintConfigurator.valueMax->setValue(current);

}


void ScalarConstraint::syncTorsion()
{
   double current(Layer::Atom::torsion(m_constraint.m_atoms[0], 
                                       m_constraint.m_atoms[1], 
                                       m_constraint.m_atoms[2], 
                                       m_constraint.m_atoms[3]));
   m_constraintConfigurator.value->setValue(current);
   m_constraintConfigurator.valueMax->setValue(current);
}


void ScalarConstraint::updateRange()
{
   bool tf(m_constraintConfigurator.scanButton->isChecked());
   m_constraintConfigurator.valueMax->setVisible(tf);
   m_constraintConfigurator.points->setVisible(tf);
   m_constraintConfigurator.labelTo->setVisible(tf);
   m_constraintConfigurator.labelPoints->setVisible(tf);
}


void ScalarConstraint::on_deleteButton_clicked(bool)
{
   reject(); 
   m_constraint.invalid();
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

   bool optimize(m_constraintConfigurator.constrainButton->isChecked());
   m_constraint.m_optimizeConstraint = optimize;

   bool scan(m_constraintConfigurator.scanButton->isChecked());
   m_constraint.m_scanConstraint = scan;
   if (scan) {
      m_constraint.m_maxValue = m_constraintConfigurator.valueMax->value();
      m_constraint.m_points   = m_constraintConfigurator.points->value();
   }

   QDialog::accept();
   m_constraintConfigurator.setButton->setEnabled(false);
   m_constraintConfigurator.constrainButton->setEnabled(optimize);
   m_constraintConfigurator.scanButton->setEnabled(scan);

   m_constraintConfigurator.deleteButton->setVisible(true);
}


} } // end namespace IQmol::Configurator
