#ifndef IQMOL_CONFIGURATOR_MOLECULARSURFACES_H
#define IQMOL_CONFIGURATOR_MOLECULARSURFACES_H
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

#include "Configurator.h"
#include "ui_MolecularSurfacesConfigurator.h"


namespace IQmol {

namespace Data {
   class SurfaceInfo;
}

namespace Configurator {

   class MolecularSurfaces : public Base {

      Q_OBJECT

      public:
         MolecularSurfaces();

      Q_SIGNALS:
         void calculateSurface(Data::SurfaceInfo const&);

      private Q_SLOTS:
         void on_calculateButton_clicked(bool);
         void on_cancelButton_clicked(bool);
         void on_positiveColorButton_clicked(bool);
         void on_negativeColorButton_clicked(bool);
         void on_surfaceType_currentIndexChanged(int index);

      private:
         void enableNegativeColor(bool);
         void setPositiveColor(QColor const& color);
         void setNegativeColor(QColor const& color);

         Ui::MolecularSurfacesConfigurator m_configurator;
   };

} } // End namespace IQmol::Configurator

#endif
