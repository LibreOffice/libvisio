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

#include "VSDXCollector.h"
#include "VSDXGeometryList.h"
#include "libvisio_utils.h"

namespace libvisio
{


class VSDXGeometry : public VSDXGeometryListElement
{
public:
  VSDXGeometry(unsigned id, unsigned level, unsigned geomFlags) :
    m_id(id), m_level(level), m_geomFlags(geomFlags) {}
  ~VSDXGeometry() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id;
  unsigned m_level;
  unsigned char m_geomFlags;
};

class VSDXMoveTo : public VSDXGeometryListElement
{
public:
  VSDXMoveTo(unsigned id, unsigned level, double x, double y) :
    m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDXMoveTo() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
};

class VSDXLineTo : public VSDXGeometryListElement
{
public:
  VSDXLineTo(unsigned id, unsigned level, double x, double y) :
    m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDXLineTo() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
};

class VSDXArcTo : public VSDXGeometryListElement
{
public:
  VSDXArcTo(unsigned id, unsigned level, double x2, double y2, double bow) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_bow(bow) {}
  ~VSDXArcTo() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x2, m_y2, m_bow;
};

class VSDXEllipse : public VSDXGeometryListElement
{
public:
  VSDXEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) :
    m_id(id), m_level(level), m_cx(cx), m_cy(cy), m_xleft(xleft), m_yleft(yleft), m_xtop(xtop), m_ytop(ytop) {}
  ~VSDXEllipse() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop;
};

class VSDXEllipticalArcTo : public VSDXGeometryListElement
{
public:
  VSDXEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) :
    m_id(id), m_level(level), m_x3(x3), m_y3(y3), m_x2(x2), m_y2(y2), m_angle(angle), m_ecc(ecc) {}
  ~VSDXEllipticalArcTo() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDXNURBSTo1 : public VSDXGeometryListElement
{
public:
  VSDXNURBSTo1(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_xType(xType), m_yType(yType), m_degree(degree), m_controlPoints(controlPoints), m_knotVector(knotVector), m_weights(weights) {}
  ~VSDXNURBSTo1() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x2, m_y2;
  unsigned m_xType, m_yType;
  unsigned m_degree;
  std::vector<std::pair<double, double> > m_controlPoints;
  std::vector<double> m_knotVector, m_weights;
};

class VSDXPolylineTo1 : public VSDXGeometryListElement
{
public:
  VSDXPolylineTo1(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_xType(xType), m_yType(yType), m_points(points) {}
  ~VSDXPolylineTo1() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  unsigned m_xType, m_yType;
  std::vector<std::pair<double, double> > m_points;
};

class VSDXSplineStart : public VSDXGeometryListElement
{
public:
  VSDXSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_secondKnot(secondKnot), m_firstKnot(firstKnot), m_lastKnot(lastKnot), m_degree(degree) {}
  ~VSDXSplineStart() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  double m_secondKnot, m_firstKnot, m_lastKnot;
  unsigned m_degree;
};

class VSDXSplineKnot : public VSDXGeometryListElement
{
public:
  VSDXSplineKnot(unsigned id, unsigned level, double x, double y, double knot) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_knot(knot) {}
  ~VSDXSplineKnot() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  double m_knot;
};

class VSDXInfiniteLine : public VSDXGeometryListElement
{
public:
  VSDXInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) :
    m_id(id), m_level(level), m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}
  ~VSDXInfiniteLine() {}
  void handle(VSDXCollector *collector);
  VSDXGeometryListElement *clone();
private:
  unsigned m_id, m_level;
  double m_x1, m_y1, m_x2, m_y2;
};

} // namespace libvisio


void libvisio::VSDXGeometry::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectGeometry(m_id, m_level, m_geomFlags);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXGeometry::clone()
{
  return new VSDXGeometry(m_id, m_level, m_geomFlags);
}


void libvisio::VSDXMoveTo::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectMoveTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXMoveTo::clone()
{
  return new VSDXMoveTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDXLineTo::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectLineTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXLineTo::clone()
{
  return new VSDXLineTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDXArcTo::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectArcTo(m_id, m_level, m_x2, m_y2, m_bow);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXArcTo::clone()
{
  return new VSDXArcTo(m_id, m_level, m_x2, m_y2, m_bow);
}


void libvisio::VSDXEllipse::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectEllipse(m_id, m_level, m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXEllipse::clone()
{
  return new VSDXEllipse(m_id, m_level, m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop);
}


void libvisio::VSDXEllipticalArcTo::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXEllipticalArcTo::clone()
{
  return new VSDXEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}


void libvisio::VSDXNURBSTo1::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_xType, m_yType, m_degree, m_controlPoints, m_knotVector, m_weights);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXNURBSTo1::clone()
{
  return new VSDXNURBSTo1(m_id, m_level, m_x2, m_y2, m_xType, m_yType, m_degree, m_controlPoints, m_knotVector, m_weights);
}


void libvisio::VSDXNURBSTo2::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_dataID);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXNURBSTo2::clone()
{
  return new VSDXNURBSTo2(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_dataID);
}


void libvisio::VSDXPolylineTo1::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_xType, m_yType, m_points);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXPolylineTo1::clone()
{
  return new VSDXPolylineTo1(m_id, m_level, m_x, m_y, m_xType, m_yType, m_points);
}


