/*!
 *  \file GeometryConstraint.C
 *  
 *  \author Andrew Gilbert
 *  \date   February 2009
 */

#include <QtDebug>
#include <QMessageBox>
#include "GeometryConstraint.h"
#include "OptSection.h"
#include "Qui.h"

#include <algorithm>
#include <vector>
#include <QHeaderView>
#include <QMenu>


namespace Qui {
namespace GeometryConstraint {


QString ToString(Type::ID const& type) {
   QString text;
   switch (type) {
      case Type::Stretch:       { text = "Stretch";                } break;
      case Type::Bend:          { text = "Bend";                   } break;
      case Type::OutOfPlane:    { text = "Out Of Plane";           } break;
      case Type::Dihedral:      { text = "Dihedral";               } break;
      case Type::Coplanar:      { text = "Coplanar";               } break;
      case Type::Perpendicular: { text = "Perpendicular";          } break;
      case Type::Fixed:         { text = "Fixed";                  } break;
      case Type::DummyNormal:   { text = "Dummy - Plane Normal";   } break;
      case Type::DummyBisector: { text = "Dummy - Angle Bisector"; } break;
      case Type::Connect:       { text = "Connect";                } break;
   }
   return text;
}


// ------
// Dialog
// ------
Dialog::Dialog(QWidget* parent, OptSection* opt, int nAtoms) 
 : QDialog(parent), m_opt(opt) {

   m_ui.setupUi(this);
   m_ui.constraintTable->hideColumn(0);
   loadConstraintsToTable(opt->getConstraints());
   m_nAtoms = nAtoms + opt->numberOfDummyAtoms();

   updateAtomSpinBoxRanges();

   QString tip = "Specifies connectivity between the given atom and up to four "
      "other atoms in the system.";
   m_ui.targetAtom->setToolTip(tip);

   on_constraintType_currentIndexChanged(0);
   on_dummyType_currentIndexChanged(0);

   m_ui.constraintTable->verticalHeader()->
      setDefaultSectionSize(fontMetrics().lineSpacing() + 5);

    
   m_ui.constraintTable->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_ui.constraintTable, SIGNAL(customContextMenuRequested(QPoint const&)),
      this, SLOT(contextMenu(QPoint const&)));
}



void Dialog::contextMenu(QPoint const& pos) {
   QTableWidgetItem* item = m_ui.constraintTable->itemAt(pos);
   if (item) {
      QMenu *menu = new QMenu(this);
      menu->addAction(tr("Delete"), this, SLOT(deleteConstraint()));
      menu->exec(m_ui.constraintTable->mapToGlobal(pos));
   }
}



void Dialog::updateAtomSpinBoxRanges() {
   m_ui.atom1->setRange(1,m_nAtoms);
   m_ui.atom2->setRange(1,m_nAtoms);
   m_ui.atom3->setRange(1,m_nAtoms);
   m_ui.atom4->setRange(1,m_nAtoms);

   m_ui.fixedAtom->setRange(1,m_nAtoms);

   m_ui.dAtom1->setRange(1,m_nAtoms);
   m_ui.dAtom2->setRange(1,m_nAtoms);
   m_ui.dAtom3->setRange(1,m_nAtoms);

   m_ui.targetAtom->setRange(1,m_nAtoms);
   m_ui.cAtom1->setRange(0,m_nAtoms);
   m_ui.cAtom2->setRange(0,m_nAtoms);
   m_ui.cAtom3->setRange(0,m_nAtoms);
   m_ui.cAtom4->setRange(0,m_nAtoms);
}


void Dialog::loadConstraintsToTable(List const& constraints) {
   List::const_iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       addConstraintToTable(**iter);
   }
}


