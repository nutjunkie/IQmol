#ifndef IQMOL_DATA_SERIALIZATION_H
#define IQMOL_DATA_SERIALIZATION_H
/*******************************************************************************

  Copyright (C) 2011-2013 Andrew Gilbert

  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.

  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************/

#include <QMap>
#include <QList>
#include <QString>
#include <QGLViewer/vec.h>
#include <QGLViewer/quaternion.h>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <exception>
 

namespace boost {
namespace serialization {
 
// ---------- QString ----------
template<class Archive>
inline void save( Archive& ar, const QString& s, const unsigned int /*version*/ )
{
	using boost::serialization::make_nvp;
	std::string stdStr(s.toStdString());
	ar << make_nvp("value", stdStr);
}
 

template<class Archive>
inline void load( Archive& ar, QString& s, const unsigned int /*version*/ )
{
	using boost::serialization::make_nvp;
	std::string stdStr;
	ar >> make_nvp("value", stdStr);
	s = QString::fromStdString(stdStr);
}
 

template<class Archive>
inline void serialize(Archive& ar, QString& s, const unsigned int file_version )
{
	boost::serialization::split_free(ar, s, file_version);
}
 
 

// ---------- QList ----------
template<class Archive, class U >
inline void save(Archive& ar, const QList<U>& t, const unsigned /* version */)
{
   boost::serialization::stl::save_collection< Archive, QList<U> >(ar, t);
}


template<class Archive, class U>
inline void load(Archive& ar, QList<U>& t, unsigned const /* version */)
{
   boost::serialization::stl::load_collection< 
      Archive, 
      QList<U>, 
      boost::serialization::stl::archive_input_seq<Archive, QList<U> >,
      boost::serialization::stl::no_reserve_imp< QList<U> > >(ar, t);
}


template<class Archive, class U >
inline void serialize(Archive &ar, QList<U>& t, unsigned const version)
{
   boost::serialization::split_free(ar, t, version);
}



// ---------- QMap ----------
template<class Archive, class K, class V >
inline void save(Archive& ar, const QMap<K,V>& map, const unsigned /* version */)
{
   QList<K> keys(map.keys());
   QList<V> values(map.values());
   if (keys.size() != values.size()) {
      throw std::runtime_error("Attempt to serialize invalid map");
   }
   ar & keys;
   ar & values;
}


template<class Archive, class K, class V >
inline void load(Archive& ar, QMap<K,V>& map, unsigned const /* version */)
{
   QList<K> keys;
   QList<V> values;
   ar & keys;
   ar & values;
   if (keys.size() != values.size()) {
      throw std::runtime_error("Attempt to serialize invalid map");
   }
   map.clear();
   for (int i = 0; i < keys.size(); ++i) {
       map.insert(keys[i], values[i]);
   }
}


template<class Archive, class K, class V >
inline void serialize(Archive& ar, QMap<K,V>& map, unsigned const version)
{
   boost::serialization::split_free(ar, map, version);
}



// ---------- qglviewer::Vec ----------
template<class Archive>
inline void save( Archive& ar, const qglviewer::Vec& v, const unsigned int /*version*/ )
{
	ar << v.x;
	ar << v.y;
	ar << v.z;
}
 

template<class Archive>
inline void load( Archive& ar, qglviewer::Vec& v, const unsigned int /*version*/ )
{
    double x, y, z;
    ar >> x;
    ar >> y;
    ar >> z;
    v.setValue(x,y,z);
}
 

template<class Archive>
inline void serialize( Archive& ar, qglviewer::Vec& v, const unsigned int file_version )
{
	boost::serialization::split_free(ar, v, file_version);
}


// ---------- qglviewer::Quaternion ----------
template<class Archive>
inline void save( Archive& ar, const qglviewer::Quaternion& q, const unsigned int /*version*/ )
{
    double q0(q[0]), q1(q[1]), q2(q[2]), q3(q[3]);
	ar << q0;
	ar << q1;
	ar << q2;
	ar << q3;
}


template<class Archive>
inline void load( Archive& ar, qglviewer::Quaternion& q, const unsigned int /*version*/ )
{
    double q0, q1, q2, q3;
    ar >> q0;
    ar >> q1;
    ar >> q2;
    ar >> q3;
    q.setValue(q0, q1, q2, q3); 
}
 

template<class Archive>
inline void serialize( Archive& ar, qglviewer::Quaternion& q, const unsigned int file_version )
{
	boost::serialization::split_free(ar, q, file_version);
}


} } // end namespace boost::serialization

BOOST_SERIALIZATION_COLLECTION_TRAITS(QList)

#endif 
