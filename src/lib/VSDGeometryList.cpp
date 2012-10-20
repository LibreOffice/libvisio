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

#define FROM_OPTIONAL(t, u) t.is_initialized() ? t.get() : u
#define ASSIGN_OPTIONAL(t, u) if(t.is_initialized()) u = t.get()

namespace libvisio
{


class VSDGeometry : public VSDGeometryListElement
{
public:
  VSDGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow) :
    VSDGeometryListElement(id, level), m_noFill(noFill), m_noLine(noLine), m_noShow(noShow) {}
  virtual ~VSDGeometry() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  bool m_noFill;
  bool m_noLine;
  bool m_noShow;
};

class VSDEmpty : public VSDGeometryListElement
{
public:
  VSDEmpty(unsigned id, unsigned level) :
    VSDGeometryListElement(id, level) {}
  virtual ~VSDEmpty() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
};

class VSDMoveTo : public VSDGeometryListElement
{
public:
  VSDMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(FROM_OPTIONAL(x, 0.0)), m_y(FROM_OPTIONAL(y, 0.0)) {}
  virtual ~VSDMoveTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  double m_x, m_y;
};

class VSDLineTo : public VSDGeometryListElement
{
public:
  VSDLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(FROM_OPTIONAL(x, 0.0)), m_y(FROM_OPTIONAL(y, 0.0)) {}
  virtual ~VSDLineTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  double m_x, m_y;
};

class VSDArcTo : public VSDGeometryListElement
{
public:
  VSDArcTo(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2, const boost::optional<double> &bow) :
    VSDGeometryListElement(id, level), m_x2(FROM_OPTIONAL(x2, 0.0)), m_y2(FROM_OPTIONAL(y2, 0.0)), m_bow(FROM_OPTIONAL(bow, 0.0)) {}
  virtual ~VSDArcTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  double m_x2, m_y2, m_bow;
};

class VSDEllipse : public VSDGeometryListElement
{
public:
  VSDEllipse(unsigned id, unsigned level, const boost::optional<double> &cx, const boost::optional<double> &cy,
             const boost::optional<double> &xleft, const boost::optional<double> &yleft,
             const boost::optional<double> &xtop, const boost::optional<double> &ytop) :
    VSDGeometryListElement(id, level), m_cx(FROM_OPTIONAL(cx, 0.0)), m_cy(FROM_OPTIONAL(cy, 0.0)),
    m_xleft(FROM_OPTIONAL(xleft, 0.0)), m_yleft(FROM_OPTIONAL(yleft, 0.0)), m_xtop(FROM_OPTIONAL(xtop, 0.0)),
    m_ytop(FROM_OPTIONAL(ytop, 0.0)) {}
  virtual ~VSDEllipse() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  double m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop;
};