void Dialog::addConstraintToTable(Constraint const& constraint) {

   if (!constraint.isValid(m_nAtoms)) {
      QString msg("The following constraint is invalid:\n");
      msg += constraint.format() + "\n";
      msg += "Most likely this is because your atom numbers are not unique "
             "or out of range. Please correct this before proceeding.\n";
      QMessageBox::warning(this, "Invalid Constraint", msg);
      return;
   }else if (m_constraintKeys.find(constraint.key()) != m_constraintKeys.end()) {
      QString msg("A constraint involving the following atoms has already been defined:\n");
      msg += constraint.key() + "\n";
      QMessageBox::warning(this, "Duplicate Constraint", msg);
      return;
   }

   m_constraintKeys.insert(constraint.key());

   QTableWidget* table(m_ui.constraintTable);
   TableRow row(constraint.tableForm());

   table->insertRow(0);
   Type::ID type(row.get<0>());

   if (type == Type::DummyNormal || type == Type::DummyBisector) {
      m_nAtoms++;
      on_constraintType_currentIndexChanged(m_ui.constraintType->currentIndex());
   }

   int atom;
   QString value;

   table->setItem(0, 0, new QTableWidgetItem(QString::number((int)type)));
   table->setItem(0, 1, new QTableWidgetItem(ToString(constraint.type())));

   atom = row.get<1>();
   value = atom > 0 ? QString::number(atom) : "-";
   table->setItem(0, 2, new QTableWidgetItem(value));

   atom = row.get<2>();
   value = atom > 0 ? QString::number(atom) : "-";
   table->setItem(0, 3, new QTableWidgetItem(value));

   atom = row.get<3>();
   value = atom > 0 ? QString::number(atom) : "-";
   table->setItem(0, 4, new QTableWidgetItem(value));

   atom = row.get<4>();
   value = atom > 0 ? QString::number(atom) : "-";
   table->setItem(0,5,new QTableWidgetItem(value));

   if (type == Type::Connect) {
      QString s;
      QList<QVariant> list = row.get<5>().toList();
      for (int i = 0; i <  list.size(); ++i) {
          s += list[i].toString() + ", ";
      }
      s.chop(2);
      table->setItem(0,6,new QTableWidgetItem(s));
   }else {
      table->setItem(0,6,new QTableWidgetItem(row.get<5>().toString()));
   }
}


void Dialog::addConstraintToTable(TableRow const& row) {
   Constraint* constraint = Constraint::fromTable(row);
   addConstraintToTable(*constraint);
   delete constraint;
}


Constraint* Dialog::getConstraint(int row) {
   QTableWidget* table(m_ui.constraintTable);
   Constraint* constraint(0);

   if (row < table->rowCount()) {
      bool ok;
      int i(table->item(row,0)->text().toInt());
      Type::ID type = static_cast<Type::ID>(i);

      int atom1(table->item(row,2)->text().toInt(&ok));
      if (!ok) atom1 = -1;
      int atom2(table->item(row,3)->text().toInt(&ok));
      if (!ok) atom2 = -1;
      int atom3(table->item(row,4)->text().toInt(&ok));
      if (!ok) atom3 = -1;
      int atom4(table->item(row,5)->text().toInt(&ok));
      if (!ok) atom4 = -1;

      QString value(table->item(row,6)->text());
      value.remove("id=");

      if (type == Type::Connect) {
         QStringList list = value.split(QRegExp(",\\s+"),QString::SkipEmptyParts);
         QList<QVariant> ints;
         for (int i = 0; i < list.size(); ++i) {
             ints.push_back(QVariant(list[i]));
         }
         constraint = Constraint::fromTable(
            boost::make_tuple(type, atom1, atom2, atom3, atom4, QVariant(ints)) );
      }else {
         constraint = Constraint::fromTable(
            boost::make_tuple(type, atom1, atom2, atom3, atom4, QVariant(value)) );
      }

   }

   return constraint;
}


void Dialog::on_dummyType_currentIndexChanged(int i) {
   QString tip;
   if (i == 0) {
      tip = "Dummy atom is positioned a unit distance along the normal to a "
            "plane defined by ABC, centered on B";
   }else {
      tip = "Dummy atom is positioned a unit distance along the bisector of "
            "the angle A-B-C";
   }
   m_ui.dummyType->setToolTip(tip);
}


