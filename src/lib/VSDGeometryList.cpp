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

#include "VSDCollector.h"
#include "VSDGeometryList.h"
#include "libvisio_utils.h"

namespace libvisio
{


class VSDGeometry : public VSDGeometryListElement
{
public:
  VSDGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow) :
    m_id(id), m_level(level), m_noFill(noFill), m_noLine(noLine), m_noShow(noShow) {}
  ~VSDGeometry() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id;
  unsigned m_level;
  bool m_noFill;
  bool m_noLine;
  bool m_noShow;
};

class VSDMoveTo : public VSDGeometryListElement
{
public:
  VSDMoveTo(unsigned id, unsigned level, double x, double y) :
    m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDMoveTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
};

class VSDLineTo : public VSDGeometryListElement
{
public:
  VSDLineTo(unsigned id, unsigned level, double x, double y) :
    m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDLineTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
};

class VSDArcTo : public VSDGeometryListElement
{
public:
  VSDArcTo(unsigned id, unsigned level, double x2, double y2, double bow) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_bow(bow) {}
  ~VSDArcTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x2, m_y2, m_bow;
};

class VSDEllipse : public VSDGeometryListElement
{
public:
  VSDEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) :
    m_id(id), m_level(level), m_cx(cx), m_cy(cy), m_xleft(xleft), m_yleft(yleft), m_xtop(xtop), m_ytop(ytop) {}
  ~VSDEllipse() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop;
};

class VSDEllipticalArcTo : public VSDGeometryListElement
{
public:
  VSDEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) :
    m_id(id), m_level(level), m_x3(x3), m_y3(y3), m_x2(x2), m_y2(y2), m_angle(angle), m_ecc(ecc) {}
  ~VSDEllipticalArcTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDNURBSTo1 : public VSDGeometryListElement
{
public:
  VSDNURBSTo1(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_xType(xType), m_yType(yType), m_degree(degree), m_controlPoints(controlPoints), m_knotVector(knotVector), m_weights(weights) {}
  ~VSDNURBSTo1() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x2, m_y2;
  unsigned m_xType, m_yType;
  unsigned m_degree;
  std::vector<std::pair<double, double> > m_controlPoints;
  std::vector<double> m_knotVector, m_weights;
};

class VSDPolylineTo1 : public VSDGeometryListElement
{
public:
  VSDPolylineTo1(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_xType(xType), m_yType(yType), m_points(points) {}
  ~VSDPolylineTo1() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  unsigned m_xType, m_yType;
  std::vector<std::pair<double, double> > m_points;
};

class VSDSplineStart : public VSDGeometryListElement
{
public:
  VSDSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_secondKnot(secondKnot), m_firstKnot(firstKnot), m_lastKnot(lastKnot), m_degree(degree) {}
  ~VSDSplineStart() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  double m_secondKnot, m_firstKnot, m_lastKnot;
  unsigned m_degree;
};

class VSDSplineKnot : public VSDGeometryListElement
{
public:
  VSDSplineKnot(unsigned id, unsigned level, double x, double y, double knot) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_knot(knot) {}
  ~VSDSplineKnot() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  double m_knot;
};

class VSDInfiniteLine : public VSDGeometryListElement
{
public:
  VSDInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) :
    m_id(id), m_level(level), m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}
  ~VSDInfiniteLine() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x1, m_y1, m_x2, m_y2;
};

} // namespace libvisio


void libvisio::VSDGeometry::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectGeometry(m_id, m_level, m_noFill, m_noLine, m_noShow);
}

libvisio::VSDGeometryListElement *libvisio::VSDGeometry::clone()
{
  return new VSDGeometry(m_id, m_level, m_noFill, m_noLine, m_noShow);
}


void libvisio::VSDMoveTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectMoveTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDGeometryListElement *libvisio::VSDMoveTo::clone()
{
  return new VSDMoveTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDLineTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectLineTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDGeometryListElement *libvisio::VSDLineTo::clone()
{
  return new VSDLineTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDArcTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectArcTo(m_id, m_level, m_x2, m_y2, m_bow);
}

libvisio::VSDGeometryListElement *libvisio::VSDArcTo::clone()
{
  return new VSDArcTo(m_id, m_level, m_x2, m_y2, m_bow);
}


void libvisio::VSDEllipse::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectEllipse(m_id, m_level, m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop);
}

libvisio::VSDGeometryListElement *libvisio::VSDEllipse::clone()
{
  return new VSDEllipse(m_id, m_level, m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop);
}


void libvisio::VSDEllipticalArcTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}

libvisio::VSDGeometryListElement *libvisio::VSDEllipticalArcTo::clone()
{
  return new VSDEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}


void libvisio::VSDNURBSTo1::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_xType, m_yType, m_degree, m_controlPoints, m_knotVector, m_weights);
}

libvisio::VSDGeometryListElement *libvisio::VSDNURBSTo1::clone()
{
  return new VSDNURBSTo1(m_id, m_level, m_x2, m_y2, m_xType, m_yType, m_degree, m_controlPoints, m_knotVector, m_weights);
}


void libvisio::VSDNURBSTo2::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_dataID);
}

libvisio::VSDGeometryListElement *libvisio::VSDNURBSTo2::clone()
{
  return new VSDNURBSTo2(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_dataID);
}