class VSDEllipticalArcTo : public VSDGeometryListElement
{
public:
  VSDEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                     const boost::optional<double> &x2, const boost::optional<double> &y2,
                     const boost::optional<double> &angle, const boost::optional<double> &ecc) :
    VSDGeometryListElement(id, level), m_x3(FROM_OPTIONAL(x3, 0.0)), m_y3(FROM_OPTIONAL(y3, 0.0)), m_x2(FROM_OPTIONAL(x2, 0.0)),
    m_y2(FROM_OPTIONAL(y2, 0.0)), m_angle(FROM_OPTIONAL(angle, 0.0)), m_ecc(FROM_OPTIONAL(ecc, 0.0)) {}
  virtual ~VSDEllipticalArcTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDNURBSTo1 : public VSDGeometryListElement
{
public:
  VSDNURBSTo1(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) :
    VSDGeometryListElement(id, level), m_x2(x2), m_y2(y2), m_xType(xType), m_yType(yType), m_degree(degree), m_controlPoints(controlPoints), m_knotVector(knotVector), m_weights(weights) {}
  virtual ~VSDNURBSTo1() {}
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

class VSDNURBSTo2 : public VSDGeometryListElement
{
public:
  VSDNURBSTo2(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) :
    VSDGeometryListElement(id, level), m_dataID(dataID), m_x2(x2), m_y2(y2), m_knot(knot), m_knotPrev(knotPrev), m_weight(weight), m_weightPrev(weightPrev) {}
  virtual ~VSDNURBSTo2() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  unsigned getDataID() const;

private:
  unsigned m_dataID;
  double m_x2, m_y2;
  double m_knot, m_knotPrev;
  double m_weight, m_weightPrev;

};

class VSDPolylineTo1 : public VSDGeometryListElement
{
public:
  VSDPolylineTo1(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_xType(xType), m_yType(yType), m_points(points) {}
  virtual ~VSDPolylineTo1() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y;
  unsigned m_xType, m_yType;
  std::vector<std::pair<double, double> > m_points;
};

class VSDPolylineTo2 : public VSDGeometryListElement
{
public:
  VSDPolylineTo2(unsigned id , unsigned level, double x, double y, unsigned dataID) :
    VSDGeometryListElement(id, level), m_dataID(dataID), m_x(x), m_y(y) {}
  virtual ~VSDPolylineTo2() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
  unsigned getDataID() const;

private:
  unsigned m_dataID;
  double m_x, m_y;
};

class VSDSplineStart : public VSDGeometryListElement
{
public:
  VSDSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_secondKnot(secondKnot), m_firstKnot(firstKnot), m_lastKnot(lastKnot), m_degree(degree) {}
  virtual ~VSDSplineStart() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y;
  double m_secondKnot, m_firstKnot, m_lastKnot;
  unsigned m_degree;
};

class VSDSplineKnot : public VSDGeometryListElement
{
public:
  VSDSplineKnot(unsigned id, unsigned level, double x, double y, double knot) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_knot(knot) {}
  virtual ~VSDSplineKnot() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y;
  double m_knot;
};

class VSDInfiniteLine : public VSDGeometryListElement
{
public:
  VSDInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) :
    VSDGeometryListElement(id, level), m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}
  virtual ~VSDInfiniteLine() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x1, m_y1, m_x2, m_y2;
};

class VSDRelCubBezTo : public VSDGeometryListElement
{
public:
  VSDRelCubBezTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_a(a), m_b(b), m_c(c), m_d(d) {}
  virtual ~VSDRelCubBezTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y, m_a, m_b, m_c, m_d;
};

class VSDRelEllipticalArcTo : public VSDGeometryListElement
{
public:
  VSDRelEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) :
    VSDGeometryListElement(id, level), m_x3(x3), m_y3(y3), m_x2(x2), m_y2(y2), m_angle(angle), m_ecc(ecc) {}
  virtual ~VSDRelEllipticalArcTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDRelMoveTo : public VSDGeometryListElement
{
public:
  VSDRelMoveTo(unsigned id, unsigned level, double x, double y) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y) {}
  virtual ~VSDRelMoveTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y;
};

class VSDRelLineTo : public VSDGeometryListElement
{
public:
  VSDRelLineTo(unsigned id, unsigned level, double x, double y) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y) {}
  virtual ~VSDRelLineTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y;
};

class VSDRelQuadBezTo : public VSDGeometryListElement
{
public:
  VSDRelQuadBezTo(unsigned id, unsigned level, double x, double y, double a, double b) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_a(a), m_b(b) {}
  virtual ~VSDRelQuadBezTo() {}
  void handle(VSDCollector *collector) const;
  VSDGeometryListElement *clone();
private:
  double m_x, m_y, m_a, m_b;
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


void libvisio::VSDEmpty::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
}

