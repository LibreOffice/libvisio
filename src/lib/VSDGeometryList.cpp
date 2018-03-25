/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDGeometryList.h"

#include "VSDCollector.h"
#include "libvisio_utils.h"

namespace libvisio
{


class VSDGeometry : public VSDGeometryListElement
{
public:
  VSDGeometry(unsigned id, unsigned level, const boost::optional<bool> &noFill,
              const boost::optional<bool> &noLine, const boost::optional<bool> &noShow) :
    VSDGeometryListElement(id, level), m_noFill(get_optional_value_or(noFill, false)),
    m_noLine(get_optional_value_or(noLine, false)), m_noShow(get_optional_value_or(noShow, false)) {}
  ~VSDGeometry() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  bool m_noFill;
  bool m_noLine;
  bool m_noShow;
};

class VSDEmpty : public VSDGeometryListElement
{
public:
  VSDEmpty(unsigned id, unsigned level) :
    VSDGeometryListElement(id, level) {}
  ~VSDEmpty() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
};

class VSDMoveTo : public VSDGeometryListElement
{
public:
  VSDMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)) {}
  ~VSDMoveTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y;
};

class VSDLineTo : public VSDGeometryListElement
{
public:
  VSDLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)) {}
  ~VSDLineTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y;
};

class VSDArcTo : public VSDGeometryListElement
{
public:
  VSDArcTo(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2, const boost::optional<double> &bow) :
    VSDGeometryListElement(id, level), m_x2(get_optional_value_or(x2, 0.0)), m_y2(get_optional_value_or(y2, 0.0)), m_bow(get_optional_value_or(bow, 0.0)) {}
  ~VSDArcTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x2, m_y2, m_bow;
};

class VSDEllipse : public VSDGeometryListElement
{
public:
  VSDEllipse(unsigned id, unsigned level, const boost::optional<double> &cx, const boost::optional<double> &cy,
             const boost::optional<double> &xleft, const boost::optional<double> &yleft,
             const boost::optional<double> &xtop, const boost::optional<double> &ytop) :
    VSDGeometryListElement(id, level), m_cx(get_optional_value_or(cx, 0.0)), m_cy(get_optional_value_or(cy, 0.0)),
    m_xleft(get_optional_value_or(xleft, 0.0)), m_yleft(get_optional_value_or(yleft, 0.0)), m_xtop(get_optional_value_or(xtop, 0.0)),
    m_ytop(get_optional_value_or(ytop, 0.0)) {}
  ~VSDEllipse() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_cx, m_cy, m_xleft, m_yleft, m_xtop, m_ytop;
};

class VSDEllipticalArcTo : public VSDGeometryListElement
{
public:
  VSDEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                     const boost::optional<double> &x2, const boost::optional<double> &y2,
                     const boost::optional<double> &angle, const boost::optional<double> &ecc) :
    VSDGeometryListElement(id, level), m_x3(get_optional_value_or(x3, 0.0)), m_y3(get_optional_value_or(y3, 0.0)), m_x2(get_optional_value_or(x2, 0.0)),
    m_y2(get_optional_value_or(y2, 0.0)), m_angle(get_optional_value_or(angle, 0.0)), m_ecc(get_optional_value_or(ecc, 1.0)) {}
  ~VSDEllipticalArcTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDNURBSTo1 : public VSDGeometryListElement
{
public:
  VSDNURBSTo1(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
              std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) :
    VSDGeometryListElement(id, level), m_x2(x2), m_y2(y2), m_xType(xType), m_yType(yType), m_degree(degree), m_controlPoints(controlPoints), m_knotVector(knotVector), m_weights(weights) {}
  ~VSDNURBSTo1() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

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
  ~VSDNURBSTo2() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

  unsigned getDataID() const override;
  unsigned m_dataID;
  double m_x2, m_y2;
  double m_knot, m_knotPrev;
  double m_weight, m_weightPrev;
};

