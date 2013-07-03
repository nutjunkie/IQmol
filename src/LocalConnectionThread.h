#ifndef IQMOL_LOCALCONNECTIONTHREAD_H
#define IQMOL_LOCALCONNECTIONTHREAD_H
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

#include "Threaded.h"
#include "HostDelegate.h"


namespace IQmol {
namespace LocalConnection {

   class Exec : public Threaded {

      Q_OBJECT

      public:
         Exec(QString const& command, int const timeout = 5000) 
          : m_command(command), m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_command;
         int m_timeout;
   };



   class Exists : public Threaded {
      
      Q_OBJECT

      public:
         Exists(QString const& filePath, HostDelegate::FileFlags const flags,
            int timeout = 5000) : m_filePath(filePath), m_flags(flags), m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_filePath;
         HostDelegate::FileFlags m_flags;
         int m_timeout;
   };



   class Move : public Threaded {

      Q_OBJECT

      public:
         Move(QString const& sourceFilePath, QString const& destinationFilePath, 
            int timeout = 5000) : m_sourceFilePath(sourceFilePath), 
            m_destinationFilePath(destinationFilePath), m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_sourceFilePath;
         QString m_destinationFilePath;
         int m_timeout;
   };



   class Remove : public Threaded {

      Q_OBJECT

      public:
         Remove(QString const& filePath, int timeout = 5000) : m_filePath(filePath), 
            m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_filePath;
         int m_timeout;
   };



   class Grep : public Threaded {

      Q_OBJECT

      public:
         Grep(QString const& string, QString const& filePath, int timeout = 5000) 
           : m_string(string), m_filePath(filePath), m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_string;
         QString m_filePath;
         int m_timeout;
   };



   class CheckOutputForErrors : public Threaded {

      Q_OBJECT

      public:
         CheckOutputForErrors(QString const& filePath, int timeout = 5000) 
           : m_outputFilePath(filePath), m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_outputFilePath;
         int m_timeout;
   };



   class MakeDirectory : public Threaded {

      Q_OBJECT

      public:
         MakeDirectory(QString const& path, int timeout = 5000) : m_path(path),
            m_timeout(timeout) { }
      protected:
         void run();
      private:
         QString m_path;
         int m_timeout;
   };


} } // end namespace IQmol::LocalConnection

#endif