libvisio::VSDGeometryListElement *libvisio::VSDEmpty::clone()
{
  return new VSDEmpty(m_id, m_level);
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

unsigned libvisio::VSDNURBSTo2::getDataID() const
{
  return m_dataID;
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

unsigned libvisio::VSDPolylineTo2::getDataID() const
{
  return m_dataID;
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


void libvisio::VSDRelCubBezTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectRelCubBezTo(m_id, m_level, m_x, m_y, m_a, m_b, m_c, m_d);
}

libvisio::VSDGeometryListElement *libvisio::VSDRelCubBezTo::clone()
{
  return new VSDRelCubBezTo(m_id, m_level, m_x, m_y, m_a, m_b, m_c, m_d);
}


void libvisio::VSDRelEllipticalArcTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectRelEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}

libvisio::VSDGeometryListElement *libvisio::VSDRelEllipticalArcTo::clone()
{
  return new VSDRelEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}


void libvisio::VSDRelMoveTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectRelMoveTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDGeometryListElement *libvisio::VSDRelMoveTo::clone()
{
  return new VSDRelMoveTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDRelLineTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectRelLineTo(m_id, m_level, m_x, m_y);
}

libvisio::VSDGeometryListElement *libvisio::VSDRelLineTo::clone()
{
  return new VSDRelLineTo(m_id, m_level, m_x, m_y);
}


void libvisio::VSDRelQuadBezTo::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectRelQuadBezTo(m_id, m_level, m_x, m_y, m_a, m_b);
}

libvisio::VSDGeometryListElement *libvisio::VSDRelQuadBezTo::clone()
{
  return new VSDRelQuadBezTo(m_id, m_level, m_x, m_y, m_a, m_b);
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
  clearElement(id);
  m_elements[id] = new VSDGeometry(id, level, noFill, noLine, noShow);
}

void libvisio::VSDGeometryList::addEmpty(unsigned id, unsigned level)
{
  clearElement(id);
  m_elements[id] = new VSDEmpty(id, level);
}

void libvisio::VSDGeometryList::addMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y)
{
  VSDMoveTo *tmpElement = dynamic_cast<VSDMoveTo *>(m_elements[id]);
  if (!tmpElement)
  {
    clearElement(id);
    m_elements[id] = new VSDMoveTo(id, level, x, y);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
  }
}

void libvisio::VSDGeometryList::addLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y)
{
  VSDLineTo *tmpElement = dynamic_cast<VSDLineTo *>(m_elements[id]);
  if (!tmpElement)
  {
    clearElement(id);
    m_elements[id] = new VSDLineTo(id, level, x, y);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
  }
}

void libvisio::VSDGeometryList::addArcTo(unsigned id, unsigned level, const boost::optional<double> &x2,
    const boost::optional<double> &y2, const boost::optional<double> &bow)
{
  VSDArcTo *tmpElement = dynamic_cast<VSDArcTo *>(m_elements[id]);
  if (!tmpElement)
  {
    clearElement(id);
    m_elements[id] = new VSDArcTo(id, level, x2, y2, bow);
  }
  else
  {
    ASSIGN_OPTIONAL(x2, tmpElement->m_x2);
    ASSIGN_OPTIONAL(y2, tmpElement->m_y2);
    ASSIGN_OPTIONAL(bow, tmpElement->m_bow);
  }
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
    const std::vector<std::pair<double, double> > &controlPoints, const std::vector<double> &knotVector, const std::vector<double> &weights)
{
  clearElement(id);
  m_elements[id] = new VSDNURBSTo1(id, level, x2, y2, xType, yType, degree, controlPoints, knotVector, weights);
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  clearElement(id);
  m_elements[id] = new VSDNURBSTo2(id, level, x2, y2, knot, knotPrev, weight, weightPrev, dataID);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType,
    const std::vector<std::pair<double, double> > &points)
{
  clearElement(id);
  m_elements[id] = new VSDPolylineTo1(id, level, x, y, xType, yType, points);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID)
{
  clearElement(id);
  m_elements[id] = new VSDPolylineTo2(id, level, x, y, dataID);
}

