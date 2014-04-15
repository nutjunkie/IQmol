#ifndef QUI_CONDITIONS_H
#define QUI_CONDITIONS_H

/*!
 *  \brief Declarations for custom Condition functions.  See Logic.h for details.
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "QtNode.h"


namespace Qui {
// Node Logic functions.  These are used for overloading the operators below
// and do not form part of the Node interface.
template <class T>
inline bool Equals(Node<T> const* node, T const& value) 
{
   qDebug() << "Inside Equals" << node->getValue() << value << (node->getValue() == value);
   return (node->getValue() == value);
}


//Overloaded versions for Strings to allow case insensitive comparisons
inline bool Equals2(QtNode const* node, QString const& value) 
{
   bool tf = (QString::compare(node->getValue(), value, Qt::CaseInsensitive) == 0);
   qDebug() << "Inside Equals 2" << node->name() << node->getValue() << value << tf;
   return tf;
}


template <class T>
inline bool LessThan(Node<T> const* node, T const& value) 
{
   return node->getValue() < value;
}


// Node Logic Operators.  These are all overloaded to return Conditions rather
// than bools
template <class T>
inline Condition const operator==(T const& value, Node<T> const& node) 
{
   return boost::bind(&Equals<T>, &node, value);
}

template <class T>
inline Condition const operator==(Node<T> const& node, T const& value) 
{
   qDebug() << "Operator==<NodeT> required" << node.getValue() << value;
   return boost::bind(&Equals<T>, &node, value);
} 

inline Condition const operator==(QtNode const& node, const char* value) 
{
   qDebug() << "Operator == required" << node.getValue() << QString(value);
   return boost::bind(&Equals2, &node, QString(value));
} 


// operator !=
inline Condition const operator!=(QtNode const& node, const char* value) 
{
   return !(node == value);
} 


// operator <
template <class T>
inline Condition const operator<(T const& value, Node<T> const& node) 
{
   return boost::bind(&LessThan<T>, &node, value);
} 

template <class T>
inline Condition const operator<(Node<T> const& node, T const& value) 
{
   return boost::bind(&LessThan<T>, &node, value);
} 


// operator !=
template <class T>
inline Condition const operator!=(T const& value, Node<T> const& node) 
{
   return !(node == value);
} 

template <class T>
inline Condition const operator!=(Node<T> const& node, T const& value) 
{
   return !(node == value);
} 

template <class T>
inline Condition const operator!=(Node<T> const& node1, Node<T> const& node2) 
{
   return !(node1 == node2);
} 


// operator >
template <class T>
inline Condition const operator>(T const& value, Node<T> const& node) 
{
   return !(node == value) && !(node < value);
} 

template <class T>
inline Condition const operator>(Node<T> const& node, T const& value) 
{
   return !(node == value) && !(node < value);
} 

template <class T>
inline Condition const operator>(Node<T> const& node1, Node<T> const& node2) 
{
   return !(node1 == node2) && !(node1 < node2);
} 


// operator <=
template <class T>
inline Condition const operator<=(T const& value, Node<T> const& node) 
{
   return !(node > value);
} 

template <class T>
inline Condition const operator<=(Node<T> const& node, T const& value) 
{
   return !(node > value);
} 

template <class T>
inline Condition const operator<=(Node<T> const& node1, Node<T> const& node2) 
{
   return !(node1 > node2);
} 


// operator >=
template <class T>
inline Condition const operator>=(T const& value, Node<T> const& node) 
{
   return !(node < value);
} 

template <class T>
inline Condition const operator>=(Node<T> const& node, T const& value) 
{
   return !(node < value);
} 

template <class T>
inline Condition const operator>=(Node<T> const& node1, Node<T> const& node2) 
{
   return !(node1 < node2);
} 

} // end namespace Qui

#endif