class VSDNURBSTo3 : public VSDGeometryListElement
{
public:
  VSDNURBSTo3(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2, const boost::optional<double> &knot,
              const boost::optional<double> &knotPrev, const boost::optional<double> &weight, const boost::optional<double> &weightPrev,
              const boost::optional<NURBSData> &data) :
    VSDGeometryListElement(id, level), m_data(get_optional_value_or(data, NURBSData())), m_x2(get_optional_value_or(x2, 0.0)), m_y2(get_optional_value_or(y2, 0.0)),
    m_knot(get_optional_value_or(knot, 0.0)), m_knotPrev(get_optional_value_or(knotPrev, 0.0)), m_weight(get_optional_value_or(weight, 0.0)), m_weightPrev(get_optional_value_or(weightPrev, 0.0)) {}
  ~VSDNURBSTo3() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

  NURBSData m_data;
  double m_x2, m_y2;
  double m_knot, m_knotPrev;
  double m_weight, m_weightPrev;
};

class VSDPolylineTo1 : public VSDGeometryListElement
{
public:
  VSDPolylineTo1(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) :
    VSDGeometryListElement(id, level), m_x(x), m_y(y), m_xType(xType), m_yType(yType), m_points(points) {}
  ~VSDPolylineTo1() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

  double m_x, m_y;
  unsigned m_xType, m_yType;
  std::vector<std::pair<double, double> > m_points;
};

class VSDPolylineTo2 : public VSDGeometryListElement
{
public:
  VSDPolylineTo2(unsigned id, unsigned level, double x, double y, unsigned dataID) :
    VSDGeometryListElement(id, level), m_dataID(dataID), m_x(x), m_y(y) {}
  ~VSDPolylineTo2() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  unsigned getDataID() const override;

  unsigned m_dataID;
  double m_x, m_y;
};

class VSDPolylineTo3 : public VSDGeometryListElement
{
public:
  VSDPolylineTo3(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                 const boost::optional<PolylineData> &data) :
    VSDGeometryListElement(id, level), m_data(get_optional_value_or(data, PolylineData())), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)) {}
  ~VSDPolylineTo3() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

  PolylineData m_data;
  double m_x, m_y;
};

class VSDSplineStart : public VSDGeometryListElement
{
public:
  VSDSplineStart(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                 const boost::optional<double> &secondKnot, const boost::optional<double> &firstKnot,
                 const boost::optional<double> &lastKnot, const boost::optional<unsigned> &degree) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)), m_secondKnot(get_optional_value_or(secondKnot, 0.0)),
    m_firstKnot(get_optional_value_or(firstKnot, 0.0)), m_lastKnot(get_optional_value_or(lastKnot, 0.0)), m_degree(get_optional_value_or(degree, 0)) {}
  ~VSDSplineStart() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;

  double m_x, m_y;
  double m_secondKnot, m_firstKnot, m_lastKnot;
  unsigned m_degree;
};

class VSDSplineKnot : public VSDGeometryListElement
{
public:
  VSDSplineKnot(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                const boost::optional<double> &knot) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)), m_knot(get_optional_value_or(knot, 0.0)) {}
  ~VSDSplineKnot() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y;
  double m_knot;
};

class VSDInfiniteLine : public VSDGeometryListElement
{
public:
  VSDInfiniteLine(unsigned id, unsigned level, const boost::optional<double> &x1, const boost::optional<double> &y1,
                  const boost::optional<double> &x2, const boost::optional<double> &y2) :
    VSDGeometryListElement(id, level), m_x1(get_optional_value_or(x1, 0.0)), m_y1(get_optional_value_or(y1, 0.0)),
    m_x2(get_optional_value_or(x2, 0.0)), m_y2(get_optional_value_or(y2, 0.0)) {}
  ~VSDInfiniteLine() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x1, m_y1, m_x2, m_y2;
};

class VSDRelCubBezTo : public VSDGeometryListElement
{
public:
  VSDRelCubBezTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y, const boost::optional<double> &a,
                 const boost::optional<double> &b, const boost::optional<double> &c, const boost::optional<double> &d) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)),
    m_a(get_optional_value_or(a, 0.0)), m_b(get_optional_value_or(b, 0.0)), m_c(get_optional_value_or(c, 0.0)), m_d(get_optional_value_or(d, 0.0)) {}
  ~VSDRelCubBezTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y, m_a, m_b, m_c, m_d;
};

