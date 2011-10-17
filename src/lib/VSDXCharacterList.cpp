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
#include "VSDXCharacterList.h"

namespace libvisio {

class VSDXCharacterListElement
{
public:
  VSDXCharacterListElement() {}
  virtual ~VSDXCharacterListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
  virtual VSDXCharacterListElement *clone() = 0;
};

class VSDXCharIX : public VSDXCharacterListElement
{
public:
  VSDXCharIX(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId,
             double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
             bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace) :
    m_id(id), m_level(level), m_charCount(charCount), m_fontID(fontID), m_fontColour(fontColour), m_langId(langId),
    m_fontSize(fontSize), m_bold(bold), m_italic(italic), m_underline(underline), m_doubleunderline(doubleunderline),
    m_strikeout(strikeout), m_doublestrikeout(doublestrikeout), m_allcaps(allcaps), m_initcaps(initcaps), m_smallcaps(smallcaps),
    m_superscript(superscript), m_subscript(subscript), m_fontFace(fontFace) {}
  ~VSDXCharIX() {} 
  void handle(VSDXCollector *collector);
  VSDXCharacterListElement *clone();
private:
  unsigned m_id, m_level;
  unsigned m_charCount;
  unsigned short m_fontID;
  Colour m_fontColour;
  unsigned m_langId;
  double m_fontSize;
  bool m_bold, m_italic, m_underline, m_doubleunderline, m_strikeout, m_doublestrikeout;
  bool m_allcaps, m_initcaps, m_smallcaps, m_superscript, m_subscript;
  WPXString m_fontFace; 
};
} // namespace libvisio


void libvisio::VSDXCharIX::handle(VSDXCollector *collector)
{
  collector->collectCharFormat(m_id, m_level, m_charCount, m_fontID, m_fontColour, m_langId, m_fontSize, m_bold, m_italic, m_underline,
                               m_doubleunderline, m_strikeout, m_doublestrikeout, m_allcaps, m_initcaps, m_smallcaps,
                               m_superscript, m_subscript, m_fontFace);
}

libvisio::VSDXCharacterListElement *libvisio::VSDXCharIX::clone()
{
  return new VSDXCharIX(m_id, m_level, m_charCount, m_fontID, m_fontColour, m_langId, m_fontSize, m_bold, m_italic, m_underline,
                        m_doubleunderline, m_strikeout, m_doublestrikeout, m_allcaps, m_initcaps, m_smallcaps,
                        m_superscript, m_subscript, m_fontFace);
}


libvisio::VSDXCharacterList::VSDXCharacterList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDXCharacterList::VSDXCharacterList(const libvisio::VSDXCharacterList &charList) :
  m_elements(),
  m_elementsOrder(charList.m_elementsOrder)
{
  std::map<unsigned, VSDXCharacterListElement *>::const_iterator iter = charList.m_elements.begin();
  for (; iter != charList.m_elements.end(); iter++)
      m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDXCharacterList &libvisio::VSDXCharacterList::operator=(const libvisio::VSDXCharacterList &charList)
{
  clear();
  std::map<unsigned, VSDXCharacterListElement *>::const_iterator iter = charList.m_elements.begin();
  for (; iter != charList.m_elements.end(); iter++)
      m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = charList.m_elementsOrder;
  return *this;
}

libvisio::VSDXCharacterList::~VSDXCharacterList()
{
  clear();
}

void libvisio::VSDXCharacterList::addCharIX(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId,
                                            double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                                            bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace)
{
  m_elements[id] = new VSDXCharIX(id, level, charCount, fontID, fontColour, langId, fontSize, bold, italic, underline, doubleunderline,
                                  strikeout, doublestrikeout, allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
}

void libvisio::VSDXCharacterList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXCharacterList::handle(VSDXCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDXCharacterListElement *>::iterator iter;
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

void libvisio::VSDXCharacterList::clear()
{
  for (std::map<unsigned, VSDXCharacterListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDXCharacterListElement *libvisio::VSDXCharacterList::getElement(unsigned index)
{
  std::map<unsigned, VSDXCharacterListElement *>::iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
