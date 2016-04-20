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

#include "ClippingPlaneConfigurator.h"
#include "ClippingPlaneLayer.h"
#include "OpenGL.h"
#include <cmath>
#include <QtDebug>

using namespace qglviewer;


namespace IQmol {
namespace Configurator {

ClippingPlane::ClippingPlane(Layer::ClippingPlane& clippingPlane) 
 : m_clippingPlane(clippingPlane)
{
   m_configurator.setupUi(this);
   //sync();
}


void ClippingPlane::sync()
{
   Vec origin(m_clippingPlane.getPosition());
   m_configurator.xValue->setValue(origin.x); 
   m_configurator.yValue->setValue(origin.y); 
   m_configurator.zValue->setValue(origin.z); 

   Quaternion orientation(m_clippingPlane.getOrientation());
   Vec axis(0.0, 0.0, 1.0);
   axis = orientation.rotate(axis).unit();

   double theta(std::acos(axis.z));
   if (theta < 0) theta += M_PI;
   double phi(std::atan2(axis.y, axis.x));
   if (phi < 0) phi += 2.0*M_PI;

   m_configurator.thetaValue->setValue(theta); 
   m_configurator.phiValue->setValue(phi); 

   qDebug() << "Getting Position" <<  origin.x << origin.y << origin.z;
   qDebug() << "Getting Rotation" <<  orientation[0] << orientation[1] 
                                  <<  orientation[2] << orientation[3];
   qDebug() << "Calc Angles     " <<  theta << phi;
}


void ClippingPlane::on_okButton_clicked(bool)
{
   double x(m_configurator.xValue->value());
   double y(m_configurator.yValue->value());
   double z(m_configurator.zValue->value());
   m_clippingPlane.setPosition(Vec(x,y,z));

   qDebug() << "Setting Position" <<  x << y << z;

   double theta(m_configurator.thetaValue->value());
   double phi(m_configurator.phiValue->value());

   x = std::sin(theta)*std::cos(phi);
   y = std::sin(theta)*std::sin(phi);
   z = std::cos(theta);

   m_clippingPlane.setOrientation(Quaternion(Vec(0.0, 0.0, 1.0), Vec(x,y,z)));

   Quaternion q(Vec(x,y,z), Vec(0.0, 0.0, 1.0));
   qDebug() << "Setting Rotation" << q[0] << q[1] << q[2] << q[3];
   qDebug() << "Calc Angles     " << theta << phi;

   accept();
}

} } // end namespace IQmol::Layer
