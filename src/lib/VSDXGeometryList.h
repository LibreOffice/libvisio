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

#ifndef __VSDXGEOMETRYLIST_H__
#define __VSDXGEOMETRYLIST_H__

#include <vector>
#include <map>

namespace libvisio
{

class VSDXGeometryListElement;
class VSDXCollector;

class VSDXGeometryList
{
public:
  VSDXGeometryList();
  VSDXGeometryList(const VSDXGeometryList &geomList);
  ~VSDXGeometryList();
  VSDXGeometryList &operator=(const VSDXGeometryList &geomList);

  void addGeometry(unsigned id, unsigned level, unsigned char geomFlags);
  void addMoveTo(unsigned id, unsigned level, double x, double y);
  void addLineTo(unsigned id, unsigned level, double x, double y);
  void addArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points);
  void addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void addEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void addEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void addSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void handle(VSDXCollector *collector) const;
  void clear();
  bool empty() const
  {
    return (!m_elements.size());
  }
  VSDXGeometryListElement *getElement(unsigned index) const;
  std::vector<unsigned> getElementsOrder() const
  {
    return m_elementsOrder;
  }
  unsigned count() const
  {
    return m_elements.size();
  }
private:
  std::map<unsigned, VSDXGeometryListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

class VSDXGeometryListElement
{
public:
  VSDXGeometryListElement() {}
  virtual ~VSDXGeometryListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
  virtual VSDXGeometryListElement *clone() = 0;
};

class VSDXPolylineTo2 : public VSDXGeometryListElement
{
public:
  VSDXPolylineTo2(unsigned id , unsigned level, double x, double y, unsigned dataID) :
    m_dataID(dataID), m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDXPolylineTo2() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
  unsigned m_dataID;
private:
  unsigned m_id, m_level;
  double m_x, m_y;
};

class VSDXNURBSTo2 : public VSDXGeometryListElement
{
public:
  VSDXNURBSTo2(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) :
    m_dataID(dataID), m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_knot(knot), m_knotPrev(knotPrev), m_weight(weight), m_weightPrev(weightPrev) {}
  ~VSDXNURBSTo2() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
  unsigned m_dataID;
private:
  unsigned m_id, m_level;
  double m_x2, m_y2;
  double m_knot, m_knotPrev;
  double m_weight, m_weightPrev;

};

} // namespace libvisio

#endif // __VSDXGEOMETRYLIST_H__
