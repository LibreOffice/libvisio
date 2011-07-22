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

class VSDXText : public VSDXCharacterListElement
{
public:
  VSDXText(unsigned id , unsigned level, const WPXString &text) :
    m_id(id), m_level(level), m_text(text) {}
  ~VSDXText() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id, m_level;
  const WPXString m_text;
};
} // namespace libvisio


void libvisio::VSDXText::handle(VSDXCollector *collector)
{
  collector->collectText(m_id, m_level, m_text);
}

libvisio::VSDXCharacterList::VSDXCharacterList()
{
}

libvisio::VSDXCharacterList::~VSDXCharacterList()
{
  clear();
}

void libvisio::VSDXCharacterList::addText(unsigned id, unsigned level, const WPXString &text)
{
  m_elements[id] = new VSDXText(id, level, text);
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
