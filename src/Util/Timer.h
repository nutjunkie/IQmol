#ifndef IQMOL_UTIL_TIMER_H
#define IQMOL_UTIL_TIMER_H
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

#include <QTime>
#include <QTimer>


namespace IQmol {
namespace Util {

   /// Low resolution (second) Timer class that measures time periods over a day.
   class Timer : public QObject {

      Q_OBJECT
      public:
          Timer(QObject* parent = 0);
          ~Timer() { }

          bool isRunning() const { return m_running; }
          void reset(unsigned const seconds);
          unsigned time() const;
          QString toString() const;

		  /// Takes a string formatted as hh:mm:ss or hh:mm:ss.xx 
          /// and returns the total number of seconds.  
          /// Returns 0 if the format is incorrect
          static unsigned toSeconds(QString const&);
          static QString formatTime(unsigned const seconds);

      public Q_SLOTS:
         void start();
         void reset();
         void stop();

      private Q_SLOTS:
         void anotherDay();

      private:
         unsigned  m_seconds;
         bool m_running;

         QTime   m_timer;
         QTimer  m_dayTimer;
   };

} } // end namespace IQmol::Util

#endif
