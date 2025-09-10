/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDPARAGRAPHLIST_H__
#define __VSDPARAGRAPHLIST_H__

#include <memory>
#include <vector>
#include <map>
#include "VSDStyles.h"

namespace libvisio
{

class VSDParagraphListElement;
class VSDCollector;

class VSDParagraphList
{
public:
  VSDParagraphList();
  VSDParagraphList(const VSDParagraphList &paraList);
  ~VSDParagraphList();
  VSDParagraphList &operator=(const VSDParagraphList &paraList);
  void addParaIX(unsigned id, unsigned level, unsigned charCount, const std::optional<double> &indFirst,
                 const std::optional<double> &indLeft, const std::optional<double> &indRight, const std::optional<double> &spLine,
                 const std::optional<double> &spBefore, const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                 const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                 const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                 const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags);
  void addParaIX(unsigned id, unsigned level, const VSDOptionalParaStyle &style);
  unsigned getCharCount(unsigned id) const;
  void setCharCount(unsigned id, unsigned charCount);
  void resetCharCount();
  unsigned getLevel() const;

  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void handle(VSDCollector *collector) const;
  void clear();
  bool empty() const
  {
    return (m_elements.empty());
  }
private:
  std::map<unsigned, std::unique_ptr<VSDParagraphListElement>> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDPARAGRAPHLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
