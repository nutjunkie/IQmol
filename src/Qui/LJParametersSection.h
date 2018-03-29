#ifndef QUI_LJ_PARAMETERSSECTION_H
#define QUI_LJ_PARAMETERSSECTION_H

/*!
 *  \class LJParametersSection
 *
 *  \brief A KeywordSection class representing a $lj_parameters block
 *   
 *  \author Andrew Gilbert
 *  \date March 2008
 */

#include "KeywordSection.h"
#include <map>


namespace Qui {


class LJParametersSection : public KeywordSection {
   public:
      LJParametersSection(QString data = "") : KeywordSection("lj_parameters"),
         m_data(data) { }
      ~LJParametersSection() { }

      void read(QString const& input);
      LJParametersSection* clone() const;
      void generateData(QString const&);

      static std::map<QString,QString> createMap();

   protected:
      QString dump() const;

   private:
      QString m_data;
      static std::map<QString,QString> s_parameters;
};


} // end namespace Qui
#endif
