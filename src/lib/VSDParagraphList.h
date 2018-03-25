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
  void addParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                 const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                 const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                 const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                 const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                 const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags);
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