class VSDRelEllipticalArcTo : public VSDGeometryListElement
{
public:
  VSDRelEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3, const boost::optional<double> &y3,
                        const boost::optional<double> &x2, const boost::optional<double> &y2, const boost::optional<double> &angle,
                        const boost::optional<double> &ecc) :
    VSDGeometryListElement(id, level), m_x3(get_optional_value_or(x3, 0.0)), m_y3(get_optional_value_or(y3, 0.0)),
    m_x2(get_optional_value_or(x2, 0.0)), m_y2(get_optional_value_or(y2, 0.0)), m_angle(get_optional_value_or(angle, 0.0)),
    m_ecc(get_optional_value_or(ecc, 1.0)) {}
  ~VSDRelEllipticalArcTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x3, m_y3, m_x2, m_y2, m_angle, m_ecc;
};

class VSDRelMoveTo : public VSDGeometryListElement
{
public:
  VSDRelMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)) {}
  ~VSDRelMoveTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y;
};

class VSDRelLineTo : public VSDGeometryListElement
{
public:
  VSDRelLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)), m_y(get_optional_value_or(y, 0.0)) {}
  ~VSDRelLineTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
  double m_x, m_y;
};

class VSDRelQuadBezTo : public VSDGeometryListElement
{
public:
  VSDRelQuadBezTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y,
                  const boost::optional<double> &a, const boost::optional<double> &b) :
    VSDGeometryListElement(id, level), m_x(get_optional_value_or(x, 0.0)),
    m_y(get_optional_value_or(y, 0.0)), m_a(get_optional_value_or(a, 0.0)), m_b(get_optional_value_or(b, 0.0)) {}
  ~VSDRelQuadBezTo() override {}
  void handle(VSDCollector *collector) const override;
  VSDGeometryListElement *clone() override;
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

void libvisio::VSDNURBSTo3::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectNURBSTo(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_data);
}

libvisio::VSDGeometryListElement *libvisio::VSDNURBSTo3::clone()
{
  return new VSDNURBSTo3(m_id, m_level, m_x2, m_y2, m_knot, m_knotPrev, m_weight, m_weightPrev, m_data);
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


void libvisio::VSDPolylineTo3::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
  collector->collectPolylineTo(m_id, m_level, m_x, m_y, m_data);
}

libvisio::VSDGeometryListElement *libvisio::VSDPolylineTo3::clone()
{
  return new VSDPolylineTo3(m_id, m_level, m_x, m_y, m_data);
}


void libvisio::VSDSplineStart::handle(VSDCollector *collector) const
{
  collector->collectSplineEnd();
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
  for (auto iter = geomList.m_elements.begin(); iter != geomList.m_elements.end(); ++iter)
    m_elements[iter->first] = clone(iter->second);
}

libvisio::VSDGeometryList &libvisio::VSDGeometryList::operator=(const VSDGeometryList &geomList)
{
  if (this != &geomList)
  {
    clear();
    for (auto iter = geomList.m_elements.begin(); iter != geomList.m_elements.end(); ++iter)
      m_elements[iter->first] = clone(iter->second);
    m_elementsOrder = geomList.m_elementsOrder;
  }
  return *this;
}

libvisio::VSDGeometryList::~VSDGeometryList()
{
}

void libvisio::VSDGeometryList::addGeometry(unsigned id, unsigned level, const boost::optional<bool> &noFill,
                                            const boost::optional<bool> &noLine, const boost::optional<bool> &noShow)
{
  auto *tmpElement = dynamic_cast<VSDGeometry *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDGeometry>(id, level, noFill, noLine, noShow);
  }
  else
  {
    ASSIGN_OPTIONAL(noFill, tmpElement->m_noFill);
    ASSIGN_OPTIONAL(noLine, tmpElement->m_noLine);
    ASSIGN_OPTIONAL(noShow, tmpElement->m_noShow);
  }
}

void libvisio::VSDGeometryList::addEmpty(unsigned id, unsigned level)
{
  m_elements[id] = make_unique<VSDEmpty>(id, level);
}

void libvisio::VSDGeometryList::addMoveTo(unsigned id, unsigned level, const boost::optional<double> &x,
                                          const boost::optional<double> &y)
{
  auto *tmpElement = dynamic_cast<VSDMoveTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDMoveTo>(id, level, x, y);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
  }
}

