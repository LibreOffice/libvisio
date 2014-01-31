/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDFIELDLIST_H__
#define __VSDFIELDLIST_H__

#include <vector>
#include <map>
#include <librevenge/librevenge.h>
#include "VSDDocumentStructure.h"
#include "VSDTypes.h"

namespace libvisio
{

class VSDCollector;

class VSDFieldListElement
{
public:
  VSDFieldListElement() {}
  virtual ~VSDFieldListElement() {}
  virtual void handle(VSDCollector *collector) const = 0;
  virtual VSDFieldListElement *clone() = 0;
  virtual librevenge::RVNGString getString(const std::map<unsigned, librevenge::RVNGString> &) = 0;
  virtual void setNameId(int) = 0;
  virtual void setFormat(unsigned short) = 0;
  virtual void setValue(double) = 0;
};

class VSDTextField : public VSDFieldListElement
{
public:
  VSDTextField(unsigned id, unsigned level, int nameId, int formatStringId)
    : m_id(id),
      m_level(level),
      m_nameId(nameId),
      m_formatStringId(formatStringId) {}
  ~VSDTextField() {}
  void handle(VSDCollector *collector) const;
  VSDFieldListElement *clone();
  librevenge::RVNGString getString(const std::map<unsigned, librevenge::RVNGString> &strVec);
  void setNameId(int nameId);
  void setFormat(unsigned short) {}
  void setValue(double) {}
private:
  unsigned m_id, m_level;
  int m_nameId, m_formatStringId;
};

class VSDNumericField : public VSDFieldListElement
{
public:
  VSDNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId)
    : m_id(id),
      m_level(level),
      m_format(format),
      m_number(number),
      m_formatStringId(formatStringId) {}
  ~VSDNumericField() {}
  void handle(VSDCollector *collector) const;
  VSDFieldListElement *clone();
  librevenge::RVNGString getString(const std::map<unsigned, librevenge::RVNGString> &);
  void setNameId(int) {}
  void setFormat(unsigned short format);
  void setValue(double number);
private:
  librevenge::RVNGString datetimeToString(const char *format, double datetime);
  unsigned m_id, m_level;
  unsigned short m_format;
  double m_number;
  int m_formatStringId;
};

class VSDFieldList
{
public:
  VSDFieldList();
  VSDFieldList(const VSDFieldList &fieldList);
  ~VSDFieldList();
  VSDFieldList &operator=(const VSDFieldList &fieldList);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void addFieldList(unsigned id, unsigned level);
  void addTextField(unsigned id, unsigned level, int nameId, int formatStringId);
  void addNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId);
  void addClonedField(unsigned id);
  void handle(VSDCollector *collector) const;
  void clear();
  unsigned long size() const
  {
    return (unsigned long)m_elements.size();
  }
  bool empty() const
  {
    return (m_elements.empty());
  }
  VSDFieldListElement *getElement(unsigned index);
private:
  std::map<unsigned, VSDFieldListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
  unsigned m_id, m_level;
};

} // namespace libvisio

#endif // __VSDFIELDLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
