/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "libvisio_xml.h"

#ifndef BOOST_LEXICAL_CAST_ASSUME_C_LOCALE
#define BOOST_LEXICAL_CAST_ASSUME_C_LOCALE 1
#endif
#include <boost/lexical_cast.hpp>

#include "VSDTypes.h"
#include "libvisio_utils.h"

namespace libvisio
{

namespace
{

extern "C"
{

  static int vsdxInputCloseFunc(void *)
  {
    return 0;
  }

  static int vsdxInputReadFunc(void *context, char *buffer, int len)
  {
    auto *input = (librevenge::RVNGInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    if (input->isEnd())
      return 0;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read(len, tmpNumBytesRead);

    if (tmpBuffer && tmpNumBytesRead)
      memcpy(buffer, tmpBuffer, tmpNumBytesRead);
    return tmpNumBytesRead;
  }

#ifdef DEBUG
  static void vsdxReaderErrorFunc(void *arg, const char *message, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#else
  static void vsdxReaderErrorFunc(void *arg, const char *, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#endif
  {
    auto *const watcher = reinterpret_cast<XMLErrorWatcher *>(arg);
    switch (severity)
    {
    case XML_PARSER_SEVERITY_VALIDITY_WARNING:
      VSD_DEBUG_MSG(("Found xml parser severity validity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_VALIDITY_ERROR:
      VSD_DEBUG_MSG(("Found xml parser severity validity error %s\n", message));
      break;
    case XML_PARSER_SEVERITY_WARNING:
      VSD_DEBUG_MSG(("Found xml parser severity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_ERROR:
      VSD_DEBUG_MSG(("Found xml parser severity error %s\n", message));
      if (watcher)
        watcher->setError();
      break;
    default:
      break;
    }
  }

} // extern "C"

} // anonymous namespace

XMLErrorWatcher::XMLErrorWatcher()
  : m_error(false)
{
}

bool XMLErrorWatcher::isError() const
{
  return m_error;
}

void XMLErrorWatcher::setError()
{
  m_error = true;
}

std::unique_ptr<xmlTextReader, void (*)(xmlTextReaderPtr)>
xmlReaderForStream(librevenge::RVNGInputStream *input, XMLErrorWatcher *const watcher, bool recover)
{
  int options = XML_PARSE_NOBLANKS | XML_PARSE_NONET;
  if (recover)
    options |= XML_PARSE_RECOVER;
  std::unique_ptr<xmlTextReader, void (*)(xmlTextReaderPtr)> reader
  {
    xmlReaderForIO(vsdxInputReadFunc, vsdxInputCloseFunc, (void *)input, nullptr, nullptr, options),
    xmlFreeTextReader
  };
  if (reader)
    xmlTextReaderSetErrorHandler(reader.get(), vsdxReaderErrorFunc, watcher);
  return reader;
}

Colour xmlStringToColour(const xmlChar *s)
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return Colour();
  std::string str((const char *)s);
  if (str[0] == '#')
  {
    if (str.length() != 7)
    {
      VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
      throw XmlParserException();
    }
    else
      str.erase(str.begin());
  }
  else
  {
    if (str.length() != 6)
    {
      VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
      throw XmlParserException();
    }
  }

  std::istringstream istr(str);
  unsigned val = 0;
  istr >> std::hex >> val;

  return Colour((val & 0xff0000) >> 16, (val & 0xff00) >> 8, val & 0xff, 0);
}

Colour xmlStringToColour(const std::shared_ptr<xmlChar> &s)
{
  return xmlStringToColour(s.get());
}

long xmlStringToLong(const xmlChar *s)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast;
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0;

  try
  {
    return boost::lexical_cast<long, const char *>((const char *)s);
  }
  catch (const boost::bad_lexical_cast &)
  {
    VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
    throw XmlParserException();
  }
  return 0;
}

long xmlStringToLong(const std::shared_ptr<xmlChar> &s)
{
  return xmlStringToLong(s.get());
}

double xmlStringToDouble(const xmlChar *s) try
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0.0;

  return boost::lexical_cast<double, const char *>((const char *)s);
}
catch (const boost::bad_lexical_cast &)
{
  VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
  throw XmlParserException();
}

double xmlStringToDouble(const std::shared_ptr<xmlChar> &s)
{
  return xmlStringToDouble(s.get());
}

bool xmlStringToBool(const xmlChar *s)
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0;

  bool value = false;
  if (xmlStrEqual(s, BAD_CAST("true")) || xmlStrEqual(s, BAD_CAST("1")))
    value = true;
  else if (xmlStrEqual(s, BAD_CAST("false")) || xmlStrEqual(s, BAD_CAST("0")))
    value = false;
  else
  {
    VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
    throw XmlParserException();
  }
  return value;

}

bool xmlStringToBool(const std::shared_ptr<xmlChar> &s)
{
  return xmlStringToBool(s.get());
}

} // namespace libvisio

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
