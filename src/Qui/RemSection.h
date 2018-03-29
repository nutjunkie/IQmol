#ifndef QUI_REMSECTION_H
#define QUI_REMSECTION_H

/*!
 *  \class RemSection
 *
 *  \brief A KeywordSection class representing a $rem block
 *   
 *  \author Andrew Gilbert
 *  \date January 2008
 */

#include <QMap>
#include <set>
#include <QString>
#include "KeywordSection.h" 


namespace Qui {

class RemSection : public KeywordSection {
   public:
      RemSection() : KeywordSection("rem") { init(); }

      void read(QString const& data);
      RemSection* clone() const;

      void printOption(QString const& option, bool print);

      bool printOption(QString const& option) const {
           return m_toPrint.count(option) > 0;
      }

      QString getOption(QString const& name);

      // This is a debug function and probably shouldn't have public access.
      static void printAdHoc();

      QMap<QString,QString> getOptions() {
         return m_options;
      } 
      
      static void addAdHoc(QString const& rem, QString const& v1, QString const& v2);

      void setOption(QString const& name, QString const& value) { 
           m_options[name] = value; 
      }


   protected:
      QString dump() const;


   private:
      // ---------- Data ---------
      static QMap<QString,QString> m_adHoc;

      //! Contains a list of the option values that have been set
      QMap<QString,QString> m_options;
	  //! Contains a list of only those options that should be printed to the
	  //! input file.  This acts as a filter on the contents of the m_options
	  //! list.
      std::set<QString> m_toPrint;

      // ---------- Member Functions ---------
      void init();
      bool fixOptionForQui(QString& name, QString& value) const; 
      bool fixOptionForQChem(QString& name, QString& value) const;

      //bool printOption(QString const& option) const {
      //   return m_toPrint.count(option) > 0;
      //}

      void setOptions(QMap<QString,QString> const& options);
      void setPrint(std::set<QString> const& toPrint);
};

} // end namespace Qui
#endif
