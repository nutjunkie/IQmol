/*!
 *  \file Option.C
 *
 *  \brief Non-inline member functions of the Option class, see Option.h for details
 *
 *  \author Andrew Gilbert
 *  \date August 2008
 */

#include "Option.h"
#include <QtDebug>


namespace Qui {

Option const& Option::operator=(Option const& that) {
   if (this != &that) {
      destroy();
      copy(that);
   }
   return *this;
}
 




// Private member function
void Option::destroy() {
   m_options.clear();
}


void Option::copy(Option const& that) {
   m_name           = that.m_name;
   m_type           = that.m_type;
   m_default        = that.m_default;
   m_options        = that.m_options;
   m_description    = that.m_description;
   m_implementation = that.m_implementation;
}


void Option::optionArray(QString const& options) {
   m_options = options.split(":",QString::SkipEmptyParts);
}


QString Option::optionString() const {
   return m_options.join(":");
}


QString Option::getDefaultValue() const {
   Q_ASSERT(0 <= m_default && m_default < m_options.count());
   QString dv = m_options[m_default];
   int i(dv.indexOf("//"));
   if (i > 0) dv.truncate(i);
   return dv;
}


void Option::setType(int const& type) {
   if (type == Type_Integer) {
      m_type = Type_Integer;
   }else if (type == Type_Logical) {
      m_type = Type_Logical;
   }else if (type == Type_String) {
      m_type = Type_String;
   }else if (type == Type_Real) {
      m_type = Type_Real;
   }else if (type == Type_Array) {
      m_type = Type_Array;
   }
}


void Option::setImplementation(int const& impl) {
   if (impl == Impl_None) {
      m_implementation = Impl_None;
   }else if (impl == Impl_Combo) {
      m_implementation = Impl_Combo;
   }else if (impl == Impl_Check) {
      m_implementation = Impl_Check;
   }else if (impl == Impl_Text) {
      m_implementation = Impl_Text;
   }else if (impl == Impl_Spin) {
      m_implementation = Impl_Spin;
   }else if (impl == Impl_DSpin) {
      m_implementation = Impl_DSpin;
   }else if (impl == Impl_Radio) {
      m_implementation = Impl_Radio;
   }else {
      qDebug() << "ERROR: Option::setImplementation()\n"
               << "       Unknown Implementation type" << impl 
               << "for option" << m_name;
   }
}



int Option::intMin() const {
   int min(0);
   if (m_type == Type_Integer && m_options.count() == 4) {
      min = m_options[Limit_Min].toInt(); 
   }
   return min;
}

int Option::intMax() const {
   int max(100);
   if (m_type == Type_Integer && m_options.count() == 4) {
      max = m_options[Limit_Max].toInt(); 
   }
   return max;
}

int Option::intDefault() const {
   int def(0);
   if (m_type == Type_Integer && m_options.count() == 4) {
      def = m_options[Limit_Default].toInt(); 
   }
   return def;
}

int Option::intStep() const {
   int step(1);
   if (m_type == Type_Integer && m_options.count() == 4) {
      step = m_options[Limit_Step].toInt(); 
   }
   return step;
}


double Option::doubleMin() const {
   double min(0.0);
   if (m_type == Type_Real&& m_options.count() == 4) {
      min = m_options[Limit_Min].toDouble(); 
   }
   return min;
}

double Option::doubleMax() const {
   double max(1.0);
   if (m_type == Type_Real && m_options.count() == 4) {
      max = m_options[Limit_Max].toDouble(); 
   }
   return max;
}

double Option::doubleDefault() const {
   double def(0.0);
   if (m_type == Type_Real && m_options.count() == 4) {
      def = m_options[Limit_Default].toDouble(); 
   }
   return def;
}

double Option::doubleStep() const {
   double step(0.01);
   if (m_type == Type_Real && m_options.count() == 4) {
      step = m_options[Limit_Step].toDouble(); 
   }
   return step;
}



} // end namespace Qui
