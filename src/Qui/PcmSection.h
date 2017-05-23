#ifndef QUI_PCMSECTION_H
#define QUI_PCMSECTION_H

/*!
 *  \class PcmSection 
 *
 *  \brief A KeywordSection class representing a $pcm block
 *   
 *  \author Zhi-Qiang You
 *  \date March 2017 
 */

#include "KeywordSection.h"


namespace Qui {


class PcmSection : public KeywordSection {
   public:
      PcmSection(QString const& data = "", bool print = true) 
       : KeywordSection("pcm", print), m_data(data.trimmed()) {
         processData();
      }

      ~PcmSection() {  }

      void read(QString const& input);
      PcmSection* clone() const;
      QString previewFormat() const;

   protected:
      QString dump();

   private:
      QString m_data;

      void processData();
};


} // end namespace Qui
#endif