void libvisio::VSDGeometryList::addLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y)
{
  auto *tmpElement = dynamic_cast<VSDLineTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDLineTo>(id, level, x, y);
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
  auto *tmpElement = dynamic_cast<VSDArcTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDArcTo>(id, level, x2, y2, bow);
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
  m_elements[id] = libvisio::make_unique<VSDNURBSTo1>(id, level, x2, y2, xType, yType, degree, controlPoints, knotVector, weights);
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  m_elements[id] = make_unique<VSDNURBSTo2>(id, level, x2, y2, knot, knotPrev, weight, weightPrev, dataID);
}

void libvisio::VSDGeometryList::addNURBSTo(unsigned id, unsigned level, const boost::optional<double> &x2, const boost::optional<double> &y2,
                                           const boost::optional<double> &knot, const boost::optional<double> &knotPrev, const boost::optional<double> &weight,
                                           const boost::optional<double> &weightPrev, const boost::optional<NURBSData> &data)
{
  auto *tmpElement = dynamic_cast<VSDNURBSTo3 *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDNURBSTo3>(id, level, x2, y2, knot, knotPrev, weight, weightPrev, data);
  }
  else
  {
    ASSIGN_OPTIONAL(x2, tmpElement->m_x2);
    ASSIGN_OPTIONAL(y2, tmpElement->m_y2);
    ASSIGN_OPTIONAL(knot, tmpElement->m_knot);
    ASSIGN_OPTIONAL(knotPrev, tmpElement->m_knotPrev);
    ASSIGN_OPTIONAL(weight, tmpElement->m_weight);
    ASSIGN_OPTIONAL(weightPrev, tmpElement->m_weightPrev);
    ASSIGN_OPTIONAL(data, tmpElement->m_data);
  }
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                                              const std::vector<std::pair<double, double> > &points)
{
  m_elements[id] = libvisio::make_unique<VSDPolylineTo1>(id, level, x, y, xType, yType, points);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID)
{
  m_elements[id] = make_unique<VSDPolylineTo2>(id, level, x, y, dataID);
}

void libvisio::VSDGeometryList::addPolylineTo(unsigned id, unsigned level, boost::optional<double> &x, boost::optional<double> &y, boost::optional<PolylineData> &data)
{
  auto *tmpElement = dynamic_cast<VSDPolylineTo3 *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDPolylineTo3>(id, level, x, y, data);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
    ASSIGN_OPTIONAL(data, tmpElement->m_data);
  }
}

void libvisio::VSDGeometryList::addEllipse(unsigned id, unsigned level, const boost::optional<double> &cx,
                                           const boost::optional<double> &cy,const boost::optional<double> &xleft, const boost::optional<double> &yleft,
                                           const boost::optional<double> &xtop, const boost::optional<double> &ytop)
{
  auto *tmpElement = dynamic_cast<VSDEllipse *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDEllipse>(id, level, cx, cy, xleft, yleft, xtop, ytop);
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
  auto *tmpElement = dynamic_cast<VSDEllipticalArcTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDEllipticalArcTo>(id, level, x3, y3, x2, y2, angle, ecc);
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

void libvisio::VSDGeometryList::addSplineStart(unsigned id, unsigned level, const boost::optional<double> &x,
                                               const boost::optional<double> &y, const boost::optional<double> &secondKnot, const boost::optional<double> &firstKnot,
                                               const boost::optional<double> &lastKnot, const boost::optional<unsigned> &degree)
{
  auto *tmpElement = dynamic_cast<VSDSplineStart *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDSplineStart>(id, level, x, y, secondKnot, firstKnot, lastKnot, degree);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
    ASSIGN_OPTIONAL(secondKnot, tmpElement->m_secondKnot);
    ASSIGN_OPTIONAL(firstKnot, tmpElement->m_firstKnot);
    ASSIGN_OPTIONAL(lastKnot, tmpElement->m_lastKnot);
    ASSIGN_OPTIONAL(degree, tmpElement->m_degree);
  }
}

void libvisio::VSDGeometryList::addSplineKnot(unsigned id, unsigned level, const boost::optional<double> &x,
                                              const boost::optional<double> &y, const boost::optional<double> &knot)
{
  auto *tmpElement = dynamic_cast<VSDSplineKnot *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDSplineKnot>(id, level, x, y, knot);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
    ASSIGN_OPTIONAL(knot, tmpElement->m_knot);
  }
}

