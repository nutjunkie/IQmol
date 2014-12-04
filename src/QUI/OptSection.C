/*!
 *  \file OptSection.C 
 *
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "OptSection.h"
#include "GeometryConstraint.h"

#include <QtDebug>


namespace Qui {

using namespace GeometryConstraint;


List OptSection::getConstraints() {
   List all(m_constraints);
   all.insert(all.end(), m_dummyAtoms.begin(), m_dummyAtoms.end());
   all.insert(all.end(), m_fixedAtoms.begin(), m_fixedAtoms.end());
   all.insert(all.end(), m_connects.begin(), m_connects.end());
   return all;
}



void OptSection::addConstraint(Constraint* constraint) {
   Type::ID type(constraint->type());

   if (type == Type::DummyNormal || type == Type::DummyBisector) {
      m_dummyAtoms.push_back(constraint);
   }else if (type == Type::Fixed) {
      m_fixedAtoms.push_back(constraint);
   }else if (type == Type::Connect) {
      m_connects.push_back(constraint);
   }else {
      m_constraints.push_back(constraint);
   }
}



void OptSection::setConstraints(List& constraints) {
   deleteConstraints();
   List::iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       addConstraint(*iter);
   }
}



void OptSection::deleteConstraints() {
   List::iterator iter;
   for (iter = m_constraints.begin(); iter != m_constraints.end(); ++iter) {
       delete (*iter);
   }
   for (iter = m_dummyAtoms.begin(); iter != m_dummyAtoms.end(); ++iter) {
       delete (*iter);
   }
   for (iter = m_fixedAtoms.begin(); iter != m_fixedAtoms.end(); ++iter) {
       delete (*iter);
   }
   for (iter = m_connects.begin(); iter != m_connects.end(); ++iter) {
       delete (*iter);
   }
   m_constraints.clear();
   m_fixedAtoms.clear();
   m_dummyAtoms.clear();
   m_connects.clear();
}



void OptSection::read(QString const& input) {
   deleteConstraints();
   Constraint* constraint;
   QStringList lines( input.trimmed().split(
      QRegExp("\\n"), QString::SkipEmptyParts) );

   for (int i = 0; i < lines.count(); ++i) {
       constraint = Constraint::fromString(lines[i]);
       if (constraint) addConstraint(constraint);
   }
}



QString OptSection::dump() {
   QString s;

   int n(m_constraints.size());
   n += m_dummyAtoms.size();
   n += m_fixedAtoms.size();
   n += m_connects.size();

   if (n != 0) {
      List constraints, dummyAtoms, fixedAtoms;
      List::iterator iter;

      s += "$opt\n";

      if (m_constraints.size() != 0) {
         s+= "CONSTRAINT\n";
         for (iter = m_constraints.begin(); iter != m_constraints.end(); ++iter) {
             s += (*iter)->format() + "\n";
         }
         s+= "ENDCONSTRAINT\n";
      }

      if (m_dummyAtoms.size() != 0) {
         s+= "DUMMY\n";
         for (iter = m_dummyAtoms.begin(); iter != m_dummyAtoms.end(); ++iter) {
             s += (*iter)->format() + "\n";
         }
         s+= "ENDDUMMY\n";
      }

      if (m_fixedAtoms.size() != 0) {
         s+= "FIXED\n";
         for (iter = m_fixedAtoms.begin(); iter != m_fixedAtoms.end(); ++iter) {
             s += (*iter)->format() + "\n";
         }
         s+= "ENDFIXED\n";
      }

      if (m_connects.size() != 0) {
         s+= "CONNECT\n";
         for (iter = m_connects.begin(); iter != m_connects.end(); ++iter) {
             s += (*iter)->format() + "\n";
         }
         s+= "ENDCONNECT\n";
      }
      s += "$end\n";
  }

  return s;
}


OptSection* OptSection::clone() const {
   OptSection* os = new OptSection();
   List::const_iterator iter;
   for (iter = m_constraints.begin(); iter != m_constraints.end(); ++iter) {
       os->addConstraint((*iter)->clone());
   }
   return os;
}


} // end namespace Qui
