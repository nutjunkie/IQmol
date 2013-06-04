#ifndef IQMOL_HTTPTHREAD_H
#define IQMOL_HTTPTHREAD_H
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

#include "Threaded.h"
#include <QUrl>


class QFile;
class QHttp;
class QBuffer;
class QEventLoop;
class QHttpResponseHeader;

namespace IQmol {

   class HttpHost;

   class HttpThread : public Threaded {

      Q_OBJECT

      public:
         HttpThread(QUrl const& url, int const timeout);
         ~HttpThread();

         QString block() {
            start();
            wait();
            return m_outputMessage;
         }
         
         bool active() const { return m_httpActive; }

      Q_SIGNALS:
         void copyProgress(double);
         void killEventLoop();
         

      public Q_SLOTS:
         void stopWhatYouAreDoing();

      protected:
         void run();
         QString lastError();


      private Q_SLOTS:
         void httpRequestFinished(int, bool); 
         void updateDataReadProgress(int, int);
         void readResponseHeader(QHttpResponseHeader const&);

      private:
         void downloadFile();
         void getResponse();
         QEventLoop* m_eventLoop;
         QHttp* m_http;
         QUrl m_url;
         int m_httpGetId;
         QFile* m_file;
         QBuffer* m_buffer;
         bool m_httpActive;
   };


} // end namespace IQmol

#endif
