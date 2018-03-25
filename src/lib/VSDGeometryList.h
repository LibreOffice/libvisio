/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDGEOMETRYLIST_H__
#define __VSDGEOMETRYLIST_H__

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <boost/optional.hpp>
#include "VSDTypes.h"

namespace libvisio
{

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
    return MINUS_ONE;
  }
  void setLevel(unsigned level)
  {
    m_level = level;
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
  void addNURBSTo(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2,
                  const boost::optional<double> &knot, const boost::optional<double> &knotPrev, const boost::optional<double> &weight,
                  const boost::optional<double> &weightPrev, const boost::optional<NURBSData> &data);
  void addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                     const std::vector<std::pair<double, double> > &points);
  void addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID);
  void addPolylineTo(unsigned id, unsigned level, boost::optional<double> &x, boost::optional<double> &y, boost::optional<PolylineData> &data);
  void addEllipse(unsigned id, unsigned level, const boost::optional<double> &cx, const boost::optional<double> &cy,
                  const boost::optional<double> &xleft, const boost::optional<double> &yleft,
                  const boost::optional<double> &xtop, const boost::optional<double> &ytop);
  void addEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                          const boost::optional<double> &x2, const boost::optional<double> &y2,
                          const boost::optional<double> &angle, const boost::optional<double> &ecc);
  void addSplineStart(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                      const boost::optional<double> &secondKnot, const boost::optional<double> &firstKnot,
                      const boost::optional<double> &lastKnot, const boost::optional<unsigned> &degree);
  void addSplineKnot(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                     const boost::optional<double> &knot);
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
    return (unsigned)m_elements.size();
  }
  void resetLevel(unsigned level);
private:
  std::map<unsigned, std::unique_ptr<VSDGeometryListElement>> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDGEOMETRYLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
