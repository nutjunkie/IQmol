#ifndef IQMOL_NETWORK_CONNECTION_H
#define IQMOL_NETWORK_CONNECTION_H
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

#include <QObject>
#include <QThread>

#define TIMEOUT 10000

namespace IQmol {
namespace Network {

   class Reply;

   /// Base class for network connections
   class Connection : public QObject {

     Q_OBJECT

      public:
         enum Status { Closed, Opened, Authenticated };

         enum AuthenticationT { None, Agent, HostBased, KeyboardInteractive, Password,
            PublicKey };

         Connection(QString const& hostname, int const port) : QObject(), m_hostname(hostname),
            m_port(port), m_status(Closed), m_timeout(10000) { }  // 10s default
         
         virtual ~Connection() { }

         virtual void open() = 0;
         virtual void close() = 0;
         virtual void authenticate(AuthenticationT const, QString& /*userName*/) = 0;

         virtual bool blockingExecute(QString const& command, QString*) = 0;
         virtual bool exists(QString const& filePath) = 0;
         virtual bool makeDirectory(QString const& filePath) = 0;
         virtual bool removeDirectory(QString const& filePath) = 0;

         virtual QString obtainCookie() {
            return QString();
         }

         virtual Reply* execute(QString const& command) = 0;
         virtual Reply* getFile(QString const& sourcePath, QString const& destinationPath) = 0;
         virtual Reply* putFile(QString const& sourcePath, QString const& destinationPath) = 0;
         virtual Reply* getFiles(QStringList const& fileList, QString const& destinationPath) = 0;

         void setTimeout(unsigned timeout) { m_timeout = timeout; }
         unsigned timeout() const { return m_timeout; }

         Status status() const { return m_status; }
         QString const& hostname() const { return m_hostname; }
         int port() const { return m_port; }

         bool isConnected() const { return m_status == Authenticated; }

      Q_SIGNALS:
         void closing();

      protected:
         QString  m_hostname;
         int      m_port;
         Status   m_status;
         unsigned m_timeout;

         void thread(Reply*);
         void killThread();

      private:
         explicit Connection(Connection const&) : QObject() { }
         QThread  m_thread;
   };

} } // end namespace IQmol::Network

#endif
