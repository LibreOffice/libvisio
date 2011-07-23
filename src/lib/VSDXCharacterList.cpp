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
#include "VSDXCharacterList.h"

namespace libvisio {

class VSDXCharacterListElement
{
public:
  VSDXCharacterListElement() {}
  virtual ~VSDXCharacterListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
};

class VSDXCharIX : public VSDXCharacterListElement
{
public:
  VSDXCharIX(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId,
           double fontSize, bool bold, bool italic, bool underline,
           WPXString fontFace) :
    m_id(id), m_level(level), m_charCount(charCount), m_fontID(fontID), m_fontColour(fontColour), m_langId(langId),
    m_fontSize(fontSize), m_bold(bold), m_italic(italic), 
    m_underline(underline), m_fontFace(fontFace) {}
  ~VSDXCharIX() {} 
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  unsigned m_charCount;
  unsigned short m_fontID;
  Colour m_fontColour;
  unsigned m_langId;
  double m_fontSize;
  bool m_bold, m_italic, m_underline;
  WPXString m_fontFace; 
};
} // namespace libvisio


void libvisio::VSDXCharIX::handle(VSDXCollector *collector)
{
  collector->collectCharFormat(m_id, m_level, m_charCount, m_fontID, m_fontColour, m_langId, m_fontSize, m_bold, m_italic, m_underline, m_fontFace);
}

libvisio::VSDXCharacterList::VSDXCharacterList()
{
}

libvisio::VSDXCharacterList::~VSDXCharacterList()
{
  clear();
}

void libvisio::VSDXCharacterList::addCharIX(unsigned id, unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId, double fontSize, bool bold, bool italic, bool underline, WPXString fontFace)
{
  m_elements[id] = new VSDXCharIX(id, level, charCount, fontID, fontColour, langId, fontSize, bold, italic, underline, fontFace);
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
