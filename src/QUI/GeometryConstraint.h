#ifndef QUI_GEOMETRYCONSTRAINT_H
#define QUI_GEOMETRYCONSTRAINT_H

/*!
 *  \file GeometryConstraints.h 
 *  
 *  \brief contains class definitions associated with geometry constraints as
 *  specified in the $opt section.
 *  
 *  \author Andrew Gilbert
 *  \date   February 2009
 */

#include "ui_GeometryConstraintDialog.h"
#include "boost/tuple/tuple.hpp"
#include <vector>
#include <set>
#include <QList>


class QString;
class QVariant;


namespace Qui {
class OptSection;

namespace GeometryConstraint {


struct Type {
   enum ID { Stretch = 0, Bend, OutOfPlane, Dihedral, Coplanar, Perpendicular,
             Fixed, DummyNormal, DummyBisector, Connect};
};

QString ToSTring(Type::ID const& type);

class Constraint;

typedef std::vector<Constraint*> List;
// TableRow - [ Type, atom1, atom2, atom3, atom4, value ]
typedef boost::tuple<Type::ID, int, int, int, int, QVariant> TableRow;


//! \class Dialog is used to display and edit a list of constraints.
class Dialog: public QDialog {

   Q_OBJECT

   public:
     Dialog(QWidget* parent, OptSection* opt, int nAtoms);
     ~Dialog() { }

   private Q_SLOTS:
      void on_okButton_clicked(bool);
      void on_constraintType_currentIndexChanged(int);
      void on_dummyType_currentIndexChanged(int);
      void on_addConstraint_clicked(bool);
      void on_addFixedAtom_clicked(bool);
      void on_addDummyAtom_clicked(bool);
      void on_addConnectivityButton_clicked(bool);
      void on_deleteButton_clicked(bool);
      Constraint* getConstraint(int row);
      void contextMenu(QPoint const&);
      void deleteConstraint();


   private:
      Ui::Dialog m_ui;
      int m_nAtoms;
      std::set<QString> m_constraintKeys;
      OptSection* m_opt;

      void loadConstraintsToTable(List const&);
      void addConstraintToTable(Constraint const&);
      void addConstraintToTable(TableRow const&);
      void updateAtomSpinBoxRanges();
};




//! An ABC for geometry constraints
class Constraint {

   public:
      Constraint() : m_key("") { }
      virtual ~Constraint()  { }

      virtual bool isValid(int const& nAtoms) const;
      virtual QString format() const = 0;
      virtual Type::ID type() const = 0;
      virtual TableRow tableForm() const = 0;
      virtual Constraint* clone() const = 0;
      
      static Constraint* fromTable(TableRow const& row);
      static Constraint* fromString(QString const& s);
      QString formatAtomList() const;
      virtual QString key() const;

   protected:
      QList<int> m_atomList;
      mutable QString m_key;
};



class Stretch : public Constraint {
   public:
      Stretch(int const& atom1, int const& atom2, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Stretch; }
      TableRow tableForm() const;
      Stretch* clone() const;

   private:
     double m_value;
};



class Bend : public Constraint {
   public:
      Bend(int const& atom1, int const& atom2, int const& atom3, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Bend; }
      TableRow tableForm() const;
      Bend* clone() const;

   private:
     double m_value;
};



class OutOfPlane : public Constraint {
   public:
      OutOfPlane(int const& atom1, int const& atom2, int const& atom3, 
         int const& atom4, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::OutOfPlane; }
      TableRow tableForm() const;
      OutOfPlane* clone() const;

   private:
     double m_value;
};



class Dihedral: public Constraint {
   public:
      Dihedral(int const& atom1, int const& atom2, int const& atom3, 
         int const& atom4, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Dihedral; }
      TableRow tableForm() const;
      Dihedral* clone() const;

   private:
     double m_value;
};



class Coplanar : public Constraint {
   public:
      Coplanar(int const& atom1, int const& atom2, int const& atom3, 
         int const& atom4, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Coplanar; }
      TableRow tableForm() const;
      Coplanar* clone() const;

   private:
     double m_value;
};



class Perpendicular : public Constraint {
   public:
      Perpendicular(int const& atom1, int const& atom2, int const& atom3, 
         int const& atom4, double const& value);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Perpendicular; }
      TableRow tableForm() const;
      Perpendicular* clone() const;

   private:
     double m_value;
};



class Fixed : public Constraint {
   public:
      Fixed(int const& atom1, QString const& xyz);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Fixed; }
      TableRow tableForm() const;
      Fixed* clone() const;

   private:
     QString m_xyz;
};



class DummyNormal : public Constraint {
   public:
      DummyNormal(int const& atom1, int const& atom2, int const& atom3, int const& dummy);

      QString format() const;
      Type::ID type() const { return Type::DummyNormal; }
      TableRow tableForm() const;
      QString key() const;
      DummyNormal* clone() const;

   private:
     int m_atomNumber;
};



class DummyBisector : public Constraint {
   public:
      DummyBisector(int const& atom1, int const& atom2, int const& atom3, int const& dummy);

      QString format() const;
      Type::ID type() const { return Type::DummyBisector; }
      TableRow tableForm() const;
      QString key() const;
      DummyBisector* clone() const;

   private:
     int m_atomNumber;
};


class Connect : public Constraint {
   public:
      Connect(int const& targetAtom, QList<int> const& linkedAtoms);
      Connect(int const& targetAtom, QList<QVariant> const& linkedAtoms);

      bool isValid(int const& nAtoms) const;
      QString format() const;
      Type::ID type() const { return Type::Connect; }
      TableRow tableForm() const;
      QString key() const;
      Connect* clone() const;

   private:
     int m_targetAtom;
};


} } // end namespaces Qui::GeometryConstraint

#endif
