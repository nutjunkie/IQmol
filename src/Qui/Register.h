#ifndef QUI_REGISTER_H
#define QUI_REGISTER_H

/*!
 *  \class Register 
 *  
 *  \brief Implements a singleton which provides a global interface to
 *  instances of a class.  The Register takes ownership of the objects and
 *  takes care of the necessary destruction at program termination.  Objects
 *  have an associated key used for retrieval through the Register.
 *  
 *  \author Andrew Gilbert
 *  \date   July 2008
 */

#include <map>
#include <QString>

using namespace std;

namespace Qui {

template <class K, class T>
class Register {

   public:
      static Register& instance() {
         if (s_instance == 0) {
            s_instance = new Register();
            atexit(Register::destroy);
         }
         return *s_instance;
      }

      T& add(K key, T*& item) { 
         remove(key);
         s_items.insert(std::make_pair(key,item)); 
         item = 0;  // we take over ownership of the resource
         return *s_items[key];
      }

      void remove(K key) {
         if (exists(key)) {
            delete s_items[key];
            s_items.erase(key);
         }
      }

      bool exists(K key) {
         return s_items.find(key) != s_items.end();
      }

      virtual T& get(K key) { 
         if (exists(key)) {
            return *s_items[key];
         } else {
            //s_items.insert(std::make_pair(key, new T));
            // HACK!!! - why is this useful?
            s_items.insert(std::make_pair(key, new T(key)));
            return *s_items[key];
         }
      }


   private:
      static Register* s_instance;
      static std::map<K, T*> s_items;

      Register() { }
      explicit Register(Register const&) { }
      virtual ~Register() { }

      static void destroy() {
         typename std::map<K,T*>::iterator iter;
         for (iter = s_items.begin(); iter != s_items.end();) {
             delete iter->second;
             s_items.erase(iter++);
         }
      }
};


template<class K, class T>
Register<K, T>* Register<K,T>::s_instance = 0;
template<class K, class T>
std::map<K,T*> Register<K,T>::s_items;

} // end namespace Qui


#endif
