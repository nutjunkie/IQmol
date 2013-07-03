#ifndef IQMOL_CURSORS_H
#define IQMOL_CURSORS_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include <QCursor>
#include <QList>


namespace IQmol {

   /// Simple class that manages a collection of cursors.
   class Cursors {

      public:
		 /// Enumeration of the different types of cursor available.  Note that
		 /// the ordering of these must match the order they are loaded in the
		 /// constructor (see Cursors.C).
         enum Type { Arrow = 0, OpenHand, OpenHandSelection, ClosedHand, 
              ClosedHandSelection, Build, Rotate, Select, Translate, 
              TranslateSelection, Zoom };
             
         Cursors();
         ~Cursors();
         QCursor& get(Type const& type) { return *m_cursors[type]; }

      private:
         QCursor* load(QString const& file);
         QList<QCursor*> m_cursors;
   };

} // end namespace IQmol

#endif
