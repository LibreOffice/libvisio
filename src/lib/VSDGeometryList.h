/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libvisio
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#ifndef __VSDGEOMETRYLIST_H__
#define __VSDGEOMETRYLIST_H__

#include <vector>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <boost/optional.hpp>

namespace libvisio
{

// forward declared because it's default tempate arg for sorted_vector
template<class Value, class Compare>
struct find_unique;

/** Represents a sorted vector of values.

    @tpl Value class of item to be stored in container
    @tpl Compare comparison method
    @tpl Find   look up index of a Value in the array
*/
template<typename Value, typename Compare = std::less<Value>,
         template<typename, typename> class Find = find_unique >
class sorted_vector
  : private std::vector<Value>
{
private:
  typedef Find<Value, Compare> Find_t;
  typedef typename std::vector<Value> base_t;
  typedef typename std::vector<Value>::iterator  iterator;
public:
  typedef typename std::vector<Value>::const_iterator const_iterator;
  typedef typename std::vector<Value>::size_type size_type;

  using base_t::clear;
  using base_t::empty;
  using base_t::size;

  // MODIFIERS

  std::pair<const_iterator,bool> insert( const Value &x )
  {
    std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
    if (!ret.second)
    {
      const_iterator const it = base_t::insert(
                                  begin_nonconst() + (ret.first - begin()), x);
      return std::make_pair(it, true);
    }
    return std::make_pair(ret.first, false);
  }

  size_type erase( const Value &x )
  {
    std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
    if (ret.second)
    {
      base_t::erase(begin_nonconst() + (ret.first - begin()));
      return 1;
    }
    return 0;
  }

  void erase( size_t index )
  {
    base_t::erase( begin_nonconst() + index );
  }

  // like C++ 2011: erase with const_iterator (doesn't change sort order)
  void erase(const_iterator const &position)
  {
    // C++98 has vector::erase(iterator), so call that
    base_t::erase(begin_nonconst() + (position - begin()));
  }

  void erase(const_iterator const &first, const_iterator const &last)
  {
    base_t::erase(begin_nonconst() + (first - begin()),
                  begin_nonconst() + (last  - begin()));
  }

  // ACCESSORS

  // Only return a const iterator, so that the vector cannot be directly updated.
  const_iterator begin() const
  {
    return base_t::begin();
  }

  // Only return a const iterator, so that the vector cannot be directly updated.
  const_iterator end() const
  {
    return base_t::end();
  }

  const Value &front() const
  {
    return base_t::front();
  }

  const Value &back() const
  {
    return base_t::back();
  }

  const Value &operator[]( size_t index ) const
  {
    return base_t::operator[]( index );
  }

  // OPERATIONS

  const_iterator lower_bound( const Value &x ) const
  {
    return std::lower_bound( base_t::begin(), base_t::end(), x, Compare() );
  }

  /* Searches the container for an element with a value of x
   * and returns an iterator to it if found, otherwise it returns an
   * iterator to sorted_vector::end (the element past the end of the container).
   *
   * Only return a const iterator, so that the vector cannot be directly updated.
   */
  const_iterator find( const Value &x ) const
  {
    std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
    return (ret.second) ? ret.first : end();
  }

  void insert(sorted_vector<Value,Compare,Find> const &rOther)
  {
    // optimisation for the rather common case that we are overwriting this with the contents
    // of another sorted vector
    if ( empty() )
    {
      base_t::insert(begin_nonconst(), rOther.begin(), rOther.end());
    }
    else
      for( const_iterator it = rOther.begin(); it != rOther.end(); ++it )
        insert( *it );
  }

  /* Clear() elements in the vector, and free them one by one. */
  void DeleteAndDestroyAll()
  {
    for( const_iterator it = begin(); it != end(); ++it )
      delete *it;
    clear();
  }

private:

  typename base_t::iterator begin_nonconst()
  {
    return base_t::begin();
  }
  typename base_t::iterator end_nonconst()
  {
    return base_t::end();
  }

};

/** Implements an ordering function over a pointer, where the comparison uses the < operator on the pointed-to types.
    Very useful for the cases where we put pointers to objects inside a sorted_vector.
*/
template <class T> struct less_ptr_to : public std::binary_function <T *,T *,bool>
{
  bool operator() ( T *const &lhs, T *const &rhs ) const
  {
    return (*lhs) < (*rhs);
  }
};

/** the elements are totally ordered by Compare,
    for no 2 elements !Compare(a,b) && !Compare(b,a) is true
  */
template<class Value, class Compare>
struct find_unique
{
  typedef typename sorted_vector<Value, Compare,
          libvisio::find_unique> ::const_iterator const_iterator;
  std::pair<const_iterator, bool> operator()(
    const_iterator first, const_iterator last,
    Value const &v)
  {
    const_iterator const it = std::lower_bound(first, last, v, Compare());
    return std::make_pair(it, (it != last && !Compare()(v, *it)));
  }
};

/** the elments are partially ordered by Compare,
    2 elements are allowed if they are not the same element (pointer equal)
  */
template<class Value, class Compare>
struct find_partialorder_ptrequals
{
  typedef typename sorted_vector<Value, Compare,
          libvisio::find_partialorder_ptrequals>::const_iterator const_iterator;
  std::pair<const_iterator, bool> operator()(
    const_iterator first, const_iterator last,
    Value const &v)
  {
    std::pair<const_iterator, const_iterator> const its =
      std::equal_range(first, last, v, Compare());
    for (const_iterator it = its.first; it != its.second; ++it)
    {
      if (v == *it)
      {
        return std::make_pair(it, true);
      }
    }
    return std::make_pair(its.first, false);
  }
};

class VSDCollector;

class VSDGeometryListElement
{
public:
  VSDGeometryListElement(unsigned id, unsigned level)
    : m_id(id), m_level(level) {}
  virtual ~VSDGeometryListElement() {}
  virtual void handle(VSDCollector *collector) const = 0;
  virtual VSDGeometryListElement *clone() = 0;
  virtual unsigned getDataID() const
  {
    return (unsigned)-1;
  }
protected:
  unsigned m_id;
  unsigned m_level;
};

class VSDGeometryList
{
public:
  VSDGeometryList();
  VSDGeometryList(const VSDGeometryList &geomList);
  ~VSDGeometryList();
  VSDGeometryList &operator=(const VSDGeometryList &geomList);