void Dialog::on_constraintType_currentIndexChanged(int i) {
   if (i == Type::Stretch) {
      m_ui.atom3->setEnabled(false);
      m_ui.atom4->setEnabled(false);
      m_ui.constraintValue->setRange(0.001,99.999);
      m_ui.constraintValue->setSingleStep(0.001);
      m_ui.constraintValue->setDecimals(3);
      m_ui.constraintValue->setSuffix(" A");
      m_ui.constraintValue->setWrapping(false);

   }else if (i == Type::Bend) {
      m_ui.atom3->setEnabled(true);
      m_ui.atom4->setEnabled(false);
      m_ui.constraintValue->setRange(0.01,179.99);
      m_ui.constraintValue->setSingleStep(0.01);
      m_ui.constraintValue->setDecimals(2);
      m_ui.constraintValue->setSuffix(" d");
      m_ui.constraintValue->setWrapping(false);

   }else {
      m_ui.atom3->setEnabled(true);
      m_ui.atom4->setEnabled(true);
      m_ui.constraintValue->setRange(-179.9,180.0);
      m_ui.constraintValue->setSingleStep(0.1);
      m_ui.constraintValue->setDecimals(1);
      m_ui.constraintValue->setSuffix(" d");
      m_ui.constraintValue->setWrapping(true);
   }

   QString tip;

   switch (i) {
      case Type::Stretch: {
         tip = "Constrain bond length A-B";
      } break;

      case Type::Bend: {
         tip = "Constrain bond angle A-B-C";
      } break;

      case Type::OutOfPlane: {
         tip = "Constrain angle between D and the ABC plane";
      } break;

      case Type::Dihedral: {
         tip = "Constrain angle between the ABC plane and the BCD plane";
      } break;

      case Type::Coplanar: {
         tip = "Constrain bending of A-B-C in the plane BCD";
      } break;

      case Type::Perpendicular: {
         tip = "Constrain bending of A-B-C perpendicular to the plane BCD";
      } break;
   }

   m_ui.constraintType->setToolTip(tip);
   m_ui.constraintValue->setToolTip(tip);
}


void Dialog::on_addConstraint_clicked(bool) {
   Type::ID type = static_cast<Type::ID>(m_ui.constraintType->currentIndex());
   int atom1(m_ui.atom1->value());
   int atom2(m_ui.atom2->value());
   int atom3(m_ui.atom3->value());
   int atom4(m_ui.atom4->value());
   QVariant value(m_ui.constraintValue->value());

   addConstraintToTable(boost::make_tuple(type, atom1, atom2, 
      atom3, atom4, value));
}


void Dialog::on_addFixedAtom_clicked(bool) {
   int atom(m_ui.fixedAtom->value());
   QString xyz;
   if (m_ui.fixX->isChecked()) xyz += "X";
   if (m_ui.fixY->isChecked()) xyz += "Y";
   if (m_ui.fixZ->isChecked()) xyz += "Z";

   addConstraintToTable(boost::make_tuple(Type::Fixed, atom, 
      -1, -1, -1, QVariant(xyz)));
}


void Dialog::on_addDummyAtom_clicked(bool) {
   QString name(m_ui.dummyType->currentText());
   int i(m_ui.dummyType->currentIndex());
   
   Type::ID type = i == 0 ? Type::DummyNormal :  Type::DummyBisector;
   
   int atom1(m_ui.dAtom1->value());
   int atom2(m_ui.dAtom2->value());
   int atom3(m_ui.dAtom3->value());

   addConstraintToTable(boost::make_tuple(type, atom1, atom2, 
      atom3, -1, QVariant(m_nAtoms+1)));

   m_nAtoms++;
   updateAtomSpinBoxRanges();
}


