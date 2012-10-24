#ifndef QUI_QTNODE_H
#define QUI_QTNODE_H

/*!
 *  \class QtNode 
 *  
 *  \brief Extends the Node class to use Qt style signals and slots.  This
 *  allows the Nodes to be connected to QObjects.
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include <QObject>
#include <QString>
#include "Node.h"


namespace Qui {


// Note the order of inheritance is important
class QtNode : public QObject, public Node<QString> {

   Q_OBJECT
   
   public:
      explicit QtNode(QString const& name = QString(), 
         QString const& value = QString()) : 
         Node<QString>(value), m_name(name) { }


   public Q_SLOTS:
      void setValue(QString const& value) {
         if (Node<QString>::getValue() != value) {
            Node<QString>::setValue(value);
         }
      }


   Q_SIGNALS:
      void valueChanged(QString const& name, QString const& newValue);
      void valueChanged(int newValue);


   protected:
      void emitSignals() {
         valueChanged(m_name,getValue());
      }


   private:
      QString m_name;
};


} // end namespace Qui

#endif
