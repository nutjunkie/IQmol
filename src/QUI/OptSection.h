#ifndef QUI_OPTSECTION_H
#define QUI_OPTSECTION_H

/*!
 *  \class OptSection
 *
 *  \brief A KeywordSection class representing a $opt block
 *   
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "KeywordSection.h"
#include "GeometryConstraint.h"


namespace Qui {


class OptSection : public KeywordSection {
   public:
      OptSection() : KeywordSection("opt") { }
      ~OptSection() { deleteConstraints(); }

      void read(QString const& input);
      OptSection* clone() const;
      void setConstraints(GeometryConstraint::List&);
      GeometryConstraint::List getConstraints();
      int numberOfDummyAtoms() const { return m_dummyAtoms.size(); }

   protected:
      QString dump();

   private:
      GeometryConstraint::List m_constraints;
      GeometryConstraint::List m_dummyAtoms;
      GeometryConstraint::List m_fixedAtoms;
      GeometryConstraint::List m_connects;
      void deleteConstraints();
      void addConstraint(GeometryConstraint::Constraint*);
};


} // end namespace Qui
#endif