void libvisio::VSDXPolylineTo2::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_dataID);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXPolylineTo2::clone()
{
  return new VSDXPolylineTo2(m_id, m_level, m_x, m_y, m_dataID);
}


void libvisio::VSDXSplineStart::handle(VSDXCollector *collector)
{
  collector->collectSplineStart(m_id, m_level, m_x, m_y, m_secondKnot, m_firstKnot, m_lastKnot, m_degree);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXSplineStart::clone()
{
  return new VSDXSplineStart(m_id, m_level, m_x, m_y, m_secondKnot, m_firstKnot, m_lastKnot, m_degree);
}


void libvisio::VSDXSplineKnot::handle(VSDXCollector *collector)
{
  collector->collectSplineKnot(m_id, m_level, m_x, m_y, m_knot);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXSplineKnot::clone()
{
  return new VSDXSplineKnot(m_id, m_level, m_x, m_y, m_knot);
}


void libvisio::VSDXInfiniteLine::handle(VSDXCollector *collector)
{
  collector->collectSplineEnd();
  collector->collectInfiniteLine(m_id, m_level, m_x1, m_y1, m_x2, m_y2);
}

libvisio::VSDXGeometryListElement *libvisio::VSDXInfiniteLine::clone()
{
  return new VSDXInfiniteLine(m_id, m_level, m_x1, m_y1, m_x2, m_y2);
}


libvisio::VSDXGeometryList::VSDXGeometryList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDXGeometryList::VSDXGeometryList(const VSDXGeometryList &geomList) :
  m_elements(),
  m_elementsOrder(geomList.m_elementsOrder)
{
  std::map<unsigned, VSDXGeometryListElement *>::const_iterator iter = geomList.m_elements.begin();
  for (; iter != geomList.m_elements.end(); iter++)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDXGeometryList &libvisio::VSDXGeometryList::operator=(const VSDXGeometryList &geomList)
{
  clear();
  std::map<unsigned, VSDXGeometryListElement *>::const_iterator iter = geomList.m_elements.begin();
  for (; iter != geomList.m_elements.end(); iter++)
    m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = geomList.m_elementsOrder;
  return *this;
}

libvisio::VSDXGeometryList::~VSDXGeometryList()
{
  clear();
}

void libvisio::VSDXGeometryList::addGeometry(unsigned id, unsigned level, unsigned char geomFlags)
{
  m_elements[id] = new VSDXGeometry(id, level, geomFlags);
}

void libvisio::VSDXGeometryList::addMoveTo(unsigned id, unsigned level, double x, double y)
{
  m_elements[id] = new VSDXMoveTo(id, level, x, y);
}

void libvisio::VSDXGeometryList::addLineTo(unsigned id, unsigned level, double x, double y)
{
  m_elements[id] = new VSDXLineTo(id, level, x, y);
}

void libvisio::VSDXGeometryList::addArcTo(unsigned id, unsigned level, double x2, double y2, double bow)
{
  m_elements[id] = new VSDXArcTo(id, level, x2, y2, bow);
}

void libvisio::VSDXGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  m_elements[id] = new VSDXNURBSTo1(id, level, x2, y2, xType, yType, degree, controlPoints, knotVector, weights);
}

void libvisio::VSDXGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  m_elements[id] = new VSDXNURBSTo2(id, level, x2, y2, knot, knotPrev, weight, weightPrev, dataID);
}

void libvisio::VSDXGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points)
{
  m_elements[id] = new VSDXPolylineTo1(id, level, x, y, xType, yType, points);
}

void libvisio::VSDXGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID)
{
  m_elements[id] = new VSDXPolylineTo2(id, level, x, y, dataID);
}

void libvisio::VSDXGeometryList::addEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop)
{
  m_elements[id] = new VSDXEllipse(id, level, cx, cy, xleft, yleft, xtop, ytop);
}

void libvisio::VSDXGeometryList::addEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  m_elements[id] = new VSDXEllipticalArcTo(id, level, x3, y3, x2, y2, angle, ecc);
}

void libvisio::VSDXGeometryList::addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree)
{
  m_elements[id] = new VSDXSplineStart(id, level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDXGeometryList::addSplineKnot(unsigned id, unsigned level, double x, double y, double knot)
{
  m_elements[id] = new VSDXSplineKnot(id, level, x, y, knot);
}

void libvisio::VSDXGeometryList::addInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2)
{
  m_elements[id] = new VSDXInfiniteLine(id, level, x1, y1, x2, y2);
}

void libvisio::VSDXGeometryList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXGeometryList::handle(VSDXCollector *collector) const
{
  if (empty())
    return;
  std::map<unsigned, VSDXGeometryListElement *>::const_iterator iter;
  if (m_elementsOrder.size())
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
    for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      iter->second->handle(collector);
  }
  collector->collectSplineEnd();
}

void libvisio::VSDXGeometryList::clear()
{
  for (std::map<unsigned, VSDXGeometryListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDXGeometryListElement *libvisio::VSDXGeometryList::getElement(unsigned index) const
{
  if (m_elementsOrder.size() > index)
    index = m_elementsOrder[index];

  std::map<unsigned, VSDXGeometryListElement *>::const_iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