void Dialog::on_addConnectivityButton_clicked(bool) {
   int target(m_ui.targetAtom->value());
   int atom1(m_ui.cAtom1->value());
   int atom2(m_ui.cAtom2->value());
   int atom3(m_ui.cAtom3->value());
   int atom4(m_ui.cAtom4->value());

   QList<QVariant> connected;
   if (atom1 > 0) connected.push_back(QVariant(atom1));
   if (atom2 > 0) connected.push_back(QVariant(atom2));
   if (atom3 > 0) connected.push_back(QVariant(atom3));
   if (atom4 > 0) connected.push_back(QVariant(atom4));

   addConstraintToTable(boost::make_tuple(Type::Connect, target, -1, -1, -1, 
      QVariant(connected)));
}



void Dialog::on_deleteButton_clicked(bool) {
   deleteConstraint();
}

void Dialog::deleteConstraint() {
   QTableWidget* table(m_ui.constraintTable);
   QList<QTableWidgetItem*> list(table->selectedItems());
   if (list.count() > 1) {
      QString msg("Are you sure you want to delete the selected constraint?");
      if (QMessageBox::question(this, "Delete Constraint?", msg, 
          QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {

         int row = table->row(list[0]);
         int i(table->item(row,0)->text().toInt());
         Type::ID type = static_cast<Type::ID>(i);


         if (type == Type::DummyNormal || type == Type::DummyBisector) {
            msg = "Constraints that reference this dummy atom (";
            msg += list[5]->text() + ") and any with a higher id will become invalid";
            QMessageBox::warning(this, "Warning", msg);
            on_constraintType_currentIndexChanged(m_ui.constraintType->currentIndex());
            m_nAtoms--;
         }

         Constraint* constraint = getConstraint(row);
         m_constraintKeys.erase(constraint->key());
         delete constraint;

         table->removeRow(row);
      }
   }
}


//! Transfers the constraints from the constraintTable to a new ConstraintList.
//! Note that we only want to replace the current constraints with the new ones
//! if the user clicks Ok.
void Dialog::on_okButton_clicked(bool) {
   List::iterator iter;
   List constraints;

   QTableWidget* table(m_ui.constraintTable);
   int nRows(table->rowCount());

   for (int row = 0; row < nRows; ++row) {
       qDebug() << "adding constraint"; 
       constraints.push_back(getConstraint(row));
   }

   m_opt->setConstraints(constraints);
}



// ----------
// Constraint
// ----------

//! Checks to see that the atom numbers are within the correct range 
//! ( 1 <= n <= max) and that they are unique.
bool Constraint::isValid(int const& max) const {
   bool valid(true);
   QList<int>::const_iterator iter1, iter2;   

   for (iter1 = m_atomList.begin(); iter1 != m_atomList.end(); ++iter1) {
       valid = valid && 0 < *iter1 && *iter1 <= max;
       for (iter2 = iter1+1; iter2 != m_atomList.end(); ++iter2) {
           valid = valid && *iter1 != *iter2;
       }
   }
   return valid;
}


QString Constraint::formatAtomList() const {
   QString s;
   QList<int>::const_iterator iter;
   for (iter = m_atomList.begin(); iter != m_atomList.end(); ++iter) {
       s += QString::number(*iter) + "  ";
   }
   return s;
}


Constraint* Constraint::fromString(QString const& s) {
   Constraint* constraint(0);

   QStringList tokens(s.trimmed().split(QRegExp("\\s+"), QString::SkipEmptyParts));
   QString id(tokens[0].toLower());
   int count(tokens.count());
   QList<int> ints;


   for (int i = 0; i < count; ++i) {
       ints.push_back(tokens[i].toInt());
   }

   if (id == "stre" && count  == 4) {
      constraint = fromTable( boost::make_tuple(Type::Stretch, ints[1], 
         ints[2], -1, -1, QVariant(tokens[3])) );

   }else if (id == "bend" && count == 5) {
      constraint = fromTable( boost::make_tuple(Type::Bend, ints[1], 
         ints[2], ints[3], -1, QVariant(tokens[4])) );

   }else if (id == "outp" && count == 6) {
      constraint = fromTable( boost::make_tuple(Type::OutOfPlane, ints[1], 
         ints[2], ints[3], ints[4], QVariant(tokens[5])) );

   }else if (id == "tors" && count == 6) {
      constraint = fromTable( boost::make_tuple(Type::Dihedral, ints[1], 
         ints[2], ints[3], ints[4], QVariant(tokens[5])) );

   }else if (id == "linc" && count == 6) {
      constraint = fromTable( boost::make_tuple(Type::Coplanar, ints[1], 
         ints[2], ints[3], ints[4], QVariant(tokens[5])) );

   }else if (id == "linp" && count == 6) {
      constraint = fromTable( boost::make_tuple(Type::Perpendicular, ints[1], 
         ints[2], ints[3], ints[4], QVariant(tokens[5])) );

   }else if (count == 2) {
      constraint = fromTable( boost::make_tuple(Type::Fixed, ints[0], 
         -1, -1, -1, QVariant(tokens[1])) );

   }else if (count == 6 && ints[1] == 2) {
      constraint = fromTable( boost::make_tuple(Type::DummyNormal, ints[3], 
         ints[4], ints[5], -1, QVariant(tokens[0])) );

   }else if (count == 6 && ints[1] == 3) {
      constraint = fromTable( boost::make_tuple(Type::DummyBisector, ints[3], 
         ints[4], ints[5], -1, QVariant(tokens[0])) );

   }else if (count >= 3) {
      ints.removeFirst();
      ints.removeFirst();
      QVariantList list;
      for (int i = 0; i < ints.size(); ++i) {
          list.push_back(QVariant(ints[i]));
      }
      constraint = fromTable( boost::make_tuple(Type::Connect, ints[0], 
         -1, -1, -1, QVariant(list)));
   }

   return constraint;
}




//! Constraint::fromTable acts like a factory
Constraint* Constraint::fromTable(TableRow const& row) {
   Type::ID type(row.get<0>());
   Constraint* constraint(0);

   switch (type) {
      case Type::Stretch: {
         double value(row.get<5>().toDouble());
         constraint = new Stretch(row.get<1>(), row.get<2>(), value);
      } break;
      
      case Type::Bend: {
         double value(row.get<5>().toDouble());
         constraint = new Bend(row.get<1>(), row.get<2>(), row.get<3>(), value);
      } break;
      
      case Type::OutOfPlane: {
         double value(row.get<5>().toDouble());
         constraint = new OutOfPlane(row.get<1>(), row.get<2>(), 
            row.get<3>(), row.get<4>(), value);
      } break;
      
      case Type::Dihedral: {
         double value(row.get<5>().toDouble());
         constraint = new Dihedral(row.get<1>(), row.get<2>(), 
            row.get<3>(), row.get<4>(), value);
      } break;
      
      case Type::Coplanar: {
         double value(row.get<5>().toDouble());
         constraint = new Coplanar(row.get<1>(), row.get<2>(), 
            row.get<3>(), row.get<4>(), value);
      } break;
      
      case Type::Perpendicular: {
         double value(row.get<5>().toDouble());
         constraint = new Perpendicular(row.get<1>(), row.get<2>(), 
            row.get<3>(), row.get<4>(), value);
      } break;
      
      case Type::Fixed: {
         QString value(row.get<5>().toString());
         constraint = new Fixed(row.get<1>(), value);
      } break;
      
      case Type::DummyNormal: {
         int value(row.get<5>().toInt());
         constraint = new DummyNormal(row.get<1>(), row.get<2>(), row.get<3>(),value);
      } break;
      
      case Type::DummyBisector: {
         int value(row.get<5>().toInt());
         constraint = new DummyBisector(row.get<1>(), row.get<2>(), row.get<3>(),value);
      } break;
      
      case Type::Connect: {
         QList<QVariant> value(row.get<5>().toList());
         constraint = new Connect(row.get<1>(), value);
      } break;
   } 

   return constraint;
}


// Generates a key to identify constraint atoms to prevent duplications.
QString Constraint::key() const {
   if (m_key.isEmpty()) {
      QList<int> atoms(m_atomList);
      qSort( atoms.begin(), atoms.end() );
      QList<int>::iterator iter;
      for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
          if (iter != atoms.begin()) m_key += ":";
          m_key += QString::number(*iter);
      }
      
   }
   return m_key;
}


// -------
// Stretch
// -------
Stretch::Stretch(int const& atom1, int const& atom2, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_value = value;
}

bool Stretch::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && 0 < m_value;
}

