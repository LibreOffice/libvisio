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

namespace libvisio
{

class VSDGeometryListElement;
class VSDCollector;

class VSDGeometryList
{
public:
  VSDGeometryList();
  VSDGeometryList(const VSDGeometryList &geomList);
  ~VSDGeometryList();
  VSDGeometryList &operator=(const VSDGeometryList &geomList);

  void addGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow);
  void addMoveTo(unsigned id, unsigned level, double x, double y);
  void addLineTo(unsigned id, unsigned level, double x, double y);
  void addArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                  const std::vector<std::pair<double, double> > &controlPoints, const std::vector<double> &knotVector,
                  const std::vector<double> &weights);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                     const std::vector<std::pair<double, double> > &points);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void addEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void addEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void addSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void addInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2);
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
  std::map<unsigned, VSDGeometryListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

class VSDGeometryListElement
{
public:
  VSDGeometryListElement() {}
  virtual ~VSDGeometryListElement() {}
  virtual void handle(VSDCollector *collector) const = 0;
  virtual VSDGeometryListElement *clone() = 0;
  virtual const unsigned *getNURBSDataID() const
  {
    return 0;
  }
  virtual const unsigned *getPolylineDataID() const
  {
    return 0;
  }
};

} // namespace libvisio

#endif // __VSDGEOMETRYLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
