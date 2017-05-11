#ifndef IQMOL_CONFIGURATOR_H
#define IQMOL_CONFIGURATOR_H
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

#include <QRect>
#include <QDialog>
#include <QCloseEvent>
#include <QApplication>


namespace IQmol {
namespace Configurator {

   /// Abstract base class for dialogs that 'configure' options associated with
   /// a Layer::Base object.  Configure is used loosely, for example configuring
   /// a Layer::File simply displays the contents of the file.  Most Layer objects
   /// have their own configurator which is activated by double clicking the Layer
   /// in the ModelView window of IQmol.  This base class implements the default
   /// display behaviour as well as saving the coordinates of the dialog so that
   /// when it is reopened it does so in the same place as the user put it.
   class Base : public QDialog {

      Q_OBJECT

      public:
         explicit Base() : QDialog(QApplication::activeWindow()), m_initialized(false) { }
         virtual ~Base() { }

		 /// Initialization function that should take care of anything that
		 /// only needs to be done once.  In most cases this does not need to
		 /// be called explicitly as it will be called the first time display()
		 /// is called.
		 virtual void init() { m_initialized = true; }
         
		 /// Displays the configurator dialog.  The dialog will appear
		 /// initially in the middle of the MainWindow and subsequently where 
		 /// the user closed it.
		 void display() {
            if (!m_initialized) {
               init();
               m_initialized = true;
            }
            if (!m_geometry.isNull()) setGeometry(m_geometry);
            sync();
            show();
            raise();
         }

      public Q_SLOTS:
		 /// Synchronization function that should copy data from the associated
		 /// Layer::Base object to the dialog.  This should be called explictly
		 /// whenever Layer data is changed programatically.
         virtual void sync() { }

      protected Q_SLOTS:
		 /// QCloseEvent handler that saves the location of the dialog so that
		 /// it can be reopened at the same place.
         virtual void closeEvent(QCloseEvent* event) {
            m_geometry = geometry();
            event->accept();
         }

      private:
         bool m_initialized;
         QRect m_geometry;
   };

} } // end namespace IQmol::Layer

#endif
