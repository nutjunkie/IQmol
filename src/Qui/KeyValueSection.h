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

typedef QMap<QString, QString> StringMap;


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

      StringMap const& getOptions() const {
         return m_options;
      }

      void printOption(QString const& key, bool print);

      bool printOption(QString const& key) const {
         return m_toPrint.count(key) > 0;
      }

      KeyValueSection* clone() const;

      QString format() const;

      static void addAdHoc(QString const& rem, QString const& v1, QString const& v2);

   protected:
      QString dump() const;

   private:
      /// An ad-hoc map which converts values in the QUI to values used in
	  /// QChem and vice versa.  The values are stored as a QMap with keys
	  /// of the form rem::quiValue => qchemValue and also 
      /// rem::qchemValue => quiValue.  This allows a given value to be 
      /// mapped to different things depending on which option the value is 
      /// associated with.   This function is called in
      /// InputDialog::initializeControl() to avoid additional database access.
      static StringMap s_adHoc;

      static void fixAdHoc(QString const& name, QString& value);
       
	  /// Contains a list of the option values that have been explicitly set
	  /// (i.e. altered from the default.
	  StringMap m_options;

	  //! Contains a list of those options that should be printed to the input
	  //! file.  This act as a filter on the contents of m_options.
      std::set<QString> m_toPrint;
};

} // end namespace Qui
#endif
