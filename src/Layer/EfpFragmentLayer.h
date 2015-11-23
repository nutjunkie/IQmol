#ifndef IQMOL_LAYER_EFPFRAGMENT_H
#define IQMOL_LAYER_EFPFRAGMENT_H
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

#include "GroupLayer.h"
#include <QMap>
#include <QDir>
#include <QSet>


namespace IQmol {

namespace Data {
   class EfpFragment;
}

namespace Layer {

   /// Currently this class assumes the fragment name and the file names are the same.
   class EfpFragment : public Group {

      Q_OBJECT

      public:
         enum Mode { Name, Frame, NameAndFrame }; 

         EfpFragment(Data::EfpFragment const&);

         QString format(Mode const mode = NameAndFrame);

         static QString efpParamsSection(QSet<QString> const& fragmentNames);
         static QString getFilePath(QString const& fragmentName);

      private:
         static QString scanDirectory(QDir const& dir, QString const& fragmentName);
         static QMap<QString, QString> s_parameterFiles;
   };


   class EfpFragments : public Base {

      Q_OBJECT
      public:
         EfpFragments() : Base("EFP Fragments") { }
         QList<EfpFragment*> getEfpFragments() { return findLayers<EfpFragment>(Children); }
   };

} } // end namespace IQmol::Layer

#endif
