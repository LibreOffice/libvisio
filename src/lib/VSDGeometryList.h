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
#include <optional>
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

  void addGeometry(unsigned id, unsigned level, const std::optional<bool> &noFill,
                   const std::optional<bool> &noLine, const std::optional<bool> &noShow);
  void addEmpty(unsigned id, unsigned level);
  void addMoveTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y);
  void addLineTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y);
  void addArcTo(unsigned id, unsigned level, const std::optional<double> &x2, const std::optional<double> &y2,
                const std::optional<double> &bow);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                  const std::vector<std::pair<double, double> > &controlPoints, const std::vector<double> &knotVector,
                  const std::vector<double> &weights);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void addNURBSTo(unsigned id, unsigned level, const std::optional<double> &x2, const std::optional<double> &y2,
                  const std::optional<double> &knot, const std::optional<double> &knotPrev, const std::optional<double> &weight,
                  const std::optional<double> &weightPrev, const std::optional<NURBSData> &data);
  void addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                     const std::vector<std::pair<double, double> > &points);
  void addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID);
  void addPolylineTo(unsigned id, unsigned level, std::optional<double> &x, std::optional<double> &y, std::optional<PolylineData> &data);
  void addEllipse(unsigned id, unsigned level, const std::optional<double> &cx, const std::optional<double> &cy,
                  const std::optional<double> &xleft, const std::optional<double> &yleft,
                  const std::optional<double> &xtop, const std::optional<double> &ytop);
  void addEllipticalArcTo(unsigned id, unsigned level, const std::optional<double> &x3, const std::optional<double> &y3,
                          const std::optional<double> &x2, const std::optional<double> &y2,
                          const std::optional<double> &angle, const std::optional<double> &ecc);
  void addSplineStart(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y,
                      const std::optional<double> &secondKnot, const std::optional<double> &firstKnot,
                      const std::optional<double> &lastKnot, const std::optional<unsigned> &degree);
  void addSplineKnot(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y,
                     const std::optional<double> &knot);
  void addInfiniteLine(unsigned id, unsigned level, const std::optional<double> &x1, const std::optional<double> &y1,
                       const std::optional<double> &x2, const std::optional<double> &y2);
  void addRelCubBezTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y,
                      const std::optional<double> &a, const std::optional<double> &b,
                      const std::optional<double> &c, const std::optional<double> &d);
  void addRelEllipticalArcTo(unsigned id, unsigned level, const std::optional<double> &x3, const std::optional<double> &y3,
                             const std::optional<double> &x2, const std::optional<double> &y2,
                             const std::optional<double> &angle, const std::optional<double> &ecc);
  void addRelMoveTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y);
  void addRelLineTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y);
  void addRelQuadBezTo(unsigned id, unsigned level, const std::optional<double> &x, const std::optional<double> &y,
                       const std::optional<double> &a, const std::optional<double> &b);
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