void libvisio::VSDGeometryList::addInfiniteLine(unsigned id, unsigned level, const boost::optional<double> &x1,
                                                const boost::optional<double> &y1, const boost::optional<double> &x2, const boost::optional<double> &y2)
{
  auto *tmpElement = dynamic_cast<VSDInfiniteLine *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDInfiniteLine>(id, level, x1, y1, x2, y2);
  }
  else
  {
    ASSIGN_OPTIONAL(x1, tmpElement->m_x1);
    ASSIGN_OPTIONAL(y1, tmpElement->m_y1);
    ASSIGN_OPTIONAL(x2, tmpElement->m_x2);
    ASSIGN_OPTIONAL(y2, tmpElement->m_y2);
  }
}

void libvisio::VSDGeometryList::addRelCubBezTo(unsigned id, unsigned level, const boost::optional<double> &x,
                                               const boost::optional<double> &y, const boost::optional<double> &a, const boost::optional<double> &b,
                                               const boost::optional<double> &c, const boost::optional<double> &d)
{
  auto *tmpElement = dynamic_cast<VSDRelCubBezTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDRelCubBezTo>(id, level, x, y, a, b, c, d);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
    ASSIGN_OPTIONAL(a, tmpElement->m_a);
    ASSIGN_OPTIONAL(b, tmpElement->m_b);
    ASSIGN_OPTIONAL(c, tmpElement->m_c);
    ASSIGN_OPTIONAL(d, tmpElement->m_d);
  }
}

void libvisio::VSDGeometryList::addRelEllipticalArcTo(unsigned id, unsigned level, const boost::optional<double> &x3,
                                                      const boost::optional<double> &y3, const boost::optional<double> &x2, const boost::optional<double> &y2,
                                                      const boost::optional<double> &angle, const boost::optional<double> &ecc)
{
  auto *tmpElement = dynamic_cast<VSDRelEllipticalArcTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDRelEllipticalArcTo>(id, level, x3, y3, x2, y2, angle, ecc);
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

void libvisio::VSDGeometryList::addRelMoveTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y)
{
  auto *tmpElement = dynamic_cast<VSDRelMoveTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDRelMoveTo>(id, level, x, y);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
  }
}

void libvisio::VSDGeometryList::addRelLineTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y)
{
  auto *tmpElement = dynamic_cast<VSDRelLineTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDRelLineTo>(id, level, x, y);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
  }
}

void libvisio::VSDGeometryList::addRelQuadBezTo(unsigned id, unsigned level, const boost::optional<double> &x, const boost::optional<double> &y, const boost::optional<double> &a, const boost::optional<double> &b)
{
  auto *tmpElement = dynamic_cast<VSDRelQuadBezTo *>(m_elements[id].get());
  if (!tmpElement)
  {
    m_elements[id] = make_unique<VSDRelQuadBezTo>(id, level, x, y, a, b);
  }
  else
  {
    ASSIGN_OPTIONAL(x, tmpElement->m_x);
    ASSIGN_OPTIONAL(y, tmpElement->m_y);
    ASSIGN_OPTIONAL(a, tmpElement->m_a);
    ASSIGN_OPTIONAL(b, tmpElement->m_b);
  }
}

void libvisio::VSDGeometryList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned int i : elementsOrder)
    m_elementsOrder.push_back(i);
}

void libvisio::VSDGeometryList::handle(VSDCollector *collector) const
{
  if (empty())
    return;
  if (!m_elementsOrder.empty())
  {
    for (unsigned int i : m_elementsOrder)
    {
      auto iter = m_elements.find(i);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  else
  {
    std::vector<unsigned> tmpVector;

    for (auto iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      tmpVector.push_back(iter->first);
    std::sort(tmpVector.begin(), tmpVector.end());

    for (unsigned int i : tmpVector)
    {
      auto iter = m_elements.find(i);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  collector->collectSplineEnd();
}

void libvisio::VSDGeometryList::clear()
{
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDGeometryListElement *libvisio::VSDGeometryList::getElement(unsigned index) const
{
  if (m_elementsOrder.size() > index)
    index = m_elementsOrder[index];

  auto iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second.get();
  else
    return nullptr;
}

void libvisio::VSDGeometryList::resetLevel(unsigned level)
{
  for (auto &element : m_elements)
    element.second->setLevel(level);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
