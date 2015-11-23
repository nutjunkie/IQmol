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

#include "AtomLayer.h"
#include "BondLayer.h"
#include "Geometry.h"
#include "openbabel/obiter.h"
#include "openbabel/mol.h"

#define _USE_MATH_DEFINES
#include <cmath>


namespace IQmol {
namespace Layer {

// Static Data
GLint   Primitive::s_resolution            = 32;
GLfloat Primitive::s_selectColor[]         = { 0.5f, 0.0f, 0.0f, 0.6f };
GLfloat Primitive::s_selectOffset          = 0.08;
GLfloat Primitive::s_selectOffsetWireFrame = 6.08;  // pixels



PrimitiveList::PrimitiveList(IQmol::Data::Geometry const& geometry, bool includeBonds)
{
   AtomList atomList;

   for (unsigned i = 0; i < geometry.nAtoms(); ++i) {
       Layer::Atom* atom = new Layer::Atom(geometry.atomicNumber(i));
       atom->setPosition(geometry.position(i));
       atomList.append(atom);
       append(atom);
   }   

   if (includeBonds) {
      QMap<OpenBabel::OBAtom*, Layer::Atom*> atomMap;
      OpenBabel::OBMol obMol;
      OpenBabel::OBAtom* obAtom;
      qglviewer::Vec position;
      obMol.BeginModify();

      for (AtomList::iterator iter = atomList.begin(); iter != atomList.end(); ++iter) {
          position = (*iter)->getPosition();
          obAtom = obMol.NewAtom();
          obAtom->SetAtomicNum((*iter)->getAtomicNumber());
          obAtom->SetVector(position.x, position.y, position.z);
          atomMap.insert(obAtom, *iter);
      }   

      obMol.EndModify();
      obMol.ConnectTheDots();
      obMol.PerceiveBondOrders();

      for (OpenBabel::OBMolBondIter obBond(&obMol); obBond; ++obBond) {
         Layer::Atom* begin = atomMap.value(obBond->GetBeginAtom());
         Layer::Atom* end   = atomMap.value(obBond->GetEndAtom());
         Layer::Bond* bond  = new Layer::Bond(begin, end);
         bond->setOrder(obBond->GetBondOrder());
         append(bond);
      }   
   }   
}


double Primitive::distance(Primitive* A, Primitive* B) 
{
   return (A->getPosition() - B->getPosition()).norm();
}


double Primitive::angle(Primitive* A, Primitive* B, Primitive* C) 
{
  if ( (A == B) || (B == C)) return 0.0;
  qglviewer::Vec u(B->getPosition() - A->getPosition());
  qglviewer::Vec v(B->getPosition() - C->getPosition());
  double theta(u * v / (u.norm() * v.norm())); 
  return std::acos(theta) * 180.0 / M_PI; 
}


double Primitive::torsion(Primitive* A, Primitive* B, Primitive* C , Primitive* D) 
{
   qglviewer::Vec a(B->getPosition() - A->getPosition());
   qglviewer::Vec b(C->getPosition() - B->getPosition());
   qglviewer::Vec c(D->getPosition() - C->getPosition());
   double x((b.norm() * a) * cross(b,c));
   double y(cross(a,b) * cross(b,c));
   return std::atan2(x,y) * 180.0 / M_PI;
}


} } // end namespace IQmol::Layer
