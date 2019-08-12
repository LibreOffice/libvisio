/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBVISIO_XML_H__
#define __LIBVISIO_XML_H__

#include <memory>

#include <librevenge-stream/librevenge-stream.h>

#include <libxml/xmlreader.h>

namespace libvisio
{

struct Colour;

class XMLErrorWatcher
{
  // disable copying
  XMLErrorWatcher(const XMLErrorWatcher &);
  XMLErrorWatcher &operator=(const XMLErrorWatcher &);

public:
  XMLErrorWatcher();

  bool isError() const;
  void setError();

private:
  bool m_error;
};

// create an xmlTextReader from a librevenge::RVNGInputStream
std::unique_ptr<xmlTextReader, void (*)(xmlTextReaderPtr)>
xmlReaderForStream(librevenge::RVNGInputStream *input, XMLErrorWatcher *watcher = nullptr, bool recover = true);

Colour xmlStringToColour(const xmlChar *s);
Colour xmlStringToColour(const std::shared_ptr<xmlChar> &s);

long xmlStringToLong(const xmlChar *s);
long xmlStringToLong(const std::shared_ptr<xmlChar> &s);

double xmlStringToDouble(const xmlChar *s);
double xmlStringToDouble(const std::shared_ptr<xmlChar> &s);

bool xmlStringToBool(const xmlChar *s);
bool xmlStringToBool(const std::shared_ptr<xmlChar> &s);

} // namespace libvisio

#endif // __LIBVISIO_XML_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
