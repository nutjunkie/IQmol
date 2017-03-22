#ifndef IQMOL_PROCESS_QUEUESYSTEM_H
#define IQMOL_PROCESS_QUEUESYSTEM_H
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


namespace IQmol {
namespace Process {

   /// Base class for queueing systems on a server.
   class QueueSystem {

      public:
         // required for serialization
         enum Type { None, Basic, PBS, SGE };

         static QueueSystem* Factory(Type const);

         Transaction* setup() = 0;
         Transaction* submit() = 0;
         Transaction* query() = 0;
         Transaction* cleanup() = 0;
         Transaction* kill() = 0;

      protected:
         QueueSystem() { }

      private:
         // No copying
         QueueSystem(QueueSystem const&);
         QueueSystem& operator=(QueueSystem const&);
   };

} } // end namespace IQmol::Process

#endif
