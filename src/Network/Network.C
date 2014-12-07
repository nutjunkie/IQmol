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

#include "HttpConnection.h"
#include "HttpReply.h"
#include "SshConnection.h"
#include "SshReply.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "NetworkException.h"
#include <QEventLoop>
#include <QRegExpValidator>
#include "Network.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif


namespace IQmol {
namespace Network {


   //Network::HttpConnection* http = new Network::HttpConnection("cactus.nci.nih.gov");
   //if (http->openConnection()) {
   //   http->request("chemical/structure/Benzene/smiles");
   //   http->request("chemical/structure/C1=CC=CC=C1/names");
  // }


bool TestNetworkConnection()
{
   bool okay(true);

   try {
      HttpConnection http("iqmol.q-chem.com");
      http.open();

      QEventLoop loop;
      Reply* reply;
/*
      QStringList input;

      input << "$molecule"
            << "0  1"
            << "He"
            << "$end"

            << "$rem"
            << "  exchange  hf"
            << "  gui       2"
            << "  basis     6-31G"
            << "$end";

      reply = http.post("submit", input.join("\n"));


      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "--------- SUCCESS -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }
      else {
         QLOG_DEBUG() << "----------- ERROR -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }

      delete reply;

*/


      // reply = http.get("list?jobid=db33b0f063b511e4bda190b11c4a068c");

      //reply = http.get("status?jobid=db33b0f063b511e4bda190b11c4a068c");

      //reply = http.get("register");
      //reply = http.get("download?jobid=22dbd72c647211e4a1d190b11c4a068c&cookie=4b6d8c1c5b3743efbc2acd550887b7a4&file=input.FChk");
      reply = http.get("download?cookie=4b6d8c1c5b3743efbc2acd550887b7a4&jobid=22dbd72c647211e4a1d190b11c4a068c&file=input.FChk");

      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "--------- SUCCESS -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }
      else {
         QLOG_DEBUG() << "----------- ERROR -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }

      delete reply;

      reply = http.get("list?jobid=db33b0f063b511e4bda190b11c4a068c&cookie=4b6d8c1c5b3743efbc2acd550887b7a4");

      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "--------- SUCCESS -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }
      else {
         QLOG_DEBUG() << "----------- ERROR -------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "-------------------------------";
      }

      delete reply;

      SshConnection raijin("raijin.nci.org.au");
      raijin.open();
      
      QString name("atg509");
      raijin.authenticate(SshConnection::Password, name);
      QLOG_TRACE() << "Authentication successful";

      reply = raijin.execute("ls");
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "----------------------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }
      delete reply;

      reply = raijin.getFile("/home/509/atg509/Test.vert", "Test.vert");
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "------ GET File ------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }
      delete reply;

      reply = raijin.putFile("/Users/agilbert/testcopy1", "testcopy5");
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "------- PUT File -----------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }
      delete reply;

      return okay;

      SshConnection chemmac11("chemmac11");
      chemmac11.open();
      chemmac11.authenticate(SshConnection::KeyboardInteractive, name);

      reply = chemmac11.execute("ls");
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "----------------------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }
      delete reply;



//    reply = chemmac11.getFile("/Users/agilbert/alkane.out2", "junk-101");
      reply = chemmac11.getFile("/tmp/test.copy", "test.copy");

//    reply = chemmac11.getFile("aniline.inp", "junk-102");
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

      loop.exec();

      okay = (reply->status() == Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "----------------------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }else if (reply->status() == Reply::Error) {
         QMsgBox::warning(0, "IQmol", reply->message());
         QLOG_ERROR() << reply->message();
      }
      delete reply;


/*
     SshConnection* junk = new SshConnection("localhost");
     QLOG_TRACE() << "About to request A";
     SshReply* reply1(junk->request("A"));
     QLOG_TRACE() << "About to request B";
     SshReply* reply2(junk->request("B"));

     sleep(5);
     QLOG_TRACE() << "Sending interrupt";
     reply1->interrupt(); 
     

     QObject::connect(reply2, SIGNAL(finished()), junk, SLOT(deleteLater()));
*/

   // These catch the connection errors, not the reply errors
   }catch (AuthenticationError& err) {
      QMsgBox::warning(0, "IQmol", "Invalid username or password");

   }catch (Exception& err) {
      okay = false;
      QMsgBox::warning(0, "IQmol", err.what());
   }

   return okay;
}


in_addr_t HostLookup(QString const& hostname)
{
   in_addr_t address(INADDR_NONE);

#ifdef WIN32

   // This can only handle IPv4 addresses and should only be used when
   // inet_ntop is unavailable.
   QString octet("(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])");
   QRegExpValidator ipv4Validator(0);
   ipv4Validator.setRegExp(QRegExp(
      "^" + octet + "\\." + octet + "\\." + octet + "\\." + octet + "$"));

   int pos;
   QString tmp(hostname);

   if (ipv4Validator.validate(tmp,pos) == QValidator::Acceptable) {
      address = inet_addr(hostname.toLatin1().data());
      if (address == INADDR_ANY || address == INADDR_NONE) {
         throw InvalidHostname(hostname);
      }
   }else {
      struct hostent* host;
      host = gethostbyname(hostname.toLatin1().data());
      if (host->h_addrtype == AF_INET6) {
         throw Exception("IPv6 addresses are not supported");
      }else if (host) {
         address = *(in_addr_t*)host->h_addr;
      }
   }

#else

   struct addrinfo hints, *res;
   int errcode;
   char addrstr[100];
   void *ptr(0);

   memset(&hints, 0, sizeof (hints));
   hints.ai_family = PF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags |= AI_CANONNAME;

   errcode = getaddrinfo(hostname.toLatin1().data(), NULL, &hints, &res);
   if (errcode != 0) throw InvalidHostname(hostname);
 
   inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

   switch (res->ai_family) {
      case AF_INET:
         ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
         break;
      case AF_INET6:
         ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
         break;
   }

   inet_ntop(res->ai_family, ptr, addrstr, 100);
   address = inet_addr(addrstr);

   QString ipv((res->ai_family == PF_INET6) ? "IPv6 address:" : "IPv4 address:");
   QLOG_DEBUG() << ipv << QString(addrstr) << "=>" << QString(res->ai_canonname);

#endif

   return address;
}

} } // end namespace IQmol::Network
