#ifndef QUI_EXTERNALCHARGESSECTION_H
#define QUI_EXTERNALCHARGESSECTION_H

/*!
 *  \class ExternalChargesSection 
 *
 *  \brief A KeywordSection class representing a $external_charges block
 *   
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "KeywordSection.h"


namespace Qui {


class ExternalChargesSection : public KeywordSection {
   public:
      ExternalChargesSection(QString const& data = "", bool print = true) 
       : KeywordSection("external_charges", print), m_data(data.trimmed()) {
         processData();
      }

      ~ExternalChargesSection() {  }

      void read(QString const& input);
      ExternalChargesSection* clone() const;
      QString previewFormat() const;

   protected:
      QString dump() const;

   private:
      QString m_data;
      QString m_truncatedData;

      void processData();
};


} // end namespace Qui
#endif
