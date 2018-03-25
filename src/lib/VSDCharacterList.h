/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDCHARACTERLIST_H__
#define __VSDCHARACTERLIST_H__

#include <memory>
#include <vector>
#include <map>
#include "VSDTypes.h"
#include "VSDStyles.h"

namespace libvisio
{

class VSDCharacterListElement;
class VSDCollector;

class VSDCharacterList
{
public:
  VSDCharacterList();
  VSDCharacterList(const VSDCharacterList &charList);
  ~VSDCharacterList();
  VSDCharacterList &operator=(const VSDCharacterList &charList);
  void addCharIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                 const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                 const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                 const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                 const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                 const boost::optional<bool> &subscript, const boost::optional<double> &scaleWidth);
  void addCharIX(unsigned id, unsigned level, const VSDOptionalCharStyle &style);
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
  std::map<unsigned, std::unique_ptr<VSDCharacterListElement>> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDCHARACTERLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