void libvisio::VSDGeometryList::addEllipse(unsigned id, unsigned level, const boost::optional<double> &cx,
    const boost::optional<double> &cy,const boost::optional<double> &xleft, const boost::optional<double> &yleft,
    const boost::optional<double> &xtop, const boost::optional<double> &ytop)
{
  VSDEllipse *tmpElement = dynamic_cast<VSDEllipse *>(m_elements[id]);
  if (!tmpElement)
  {
    clearElement(id);
    m_elements[id] = new VSDEllipse(id, level, cx, cy, xleft, yleft, xtop, ytop);
  }
  else
  {
    ASSIGN_OPTIONAL(cx, tmpElement->m_cx);
    ASSIGN_OPTIONAL(cy, tmpElement->m_cy);
    ASSIGN_OPTIONAL(xleft, tmpElement->m_xleft);
    ASSIGN_OPTIONAL(yleft, tmpElement->m_yleft);
    ASSIGN_OPTIONAL(xtop, tmpElement->m_xtop);
    ASSIGN_OPTIONAL(ytop, tmpElement->m_ytop);
  }
}

void libvisio::VSDGeometryList::addEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3,
    const boost::optional<double> &y3, const boost::optional<double> &x2, const boost::optional<double> &y2,
    const boost::optional<double> &angle, const boost::optional<double> &ecc)
{
  VSDEllipticalArcTo *tmpElement = dynamic_cast<VSDEllipticalArcTo *>(m_elements[id]);
  if (!tmpElement)
  {
    clearElement(id);
    m_elements[id] = new VSDEllipticalArcTo(id, level, x3, y3, x2, y2, angle, ecc);
  }
  else
  {
    ASSIGN_OPTIONAL(x3, tmpElement->m_x3);
    ASSIGN_OPTIONAL(y3, tmpElement->m_y3);
    ASSIGN_OPTIONAL(x2, tmpElement->m_x2);
    ASSIGN_OPTIONAL(y2, tmpElement->m_y2);
    ASSIGN_OPTIONAL(angle, tmpElement->m_angle);
    ASSIGN_OPTIONAL(ecc, tmpElement->m_ecc);
  }
}

void libvisio::VSDGeometryList::addSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree)
{
  clearElement(id);
  m_elements[id] = new VSDSplineStart(id, level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDGeometryList::addSplineKnot(unsigned id, unsigned level, double x, double y, double knot)
{
  clearElement(id);
  m_elements[id] = new VSDSplineKnot(id, level, x, y, knot);
}

void libvisio::VSDGeometryList::addInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2)
{
  clearElement(id);
  m_elements[id] = new VSDInfiniteLine(id, level, x1, y1, x2, y2);
}

void libvisio::VSDGeometryList::addRelCubBezTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d)
{
  clearElement(id);
  m_elements[id] = new VSDRelCubBezTo(id, level, x, y, a, b, c, d);
}

void libvisio::VSDGeometryList::addRelEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  clearElement(id);
  m_elements[id] = new VSDRelEllipticalArcTo(id, level, x3, y3, x2, y2, angle, ecc);
}

void libvisio::VSDGeometryList::addRelMoveTo(unsigned id, unsigned level, double x, double y)
{
  clearElement(id);
  m_elements[id] = new VSDRelMoveTo(id, level, x, y);
}

void libvisio::VSDGeometryList::addRelLineTo(unsigned id, unsigned level, double x, double y)
{
  clearElement(id);
  m_elements[id] = new VSDRelLineTo(id, level, x, y);
}

void libvisio::VSDGeometryList::addRelQuadBezTo(unsigned id, unsigned level, double x, double y, double a, double b)
{
  clearElement(id);
  m_elements[id] = new VSDRelQuadBezTo(id, level, x, y, a, b);
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
    libvisio::sorted_vector<unsigned> tmpVector;

    for (iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      tmpVector.insert(iter->first);

    for (unsigned i = 0; i < tmpVector.size(); i++)
    {
      iter = m_elements.find(tmpVector[i]);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
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

void libvisio::VSDGeometryList::clearElement(unsigned id)
{
  std::map<unsigned, VSDGeometryListElement *>::iterator iter = m_elements.find(id);
  if (m_elements.end() != iter)
  {
    if (iter->second)
      delete iter->second;
    m_elements.erase(iter);
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
