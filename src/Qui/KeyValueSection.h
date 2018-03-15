#ifndef QUI_KEYVALUESECTION_H
#define QUI_KEYVALUESECTION_H

/*!
 *  \class KeyValueSection 
 *
 *  \brief A KeywordSection made up of keys and values, such as $rem and $solvent.
 *   
 */

#include <QMap>
#include <set>
#include <QString>

#include "KeywordSection.h"


namespace Qui {


class KeyValueSection : public KeywordSection 
{
   public:
      KeyValueSection(QString const& name, bool print = true);

      virtual ~KeyValueSection();

      void read(QString const& input);

      void setOption(QString const& key, QString const& value);

      QString getOption(QString const& key) const {
         return m_options.count(key) > 0 ? m_options[key] : QString();
      }

      QMap<QString,QString> const& getOptions() const {
         return m_options;
      }

      void printOption(QString const& key, bool print);

      bool printOption(QString const& key) const {
         return m_toPrint.count(key) > 0;
      }

      KeyValueSection* clone() const;

      QString format() const;

   protected:
      QString dump() const;

   private:
      //! Contains a list of the option values that have been set
      QMap<QString,QString> m_options;
	  //! Contains a list of those options that should be printed to the input
	  //! file.  This act as a filter on the contents of m_options.
      std::set<QString> m_toPrint;
};

} // end namespace Qui
#endif
