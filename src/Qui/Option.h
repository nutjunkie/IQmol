#ifndef QUI_OPTION_H
#define QUI_OPTION_H

/*!
 *  \class Option
 *
 *  \brief An Option object contains all the information related to a single
 *  program option.  The idea is that Options are smart configuration options
 *  that know what type they are (integer, string, etc.) the default value,
 *  valid options and embedded documentation. 
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */

#include <QString>
#include <QStringList>


namespace Qui {


class Option {

   friend class OptionDatabase;
   friend class OptionDatabaseForm;

   public:
      enum Type { Type_Integer, Type_Logical, Type_String,  
                  Type_Real,    Type_Array };

      enum Impl { Impl_None, Impl_Combo, Impl_Check, 
                  Impl_Text, Impl_Spin,  Impl_DSpin, Impl_Radio };
                  
      enum Field { Field_Name,    Field_Type,        Field_Default,  
                   Field_Options, Field_Description, Field_Implementation };

      enum Limit { Limit_Min, Limit_Max, Limit_Default, Limit_Step };

	  Option(QString const& name = QString(), 
             Type const& type = Type_Integer,
			 int const& defaultValue = 0, 
             QString const& options = QString(),
			 QString const& description = QString(), 
             Impl const& implementation = Impl_None) 
	   : m_name(name), m_type(type), m_default(defaultValue),  
		 m_description(description), m_implementation(implementation) {
         optionArray(options);
      }

      ~Option() { destroy(); }

      Option(Option const& that) { copy(that); }

      Option const& operator=(Option const& that);

      // Accessors 
      QString const&     getName()        const { return m_name; }
      QString const&     getDescription() const { return m_description; }
      QStringList const& getOptions()     const { return m_options; }
      Type getType()            const { return m_type; }
      int  getDefaultIndex()    const { return m_default; }
      Impl getImplementation()  const { return m_implementation; }
      QString getOptionString() const { return optionString(); }
      QString getDefaultValue() const;
         
      int intMin() const;
      int intMax() const;
      int intDefault() const;
      int intStep() const;

      double doubleMin() const;
      double doubleMax() const;
      double doubleDefault() const;
      double doubleStep() const;


   protected:
      // Mutators
      void setName(QString const& name) { m_name = name.toUpper(); }
      void setType(Type const& type) { m_type = type; }
      void setDefault(int const& defaultValue) { m_default = defaultValue; }
      void setOptions(QStringList const& options) { m_options = options; }
      void setDescription(QString const& description) { m_description = description; }
      void setImplementation(Impl const& impl) { m_implementation = impl; }
      void setOptions(QString const& options) { optionArray(options); }
      void setType(int const& type);
      void setImplementation(int const& impl);


   private:
      // Data members
      QString m_name;
      Type m_type;
      int m_default;          // this is the default index in the m_options array
      QString m_description;
      Impl m_implementation;
      QStringList m_options;
       

      // Private member functions
      void copy(Option const&);
      void destroy();

      void optionArray(QString const& options);
      QString optionString() const;
};

} // end namespace Qui
#endif