  void addGeometry(unsigned id, unsigned level, const boost::optional<bool> &noFill,
                   const boost::optional<bool> &noLine, const boost::optional<bool> &noShow);
  void addEmpty(unsigned id, unsigned level);
  void addMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y);
  void addLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y);
  void addArcTo(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2,
                const boost::optional<double> &bow);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                  const std::vector<std::pair<double, double> > &controlPoints, const std::vector<double> &knotVector,
                  const std::vector<double> &weights);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                     const std::vector<std::pair<double, double> > &points);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void addEllipse(unsigned id, unsigned level, const boost::optional<double> &cx, const boost::optional<double> &cy,
                  const boost::optional<double> &xleft, const boost::optional<double> &yleft,
                  const boost::optional<double> &xtop, const boost::optional<double> &ytop);
  void addEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                          const boost::optional<double> &x2, const boost::optional<double> &y2,
                          const boost::optional<double> &angle, const boost::optional<double> &ecc);
  void addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void addSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void addInfiniteLine(unsigned id, unsigned level, const boost::optional<double> &x1, const boost::optional<double> &y1,
                       const boost::optional<double> &x2, const boost::optional<double> &y2);
  void addRelCubBezTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                      const boost::optional<double> &a, const boost::optional<double> &b,
                      const boost::optional<double> &c, const boost::optional<double> &d);
  void addRelEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                             const boost::optional<double> &x2, const boost::optional<double> &y2,
                             const boost::optional<double> &angle, const boost::optional<double> &ecc);
  void addRelMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y);
  void addRelLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y);
  void addRelQuadBezTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                       const boost::optional<double> &a, const boost::optional<double> &b);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void handle(VSDCollector *collector) const;
  void clear();
  bool empty() const
  {
    return (m_elements.empty());
  }
  VSDGeometryListElement *getElement(unsigned index) const;
  std::vector<unsigned> getElementsOrder() const
  {
    return m_elementsOrder;
  }
  unsigned count() const
  {
    return m_elements.size();
  }
private:
  void clearElement(unsigned id);
  std::map<unsigned, VSDGeometryListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDGEOMETRYLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
