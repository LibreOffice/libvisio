/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 */

#include "VSDXCollector.h"
#include "VSDXGeometryList.h"

namespace libvisio {

class VSDXGeometryListElement
{
public:
  VSDXGeometryListElement() {}
  virtual ~VSDXGeometryListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
};

class VSDXGeometry : public VSDXGeometryListElement
{
public:
  VSDXGeometry(unsigned id, unsigned level, unsigned geomFlags) :
    m_id(id), m_level(level), m_geomFlags(geomFlags) {}
  ~VSDXGeometry() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id;
  unsigned m_level;
  unsigned m_geomFlags;
};

class VSDXMoveTo : public VSDXGeometryListElement
{
public:
  VSDXMoveTo(unsigned id, unsigned level, double x, double y) :
    m_id(id), m_level(level), m_x(x), m_y(y) {}
  ~VSDXMoveTo() {}
  void handle(VSDXCollector *collector);
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
private:
  unsigned m_id, m_level;
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDXNURBSTo1 : public VSDXGeometryListElement
{
public:
  VSDXNURBSTo1(unsigned id, unsigned level, double x2, double y2, unsigned xType, unsigned yType, double degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_xType(xType), m_yType(yType), m_degree(degree), m_controlPoints(controlPoints), m_knotVector(knotVector), m_weights(weights) {}
  ~VSDXNURBSTo1() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  double m_x2, m_y2;
  unsigned m_xType, m_yType;
  double m_degree;
  std::vector<std::pair<double, double> > m_controlPoints;
  std::vector<double> m_knotVector, m_weights;
};

class VSDXNURBSTo2 : public VSDXGeometryListElement
{
public:
  VSDXNURBSTo2(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) :
    m_id(id), m_level(level), m_x2(x2), m_y2(y2), m_knot(knot), m_knotPrev(knotPrev), m_weight(weight), m_weightPrev(weightPrev), m_dataID(dataID) {}
  ~VSDXNURBSTo2() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  double m_x2, m_y2;
  double m_knot, m_knotPrev;
  double m_weight, m_weightPrev;
  unsigned m_dataID;
};

class VSDXPolylineTo1 : public VSDXGeometryListElement
{
public:
  VSDXPolylineTo1(unsigned id , unsigned level, double x, double y, unsigned xType, unsigned yType, std::vector<std::pair<double, double> > points) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_xType(xType), m_yType(yType), m_points(points) {}
  ~VSDXPolylineTo1() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  unsigned m_xType, m_yType;
  std::vector<std::pair<double, double> > m_points;
};

class VSDXPolylineTo2 : public VSDXGeometryListElement
{
public:
  VSDXPolylineTo2(unsigned id , unsigned level, double x, double y, unsigned dataID) :
    m_id(id), m_level(level), m_x(x), m_y(y), m_dataID(dataID) {}
  ~VSDXPolylineTo2() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  double m_x, m_y;
  unsigned m_dataID;
};

} // namespace libvisio


void libvisio::VSDXGeometry::handle(VSDXCollector *collector)
{
  collector->collectGeometry(m_id, m_level, m_geomFlags);
}

void libvisio::VSDXMoveTo::handle(VSDXCollector *collector)
{
  collector->collectMoveTo(m_id, m_level, m_x, m_y);
}

void libvisio::VSDXLineTo::handle(VSDXCollector *collector)
{
  collector->collectLineTo(m_id, m_level, m_x, m_y);
}

void libvisio::VSDXArcTo::handle(VSDXCollector *collector)
{
  collector->collectArcTo(m_id, m_level, m_x2, m_y2, m_bow);
}

void libvisio::VSDXEllipse::handle(VSDXCollector *collector)
{
  collector->collectEllipse(m_id, m_level, m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop);
}

void libvisio::VSDXEllipticalArcTo::handle(VSDXCollector *collector)
{
  collector->collectEllipticalArcTo(m_id, m_level, m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc);
}

void libvisio::VSDXNURBSTo1::handle(VSDXCollector *collector)
{
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_xType, m_yType, m_degree, m_controlPoints, m_knotVector, m_weights);
}

void libvisio::VSDXNURBSTo2::handle(VSDXCollector *collector)
{
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_dataID);
}

void libvisio::VSDXPolylineTo1::handle(VSDXCollector *collector)
{
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_xType, m_yType, m_points);
}

void libvisio::VSDXPolylineTo2::handle(VSDXCollector *collector)
{
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_dataID);
}

libvisio::VSDXGeometryList::VSDXGeometryList()
{
}

libvisio::VSDXGeometryList::~VSDXGeometryList()
{
  clear();
}

void libvisio::VSDXGeometryList::addGeometry(unsigned id, unsigned level, unsigned geomFlags)
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

void libvisio::VSDXGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned xType, unsigned yType, double degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  m_elements[id] = new VSDXNURBSTo1(id, level, x2, y2, xType, yType, degree, controlPoints, knotVector, weights);
}

void libvisio::VSDXGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  m_elements[id] = new VSDXNURBSTo2(id, level, x2, y2, knot, knotPrev, weight, weightPrev, dataID);
}

void libvisio::VSDXGeometryList::addPolylineTo(unsigned id , unsigned level, double x, double y, unsigned xType, unsigned yType, std::vector<std::pair<double, double> > points)
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

void libvisio::VSDXGeometryList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXGeometryList::handle(VSDXCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDXGeometryListElement *>::iterator iter;
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
}

void libvisio::VSDXGeometryList::clear()
{
  for (std::map<unsigned, VSDXGeometryListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}
