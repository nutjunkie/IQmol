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

#include "Timer.h"
#include "QsLog.h"
#include <QStringList>


namespace IQmol {

const int MilliSecondsInDay = 24 * 60 * 60 * 1000;

Timer::Timer(QObject* parent) : QObject(parent), m_seconds(0), m_running(false)
{ 
   m_dayTimer.setInterval(MilliSecondsInDay);
   connect(&m_dayTimer, SIGNAL(timeout()), this, SLOT(anotherDay()));
   reset();
}


int Timer::toSeconds(QString const& s)
{
   if (s.isEmpty()) return -1;
   // Remove any partial seconds
   QString t = s.split(".", QString::SkipEmptyParts).first();
   QStringList tokens(t.split(":",QString::SkipEmptyParts));

   int hours(0), minutes(0), seconds(0);
   bool okH(true), okM(true), okS(true);
   int n(tokens.size());

   if (n >= 1) seconds = tokens[n-1].toInt(&okS);
   if (n >= 2) minutes = tokens[n-2].toInt(&okM);
   if (n >= 3) hours   = tokens[n-3].toInt(&okH);

   if ((okS && okM && okH)) return 60*(60*hours + minutes) + seconds;

   QLOG_DEBUG() << "Invalid time format in Process::resetTimer: " << s;
   return -1;
}


void Timer::reset(int const seconds)
{
   m_seconds = seconds;
   m_formattedTime = formatTime(m_seconds);
}


void Timer::reset()
{
   m_seconds = 0;
   m_dayTimer.stop();
   m_formattedTime = formatTime(m_seconds);
}


void Timer::start()
{
   if (m_running) return;
   m_timer.start();
   m_dayTimer.start();
   m_running = true;
}


void Timer::stop()
{
   if (!m_running) return;
   m_dayTimer.stop();
   m_seconds += m_timer.elapsed()/1000;
   m_formattedTime = formatTime(m_seconds);
   m_running = false;
}


void Timer::anotherDay()
{ 
   m_seconds += 24*60*60;
   m_timer.start();
}


QString Timer::toString() const
{
   QString time;
   if (m_running) {
      int seconds(m_seconds);
      seconds += m_timer.elapsed()/1000;
      time = formatTime(seconds);
   }else {
      time = m_formattedTime;
   }
   return time;
}


QString Timer::formatTime(int const seconds) const
{
   int time(seconds);
   int secs = time % 60;
   time /= 60;
   int mins = time % 60;
   time /= 60;
   int hours = time;

   return QTime(hours, mins, secs).toString("h:mm:ss");
}



} // end namespace IQmol