QString Stretch::format() const {
   return  QString("stre  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow Stretch::tableForm() const {
   return boost::make_tuple(Type::Stretch, m_atomList[0], m_atomList[1], 
      -1, -1, QVariant(m_value));
}

Stretch* Stretch::clone() const {
   return new Stretch(m_atomList[0], m_atomList[1], m_value);
}


// ----
// Bend
// ----
Bend::Bend(int const& atom1, int const& atom2, int const& atom3, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_value = value;
}

bool Bend::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && 0 <= m_value && m_value <=  180;
}

QString Bend::format() const {
   return  QString("bend  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow Bend::tableForm() const {
   return boost::make_tuple(Type::Bend, m_atomList[0], m_atomList[1], 
      m_atomList[2], -1, QVariant(m_value));
}

Bend* Bend::clone() const {
   return new Bend(m_atomList[0], m_atomList[1], m_atomList[2], m_value);
}


// ----------
// OutOfPlane
// ----------
OutOfPlane::OutOfPlane(int const& atom1, int const& atom2, int const& atom3, 
   int const& atom4, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomList.push_back(atom4);
   m_value = value;
}

bool OutOfPlane::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && -180 <= m_value && m_value <=  180;
}

QString OutOfPlane::format() const {
   return  QString("outp  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow OutOfPlane::tableForm() const {
   return boost::make_tuple(Type::OutOfPlane, m_atomList[0], m_atomList[1], 
      m_atomList[2], m_atomList[3], QVariant(m_value));
}

OutOfPlane* OutOfPlane::clone() const {
   return new OutOfPlane(m_atomList[0], m_atomList[1], m_atomList[2], 
      m_atomList[3], m_value);
}



// --------
// Dihedral
// --------
Dihedral::Dihedral(int const& atom1, int const& atom2, int const& atom3, 
   int const& atom4, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomList.push_back(atom4);
   m_value = value;
}

bool Dihedral::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && -180 <= m_value && m_value <=  180;
}

