#ifndef IQMOL_EFPFRAGMENTLAYER_H
#define IQMOL_EFPFRAGMENTLAYER_H
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

#include "GroupLayer.h"
#include "DataLayer.h"
#include <QMap>
#include <QDir>
#include <QSet>


namespace IQmol {
namespace Layer {

   /// Currently this class assumes the fragment name and the file names are the same.
   class EFPFragment : public Group {

      Q_OBJECT

      public:
         enum Mode { Name, Frame, NameAndFrame }; 
         EFPFragment(QString const& filePath);
         QString format(Mode const mode = NameAndFrame);

         static QString efpParamsSection(QSet<QString> const& fragmentNames);
         static QString getFilePath(QString const& fragmentName);

      private:
         static QString scanDirectory(QDir const& dir, QString const& fragmentName);
         static QMap<QString, QString> s_parameterFiles;
   };


   class EFPFragments : public Data {

      Q_OBJECT
      public:
         EFPFragments() : Data("EFP Fragments") { }
         QList<EFPFragment*> getEfpFragments() { return findLayers<EFPFragment>(Children); }
   };


} } // end namespace IQmol::Layer

#endif
