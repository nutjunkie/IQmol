#ifndef QUI_NODE_H
#define QUI_NODE_H

/*!
 *  \class Node 
 *  
 *  \brief Node is a class whose objects can be connected with other Nodes using
 *  logical rules which can invoke Actions.  
 *  
 *  For example, let A and B be Node<int> objects and B depend on A in such a
 *  way that if A takes the value 10, then B should be set to 20, otherwise B
 *  should be set to 40.  This can be accomplished via the following:
 *  
 *  \code  
 *     Node<int> A, B;
 *     A.If(10, B.ShouldBe(20), B.ShouldBe(40));
 *  \endcode
 *  
 *  Note that this doesn't cause anthing to happen, it just sets up the rule.
 *  Later on if the following command is issued:
 *  
 *  \code  
 *     A.setValue(20);
 *  \endcode
 *  
 *  Then B is automatically updated to the value of 40.  Actually the If
 *  memeber function is more general than this and will accept any Condition
 *  and any Action (see typedefs below).
 *  
 *  \author Andrew Gilbert
 *  \date   July 2008
 */

#include <map>
#include "Logic.h"

#include <QDebug>

namespace Qui {

template <class T> class Node;

class NodeBase {
   public: 
      template <class T>
      T getValue() { 
         Node<T>* n = dynamic_cast<Node<T>*>(this);
         return n ? n->getValue() : T();
      }
};


template <class T> 
class Node : public NodeBase {

   public:
      Node() { }

      explicit Node(T const& value) : m_value(value) { }

      T getValue() const { return m_value; }

      Action shouldBe(T value) 
      {
         return boost::bind(&Node<T>::setValue, this, value); 
      }

      Action shouldBeAtLeast(T value) 
      {
         return boost::bind(&Node<T>::setAtLeast, this, value); 
      }

      // This does not appear to be working at the moment
      Action makeSameAs(Node<T>& node) 
      {
         return boost::bind(&Node<T>::setValue2, this, &node); 
      }

      virtual void setAtLeast(T const& value) 
      {
         if (m_value < value) {
            m_value = value;
            applyRules();
            emitSignals();
         }
      }

      virtual void setValue(T const& value) 
      {
         if (value != m_value) {
            m_value = value;
            applyRules();
            emitSignals();
         }
      }

      void addRule(Rule const& rule) 
      {
         m_rules.insert(
            std::make_pair( 
               new Condition(rule.get<0>()),
               new Action(rule.get<1>()) ));

         m_rules.insert(
            std::make_pair( 
               new Condition(!rule.get<0>()), 
               new Action(rule.get<2>()) ));
      }

      void applyRules() 
      {
         std::multimap<Condition*, Action*>::iterator iter;
         for (iter = m_rules.begin(); iter != m_rules.end(); ++iter) {
             if((iter->first )->operator()() ) {
                (iter->second)->operator()();
             }
         }
      }


      // This should be protected as a Register should be used for destruction
      virtual ~Node() 
      {
         std::multimap<Condition*, Action*>::iterator iter;      
         for (iter = m_rules.begin(); iter != m_rules.end(); ++iter) {
             delete iter->first;
             delete iter->second;
         }
         m_rules.clear(); 
      }


   private:
      T m_value;
      // These must be pointers due to an STL requirement
      std::multimap<Condition*, Action*> m_rules;

      void setValue2(Node<T>* node) {
         setValue(node->getValue());
      }

      virtual void emitSignals() { }

      // Don't copy nodes as the logic gets messed up
      Node(Node const& that) { }
      Node const& operator=(Node const& that) {  }
};

} // end namespace Qui

#endif
