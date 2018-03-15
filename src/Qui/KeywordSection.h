#ifndef QUI_KEYWORDSECTION_H
#define QUI_KEYWORDSECTION_H

/*!
 *  \class KeywordSection
 *
 *  \brief An abstract base class representing containers for holding $section
 *  data.  This base class primarily defines the I/O interface.
 *   
 *  \author Andrew Gilbert
 *  \date January 2008
 */

#include <QString>


namespace Qui {

class KeywordSection {

   public:
      KeywordSection(QString const& name, bool print = true) 
       : m_print(print), m_name(name) { }

      virtual ~KeywordSection() { }

      QString name() const { return m_name; }
      void print(bool print) { m_print = print; }
      bool print () const { return m_print; }

	  //! This is just a wrapper for dump() which checks the m_print flag and
	  //! is what should be called.
      QString format();

      virtual void read(QString const&) = 0;
      virtual KeywordSection* clone() const = 0;


   protected:
      virtual QString dump() const = 0;  
      bool    m_print;
      QString m_name;


   private:
      // This should prevent copying sections
      KeywordSection(KeywordSection const& that);
      KeywordSection const& operator=(KeywordSection const& that);
};



// Non-member functions
//! A factory for generating KeywordSections
KeywordSection* KeywordSectionFactory(QString const& type);



//! GenericSection is used in cases where no specialized section exists.
class GenericSection :  public KeywordSection {
   public:
      GenericSection(QString const& name, QString const& data = "", bool print = true) 
        : KeywordSection(name, print), m_data(data) { }

      void read(QString const& data);

      QString rawData();

      GenericSection* clone() const;

    protected: 
	  QString dump() const;

    private:
      QString m_data;
};


} // end namespace Qui
#endif