QString Dihedral::format() const {
   return  QString("tors  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow Dihedral::tableForm() const {
   return boost::make_tuple(Type::Dihedral, m_atomList[0], m_atomList[1], 
      m_atomList[2], m_atomList[3], QVariant(m_value));
}

Dihedral* Dihedral::clone() const {
   return new Dihedral(m_atomList[0], m_atomList[1], m_atomList[2], 
      m_atomList[3], m_value);
}



// --------
// Coplanar
// --------
Coplanar::Coplanar(int const& atom1, int const& atom2, int const& atom3, 
   int const& atom4, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomList.push_back(atom4);
   m_value = value;
}

bool Coplanar::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && -180 <= m_value && m_value <=  180;
}

QString Coplanar::format() const {
   return  QString("linc  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow Coplanar::tableForm() const {
   return boost::make_tuple(Type::Coplanar, m_atomList[0], m_atomList[1], 
      m_atomList[2], m_atomList[3], QVariant(m_value));
}

Coplanar* Coplanar::clone() const {
   return new Coplanar(m_atomList[0], m_atomList[1], m_atomList[2], 
      m_atomList[3], m_value);
}



// -------------
// Perpendicular
// -------------
Perpendicular::Perpendicular(int const& atom1, int const& atom2, int const& atom3, 
   int const& atom4, double const& value) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomList.push_back(atom4);
   m_value = value;
}

bool Perpendicular::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && -180 <= m_value && m_value <=  180;
}

QString Perpendicular::format() const {
   return  QString("linp  " + formatAtomList() + "  " + QString::number(m_value));
}

