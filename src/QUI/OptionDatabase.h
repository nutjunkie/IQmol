#ifndef QUI_OPTIONDATABASE_H
#define QUI_OPTIONDATABASE_H

/*!  
 *  \class OptionDatabase
 *   
 *  \brief Essentially a wrapper class around an sqlite database holding
 *  information on program options.  The OptionDatabase is implemented as a
 *  singleton to prevent multiple instances, references to which can be
 *  obtained via the following construct:
 *   
 *  \code
 *    OptionDatabase &myOptionDatabase = OptionDatabase::instance();
 *  \endcode
 *   
 *  \b Note: 
 *   - Valid option values are stored in the database as a single string with
 *     options separated by ':'.  This means a colon cannot be used in the
 *     value string.
 *   - Replacements can be entred into the database using '//'. So if the
 *     option is entered as 'a//b' it will appear in the interface as a, and be
 *     replaced with b in the input file.  This means the forward slash should
 *     not be used in the value string either.
 *   - The above has been extended to allow a//b1|b2|... so that several qchem
 *     values can be read from file.
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */


class QString;
class QStringList;

namespace Qui {

class Option;

class OptionDatabase {

   public:
      static OptionDatabase& instance();

      bool databaseLoaded() const { return s_okay;}
      bool insert(Option const& opt, bool const promptOnOverwrite = true);
      bool remove(QString const& name, bool const prompt = true);
      bool get(QString const& name, Option& opt);
      QStringList all();

   private:
      OptionDatabase();
      explicit OptionDatabase(OptionDatabase const&) { }
      ~OptionDatabase() { }
      static void destroy();
      bool execute(QString const&);

      static bool s_okay;
      static OptionDatabase* s_instance;   
};

} // end namespace Qui

#endif
