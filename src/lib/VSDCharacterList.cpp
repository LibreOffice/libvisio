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
#include "VSDCharacterList.h"

namespace libvisio
{

class VSDCharacterListElement
{
public:
  VSDCharacterListElement() {}
  virtual ~VSDCharacterListElement() {}
  virtual void handle(VSDCollector *collector) const = 0;
  virtual VSDCharacterListElement *clone() = 0;
};

class VSDCharIX : public VSDCharacterListElement
{
public:
  VSDCharIX(unsigned id, unsigned level, const boost::optional<unsigned> &charCount, const boost::optional<unsigned short> &fontID,
            const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
            const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
            const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
            const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
            const boost::optional<bool> &subscript, const boost::optional<VSDFont> &fontFace) :
    m_id(id), m_level(level), m_charCount(FROM_OPTIONAL(charCount, 0)), m_fontID(FROM_OPTIONAL(fontID, 0)),
    m_fontColour(FROM_OPTIONAL(fontColour, Colour())), m_fontSize(FROM_OPTIONAL(fontSize, 0.1668)), m_bold(FROM_OPTIONAL(bold, false)),
    m_italic(FROM_OPTIONAL(italic, false)), m_underline(FROM_OPTIONAL(underline, false)), m_doubleunderline(FROM_OPTIONAL(doubleunderline, false)),
    m_strikeout(FROM_OPTIONAL(strikeout, false)), m_doublestrikeout(FROM_OPTIONAL(doublestrikeout, false)),
    m_allcaps(FROM_OPTIONAL(allcaps, false)), m_initcaps(FROM_OPTIONAL(initcaps, false)), m_smallcaps(FROM_OPTIONAL(smallcaps, false)),
    m_superscript(FROM_OPTIONAL(superscript, false)), m_subscript(FROM_OPTIONAL(subscript, false)), m_fontFace(FROM_OPTIONAL(fontFace, VSDFont())) {}
  ~VSDCharIX() {}
  void handle(VSDCollector *collector) const;
  VSDCharacterListElement *clone();
private:
  unsigned m_id, m_level;
  unsigned m_charCount;
  unsigned short m_fontID;
  Colour m_fontColour;
  double m_fontSize;
  bool m_bold, m_italic, m_underline, m_doubleunderline, m_strikeout, m_doublestrikeout;
  bool m_allcaps, m_initcaps, m_smallcaps, m_superscript, m_subscript;
  VSDFont m_fontFace;
};
} // namespace libvisio


void libvisio::VSDCharIX::handle(VSDCollector *collector) const
{
  collector->collectCharIX(m_id, m_level, m_charCount, m_fontID, m_fontColour, m_fontSize, m_bold, m_italic, m_underline,
                           m_doubleunderline, m_strikeout, m_doublestrikeout, m_allcaps, m_initcaps, m_smallcaps,
                           m_superscript, m_subscript, m_fontFace);
}

libvisio::VSDCharacterListElement *libvisio::VSDCharIX::clone()
{
  return new VSDCharIX(m_id, m_level, m_charCount, m_fontID, m_fontColour, m_fontSize, m_bold, m_italic, m_underline,
                       m_doubleunderline, m_strikeout, m_doublestrikeout, m_allcaps, m_initcaps, m_smallcaps,
                       m_superscript, m_subscript, m_fontFace);
}


libvisio::VSDCharacterList::VSDCharacterList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDCharacterList::VSDCharacterList(const libvisio::VSDCharacterList &charList) :
  m_elements(),
  m_elementsOrder(charList.m_elementsOrder)
{
  std::map<unsigned, VSDCharacterListElement *>::const_iterator iter = charList.m_elements.begin();
  for (; iter != charList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDCharacterList &libvisio::VSDCharacterList::operator=(const libvisio::VSDCharacterList &charList)
{
  clear();
  std::map<unsigned, VSDCharacterListElement *>::const_iterator iter = charList.m_elements.begin();
  for (; iter != charList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = charList.m_elementsOrder;
  return *this;
}

libvisio::VSDCharacterList::~VSDCharacterList()
{
  clear();
}

void libvisio::VSDCharacterList::addCharIX(unsigned id, unsigned level, const boost::optional<unsigned> &charCount, const boost::optional<unsigned short> &fontID,
    const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
    const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
    const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
    const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
    const boost::optional<bool> &subscript, const boost::optional<VSDFont> &fontFace)
{
  m_elements[id] = new VSDCharIX(id, level, charCount, fontID, fontColour, fontSize, bold, italic, underline, doubleunderline,
                                 strikeout, doublestrikeout, allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
}

void libvisio::VSDCharacterList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDCharacterList::handle(VSDCollector *collector) const
{
  if (empty())
    return;
  std::map<unsigned, VSDCharacterListElement *>::const_iterator iter;
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
}

void libvisio::VSDCharacterList::clear()
{
  for (std::map<unsigned, VSDCharacterListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
