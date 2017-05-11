#ifndef IQMOL_DATA_SERIALIZATION_H
#define IQMOL_DATA_SERIALIZATION_H
/*******************************************************************************

  Copyright (C) 2011-2015 Andrew Gilbert

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
#include <QColor>
#include <QString>
#include <QGLViewer/vec.h>
#include <QGLViewer/quaternion.h>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 105800
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#endif
#include <exception>
#include "Matrix.h"


namespace boost {
namespace serialization {
 
// ---------- QString ----------
template<class Archive>
inline void save( Archive& ar, const QString& s, const unsigned /*version*/ )
{
	using boost::serialization::make_nvp;
	std::string stdStr(s.toStdString());
	ar << make_nvp("value", stdStr);
}
 

template<class Archive>
inline void load( Archive& ar, QString& s, const unsigned /*version*/ )
{
	using boost::serialization::make_nvp;
	std::string stdStr;
	ar >> make_nvp("value", stdStr);
	s = QString::fromStdString(stdStr);
}
 

template<class Archive>
inline void serialize(Archive& ar, QString& s, const unsigned file_version )
{
	boost::serialization::split_free(ar, s, file_version);
}
 
 
// ---------- boost::Array3D ----------
template<class Archive>
inline void save(Archive& ar, const IQmol::Array3D& m, const unsigned /* version */)
{
   size_t s0(m.shape()[0]);
   size_t s1(m.shape()[1]);
   size_t s2(m.shape()[2]);

   ar << s0 << s1 << s2;

   for (size_t i = 0; i < s0; ++i) {
       for (size_t j = 0; j < s1; ++j) {
           for (size_t k = 0; k < s2; ++k) {
               ar << m[i][j][k];
           }
       }
   }
}


template<class Archive>
inline void load(Archive& ar, IQmol::Array3D& m, unsigned const /* version */)
{
   size_t s0, s1, s2;

   ar >> s0 >> s1 >> s2;

   IQmol::Array3D::extent_gen extents;
   m.resize(extents[s0][s1][s2]);
   
   for (size_t i = 0; i < s0; ++i) {
       for (size_t j = 0; j < s1; ++j) {
           for (size_t k = 0; k < s2; ++k) {
               ar >> m[i][j][k];
           }
       }
   }
}


template<class Archive>
inline void serialize(Archive& ar, IQmol::Array3D& m, unsigned const version)
{
   boost::serialization::split_free(ar, m, version);
}


// ---------- boost::Matrix ----------
template<class Archive>
inline void save(Archive& ar, const IQmol::Matrix& m, const unsigned /* version */)
{
   size_t nRow(m.size1());
   size_t nCol(m.size2());

   ar << nRow;
   ar << nCol;

   for (size_t i = 0; i < nRow; ++i) {
       for (size_t j = 0; j < nCol; ++j) {
           ar << m(i,j);
       }
   }
}


template<class Archive>
inline void load(Archive& ar, IQmol::Matrix& m, unsigned const /* version */)
{
   size_t nCol, nRow;

   ar >> nRow;
   ar >> nCol;

   m.resize(nCol, nRow);

   for (size_t i = 0; i < nRow; ++i) {
       for (size_t j = 0; j < nCol; ++j) {
           ar >> m(i,j);
       }
   }
}


template<class Archive>
inline void serialize(Archive& ar, IQmol::Matrix& m, unsigned const version)
{
   boost::serialization::split_free(ar, m, version);
}


// ---------- QList ----------
template<class Archive, class U >
inline void save(Archive& ar, const QList<U>& list, const unsigned /* version */)
{
    //   boost::serialization::stl::save_collection< Archive, QList<U> >(ar, list);
    unsigned count = list.size (); 
    ar << BOOST_SERIALIZATION_NVP (count); 
    for (unsigned i = 0; i <count; ++ i) {
        U item = list.value (i); 
        ar << boost::serialization::make_nvp("item", item); 
    }
}


template<class Archive, class U>
inline void load(Archive& ar, QList<U>& list, unsigned const /* version */)
{
/*
   boost::serialization::stl::load_collection< 
      Archive, 
      QList<U>, 
      boost::serialization::stl::archive_input_seq<Archive, QList<U> >,
      boost::serialization::stl::no_reserve_imp< QList<U> > >(ar, list);
*/
    unsigned count(0); 
    ar >> BOOST_SERIALIZATION_NVP (count); 
    list.clear(); 
    for (unsigned i = 0; i <count; ++ i) {
        U item; 
        ar >> boost::serialization::make_nvp("item", item); 
        list.push_back(item); 
    }
}


template<class Archive, class U >
inline void serialize(Archive &ar, QList<U>& t, unsigned const version)
{
   boost::serialization::split_free(ar, t, version);
}



// ---------- QMap ----------
template<class Archive, class K, class V >
inline void save(Archive& ar, QMap<K,V> const& map, const unsigned /* version */)
{
   QList<K> keys(map.keys());
   QList<V> values;

   for (int i = 0; i < keys.size(); ++i) {
       values.append(map.value(keys[i]));
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
      throw std::runtime_error("Attempt to deserialize invalid map");
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


// ---------- QColor ----------
template<class Archive>
inline void save( Archive& ar, QColor const& color, const unsigned /*version*/ )
{
   int r(color.red());
   int g(color.green());
   int b(color.blue());
   int a(color.alpha());

   ar & r;
   ar & g;
   ar & b;
   ar & a;
}
 

template<class Archive>
inline void load( Archive& ar, QColor& color, const unsigned /*version*/ )
{
   int r, g, b, a;
   ar & r;
   ar & g;
   ar & b;
   ar & a;

   color.setRgb(r, g, b, a);
}
 

template<class Archive>
inline void serialize( Archive& ar, QColor& color, const unsigned file_version )
{
	boost::serialization::split_free(ar, color, file_version);
}


// ---------- qglviewer::Vec ----------
template<class Archive>
inline void save( Archive& ar, const qglviewer::Vec& v, const unsigned /*version*/ )
{
	ar << v.x;
	ar << v.y;
	ar << v.z;
}
 

template<class Archive>
inline void load( Archive& ar, qglviewer::Vec& v, const unsigned /*version*/ )
{
    double x, y, z;
    ar >> x;
    ar >> y;
    ar >> z;
    v.setValue(x,y,z);
}
 

template<class Archive>
inline void serialize( Archive& ar, qglviewer::Vec& v, const unsigned file_version )
{
	boost::serialization::split_free(ar, v, file_version);
}


// ---------- qglviewer::Quaternion ----------
template<class Archive>
inline void save( Archive& ar, const qglviewer::Quaternion& q, const unsigned /*version*/ )
{
    double q0(q[0]), q1(q[1]), q2(q[2]), q3(q[3]);
	ar << q0;
	ar << q1;
	ar << q2;
	ar << q3;
}


template<class Archive>
inline void load( Archive& ar, qglviewer::Quaternion& q, const unsigned /*version*/ )
{
    double q0, q1, q2, q3;
    ar >> q0;
    ar >> q1;
    ar >> q2;
    ar >> q3;
    q.setValue(q0, q1, q2, q3); 
}
 

template<class Archive>
inline void serialize( Archive& ar, qglviewer::Quaternion& q, const unsigned file_version )
{
	boost::serialization::split_free(ar, q, file_version);
}


} } // end namespace boost::serialization

BOOST_SERIALIZATION_COLLECTION_TRAITS(QList)

#endif 
