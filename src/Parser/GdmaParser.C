/*******************************************************************************
       
  Copyright (C) 2011-2015ndrew Gilbert
           
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

#include "GdmaParser.h"
#include "Geometry.h"
#include "GeometryList.h"
#include "MultipoleExpansion.h"
#include "TextStream.h"
#include "Atom.h"
#include "Numerical.h"
#include "DipoleMoment.h"


using namespace qglviewer;

namespace IQmol {
namespace Parser {

bool Gdma::parse(TextStream& textStream)
{

   Data::MultipoleExpansionList* dma(new Data::MultipoleExpansionList);
   Data::GeometryList* geometryList(new Data::GeometryList);
   Data::Geometry* geometry(new Data::Geometry);

   geometryList->append(geometry);

   bool ok(true);
   QString line;
   QString label;
   QStringList tokens;

   // Skip to the good stuff
   textStream.seek("Multipole moments in atomic units");

   while (!textStream.atEnd()) {
      line = textStream.nextLine();
      if (line.contains("Total multipoles referred to origin")) {

         tokens = textStream.seekAndSplit("Q00");
         if (tokens.size() < 3) goto error;
         double q(tokens[2].toDouble(&ok));  if (!ok) goto error;
         geometry->setCharge(Util::round(q));

         tokens = textStream.nextLineAsTokens();
         if (tokens.size() < 12) goto error;
         // order changes due to spherical -> cartesian
         double z(tokens[ 5].toDouble(&ok));  if (!ok) goto error;
         double x(tokens[ 8].toDouble(&ok));  if (!ok) goto error;
         double y(tokens[11].toDouble(&ok));  if (!ok) goto error;
         geometry->appendProperty(new Data::DipoleMoment(Vec(x,y,z)));

      } else if (line.contains("angstrom")) {

         tokens = TextStream::tokenize(line);
         // We are parsing a line such as:
         //  H          x = -0.763556  y = -0.000000  z = -0.467927 angstrom
         if (tokens.size() < 11) goto error;

         // Make sure our coordinates are valid
         double x(tokens[3].toDouble(&ok));  if (!ok) goto error;
         double y(tokens[6].toDouble(&ok));  if (!ok) goto error;
         double z(tokens[9].toDouble(&ok));  if (!ok) goto error;

         // If the site is an atom center, add that to the geometry
         int atomicNumber(Data::Atom::atomicNumber(tokens[0]));
         if (atomicNumber > 0) {
            geometry->append(atomicNumber, Vec(x, y, z));
         }

         // Add the site to the expansion
         // Note this assumes a modified version of GDMA that prints
         // even the very small moments.
         Data::MultipoleExpansion* site(new Data::MultipoleExpansion(Vec(x, y, z)));
         dma->append(site);

         tokens = textStream.seekAndSplit("Q00");
         if (tokens.size() < 3) goto error;
         double q(tokens[2].toDouble(&ok));  if (!ok) goto error;
         site->addCharge(q);

         tokens = textStream.seekAndSplit("|Q1|");
         if (tokens.size() < 12) goto error;
         z = tokens[ 5].toDouble(&ok);  if (!ok) goto error;
         x = tokens[ 8].toDouble(&ok);  if (!ok) goto error;
         y = tokens[11].toDouble(&ok);  if (!ok) goto error;
         site->addDipole(x,y,z);
      }
   }

   if (geometry->nAtoms() > 0) {
      geometry->computeGasteigerCharges();
      geometry->appendProperty(dma);
      m_dataBank.append(geometryList);
   }

   return true;;

   error:
      delete geometry;
      delete dma;
      geometry = 0;
      dma = 0;
      QString msg("Problem parsing DMA, line number ");
      m_errors.append(msg += QString::number(textStream.lineNumber()));
      return false;
}

} } // end namespace IQmol::Parser