TableRow Perpendicular::tableForm() const {
   return boost::make_tuple(Type::Perpendicular, m_atomList[0], m_atomList[1], 
      m_atomList[2], m_atomList[3], QVariant(m_value));
}

Perpendicular* Perpendicular::clone() const {
   return new Perpendicular(m_atomList[0], m_atomList[1], m_atomList[2], 
      m_atomList[3], m_value);
}



// -----
// Fixed
// -----
Fixed::Fixed(int const& atom1, QString const& xyz) {
   m_atomList.push_back(atom1);
   m_xyz = xyz;
}

bool Fixed::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && !m_xyz.isEmpty();
}

QString Fixed::format() const {
   return  formatAtomList() + "  " + m_xyz;
}

TableRow Fixed::tableForm() const {
   return boost::make_tuple(Type::Fixed, m_atomList[0], -1, -1, -1, QVariant(m_xyz));
}

Fixed* Fixed::clone() const {
   return new Fixed(m_atomList[0], m_xyz);
}



// -----------
// DummyNormal
// -----------
DummyNormal::DummyNormal(int const& atom1, int const& atom2, int const& atom3, 
   int const& atomNumber) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomNumber = atomNumber;
}

QString DummyNormal::format() const {
   return  QString::number(m_atomNumber) + "  2  3  " + formatAtomList();
}

TableRow DummyNormal::tableForm() const {
   return boost::make_tuple(Type::DummyNormal, m_atomList[0], m_atomList[1], 
      m_atomList[2], -1, QVariant("id=" + QString::number(m_atomNumber)));
}

QString DummyNormal::key() const {
   return Constraint::key() + " - plane normal";
}

DummyNormal* DummyNormal::clone() const {
   return new DummyNormal(m_atomList[0], m_atomList[1], m_atomList[2], m_atomNumber);
}



// -------------
// DummyBisector
// -------------
DummyBisector::DummyBisector(int const& atom1, int const& atom2, int const& atom3,
   int const& atomNumber) {
   m_atomList.push_back(atom1);
   m_atomList.push_back(atom2);
   m_atomList.push_back(atom3);
   m_atomNumber = atomNumber;
}

QString DummyBisector::format() const {
   return  QString::number(m_atomNumber) + "  3  3  " + formatAtomList();
}

TableRow DummyBisector::tableForm() const {
   return boost::make_tuple(Type::DummyBisector, m_atomList[0], m_atomList[1], 
      m_atomList[2], -1, QVariant("id=" + QString::number(m_atomNumber)));
}


QString DummyBisector::key() const {
   return Constraint::key() + " - angle bisector";
}

DummyBisector* DummyBisector::clone() const {
   return new DummyBisector(m_atomList[0], m_atomList[1], m_atomList[2], m_atomNumber);
}


// -------
// Connect
// -------
Connect::Connect(int const& targetAtom, QList<int> const& linkedAtoms)
  : m_targetAtom(targetAtom) {
   m_atomList = linkedAtoms;
}

Connect::Connect(int const& targetAtom, QVariantList const& linkedAtoms)
  : m_targetAtom(targetAtom) { 
  for (int i = 0; i < linkedAtoms.size(); ++i) {
      m_atomList.push_back(linkedAtoms[i].toInt());
  }
}

bool Connect::isValid(int const& nAtoms) const {
   return Constraint::isValid(nAtoms) && m_atomList.size() != 0;
}

QString Connect::format() const {
   return  QString::number(m_targetAtom) + "  " + m_atomList.size() + 
       "  " + formatAtomList();
}

TableRow Connect::tableForm() const {
   QList<QVariant> list;
   for (int i = 0; i < m_atomList.size(); ++i) {
       list.push_back(QVariant(m_atomList[i]));
   }
   return boost::make_tuple(Type::Connect, m_targetAtom, -1, -1, -1,
      QVariant(list));
}

QString Connect::key() const {
   return Constraint::key() + " - connect";
}

Connect* Connect::clone() const {
   return new Connect(m_targetAtom, m_atomList);
}


} } // end namespaces Qui::GeometryConstraint
