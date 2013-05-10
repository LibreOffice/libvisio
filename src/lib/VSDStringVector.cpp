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
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the procdrns of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include <vector>
#include <libvisio/libvisio.h>

namespace libvisio
{
class VSDStringVectorImpl
{
public:
  VSDStringVectorImpl() : m_strings() {}
  ~VSDStringVectorImpl() {}
  std::vector<WPXString> m_strings;
};

} // namespace libvisio

libvisio::VSDStringVector::VSDStringVector()
  : m_pImpl(new VSDStringVectorImpl())
{
}

libvisio::VSDStringVector::VSDStringVector(const VSDStringVector &vec)
  : m_pImpl(new VSDStringVectorImpl(*(vec.m_pImpl)))
{
}

libvisio::VSDStringVector::~VSDStringVector()
{
  delete m_pImpl;
}

libvisio::VSDStringVector &libvisio::VSDStringVector::operator=(const VSDStringVector &vec)
{
  // Check for self-assignment
  if (this == &vec)
    return *this;
  if (m_pImpl)
    delete m_pImpl;
  m_pImpl = new VSDStringVectorImpl(*(vec.m_pImpl));
  return *this;
}

unsigned libvisio::VSDStringVector::size() const
{
  return (unsigned)(m_pImpl->m_strings.size());
}

bool libvisio::VSDStringVector::empty() const
{
  return m_pImpl->m_strings.empty();
}

const WPXString &libvisio::VSDStringVector::operator[](unsigned idx) const
{
  return m_pImpl->m_strings[idx];
}

void libvisio::VSDStringVector::append(const WPXString &str)
{
  m_pImpl->m_strings.push_back(str);
}

void libvisio::VSDStringVector::clear()
{
  m_pImpl->m_strings.clear();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
