#ifndef IQMOL_NETWORK_H
#define IQMOL_NETWORK_H
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

#include <climits> // for UINT_MAX
#include <QString>

namespace IQmol {
namespace Network {

   static const unsigned Unlimited = UINT_MAX;

   enum Authentication { None = 0, 
                         Agent, 
                         PublicKey, 
                         HostBased,
                         KeyboardInteractive,  
                         Prompt,  
                         Cookie  };


   QString ToString(Authentication const& authentication) 
   {
      QString s;

      switch (authentication) {
         case None:                 s = "None";                  break;
         case Agent:                s = "Agent";                 break;
         case PublicKey:            s = "Public Key";            break;
         case HostBased::           s = "Host Based";            break;
         case KeyboardInteractive:  s = "Keyboard Interactive";  break;
         case Prompt:               s = "Prompt";                break;
         case Cookie:               s = "Cookie";                break;
      }

      return s;
   }


   Authentication AuthenticationFromString(QString const& s)
   {
      if (s.contains("Agent",    Qt::CaseInsensitive))  return Agent;
      if (s.contains("Public",   Qt::CaseInsensitive))  return PublicKey;
      if (s.contains("Host",     Qt::CaseInsensitive))  return HostBased;
      if (s.contains("Keyboard", Qt::CaseInsensitive))  return KeyboardInteractive;
      if (s.contains("Prompt",   Qt::CaseInsensitive))  return Prompt;
      if (s.contains("Cookie",   Qt::CaseInsensitive))  return Cookie;
      return None;
   }


   enum Protocol { Local = 0, 
                   SSH, 
                   HTTP  };


   QString ToString(Protocol const& protocol)
   {
      QString s;

      switch (protocol) {
         case Local:  s = "Local";  break;
         case SSH:    s = "SSH";    break;
         case HTTP:   s = "HTTP";   break;
      }

      return s;
   }


   Protocol ProtocolFromString(QString const& s) 
   {
      if (s.contains("SSH",   Qt::CaseInsensitive))  return SSH;
      if (s.contains("HTTP",  Qt::CaseInsensitive))  return HTTP;
      return Local;
   }


   enum QueueSystem { Basic = 0,
                      PBS, 
                      SGE, 
                      Web  };


   QString ToString(QueueSystem const& queueSystem)
   {
      QString s;

      switch (queueSystem) {
         case Basic:  s = "Basic";  break;
         case PBS:    s = "PBS";    break;
         case SGE:    s = "SGE";    break;
         case Web:    s = "Web";    break;
      }

      return s;
   }


   QueueSystemFromString(QString const& s) 
   {
      if (s.contains("Basic",  Qt::CaseInsensitive))  return Basic;
      if (s.contains("PBS",    Qt::CaseInsensitive))  return PBS;
      if (s.contains("SGE",    Qt::CaseInsensitive))  return SGE ;
      if (s.contains("Web",    Qt::CaseInsensitive))  return Web ;
      return Basic;
   }


} } // end namespace IQmol::Network

#endif