void libvisio::VSDPolylineTo1::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_xType, m_yType, m_points);
}

libvisio::VSDGeometryListElement *libvisio::VSDPolylineTo1::clone()
{
  return new VSDPolylineTo1(m_id, m_level, m_x, m_y, m_xType, m_yType, m_points);
}


void libvisio::VSDPolylineTo2::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_dataID);
}

libvisio::VSDGeometryListElement *libvisio::VSDPolylineTo2::clone()
{
  return new VSDPolylineTo2(m_id, m_level, m_x, m_y, m_dataID);
}


void libvisio::VSDSplineStart::handle(VSDCollector *collector) const
{
  collector->collectSplineStart(m_id, m_level, m_x, m_y, m_secondKnot, m_firstKnot, m_lastKnot, m_degree);
}

libvisio::VSDGeometryListElement *libvisio::VSDSplineStart::clone()
{
  return new VSDSplineStart(m_id, m_level, m_x, m_y, m_secondKnot, m_firstKnot, m_lastKnot, m_degree);
}


void libvisio::VSDSplineKnot::handle(VSDCollector *collector) const
{
  collector->collectSplineKnot(m_id, m_level, m_x, m_y, m_knot);
}

libvisio::VSDGeometryListElement *libvisio::VSDSplineKnot::clone()
{
  return new VSDSplineKnot(m_id, m_level, m_x, m_y, m_knot);
}


void libvisio::VSDInfiniteLine::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectInfiniteLine(m_id, m_level, m_x1, m_y1, m_x2, m_y2);
}

libvisio::VSDGeometryListElement *libvisio::VSDInfiniteLine::clone()
{
  return new VSDInfiniteLine(m_id, m_level, m_x1, m_y1, m_x2, m_y2);
}


libvisio::VSDGeometryList::VSDGeometryList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDGeometryList::VSDGeometryList(const VSDGeometryList &geomList) :
  m_elements(),
  m_elementsOrder(geomList.m_elementsOrder)
{
  std::map<unsigned, VSDGeometryListElement *>::const_iterator iter = geomList.m_elements.begin();
  for (; iter != geomList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDGeometryList &libvisio::VSDGeometryList::operator=(const VSDGeometryList &geomList)
{
  clear();
  std::map<unsigned, VSDGeometryListElement *>::const_iterator iter = geomList.m_elements.begin();
  for (; iter != geomList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = geomList.m_elementsOrder;
  return *this;
}

libvisio::VSDGeometryList::~VSDGeometryList()
{
  clear();
}

void libvisio::VSDGeometryList::addGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow)
{
  m_elements[id] = new VSDGeometry(id, level, noFill, noLine, noShow);
}

void libvisio::VSDGeometryList::addMoveTo(unsigned id, unsigned level, double x, double y)
{
  m_elements[id] = new VSDMoveTo(id, level, x, y);
}

void libvisio::VSDGeometryList::addLineTo(unsigned id, unsigned level, double x, double y)
{
  m_elements[id] = new VSDLineTo(id, level, x, y);
}

void libvisio::VSDGeometryList::addArcTo(unsigned id, unsigned level, double x2, double y2, double bow)
{
  m_elements[id] = new VSDArcTo(id, level, x2, y2, bow);
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  m_elements[id] = new VSDNURBSTo1(id, level, x2, y2, xType, yType, degree, controlPoints, knotVector, weights);
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  m_elements[id] = new VSDNURBSTo2(id, level, x2, y2, knot, knotPrev, weight, weightPrev, dataID);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points)
{
  m_elements[id] = new VSDPolylineTo1(id, level, x, y, xType, yType, points);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID)
{
  m_elements[id] = new VSDPolylineTo2(id, level, x, y, dataID);
}

void libvisio::VSDGeometryList::addEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop)
{
  m_elements[id] = new VSDEllipse(id, level, cx, cy, xleft, yleft, xtop, ytop);
}

void libvisio::VSDGeometryList::addEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  m_elements[id] = new VSDEllipticalArcTo(id, level, x3, y3, x2, y2, angle, ecc);
}

void libvisio::VSDGeometryList::addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree)
{
  m_elements[id] = new VSDSplineStart(id, level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDGeometryList::addSplineKnot(unsigned id, unsigned level, double x, double y, double knot)
{
  m_elements[id] = new VSDSplineKnot(id, level, x, y, knot);
}

void libvisio::VSDGeometryList::addInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2)
{
  m_elements[id] = new VSDInfiniteLine(id, level, x1, y1, x2, y2);
}

void libvisio::VSDGeometryList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDGeometryList::handle(VSDCollector *collector) const
{
  if (empty())
    return;
  std::map<unsigned, VSDGeometryListElement *>::const_iterator iter;
  if (!m_elementsOrder.empty())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      iter->second->handle(collector);
  }
  collector->collectSplineEnd();
}

void libvisio::VSDGeometryList::clear()
{
  for (std::map<unsigned, VSDGeometryListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDGeometryListElement *libvisio::VSDGeometryList::getElement(unsigned index) const
{
  if (m_elementsOrder.size() > index)
    index = m_elementsOrder[index];

  std::map<unsigned, VSDGeometryListElement *>::const_iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
