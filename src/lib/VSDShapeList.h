/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDSHAPELIST_H__
#define __VSDSHAPELIST_H__

#include <vector>
#include <map>

namespace libvisio
{

class VSDShapeList
{
public:
  VSDShapeList();
  ~VSDShapeList();
  VSDShapeList(const VSDShapeList &shapeList);
  VSDShapeList &operator=(const VSDShapeList &shapeList);
  void addShapeId(unsigned id, unsigned shapeId);
  void addShapeId(unsigned shapeId);
  void setElementsOrder(const std::vector<unsigned> &elementsOrder);
  void clear();
  bool empty() const
  {
    return (m_elements.empty());
  }
  const std::vector<unsigned> &getShapesOrder();
private:
  std::map<unsigned, unsigned> m_elements;
  std::vector<unsigned> m_elementsOrder;
  std::vector<unsigned> m_shapesOrder;
};

} // namespace libvisio

#endif // __VSDSHAPELIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
