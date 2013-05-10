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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stack>

#include <libwpd-stream/libwpd-stream.h>
#include <libwpd/libwpd.h>
#include <libvisio/libvisio.h>

enum PainterCallback
{
  PC_START_GRAPHICS = 0,
  PC_START_LAYER,
  PC_START_EMBEDDED_GRAPHICS,
  PC_START_TEXT_OBJECT,
  PC_START_TEXT_LINE,
  PC_START_TEXT_SPAN
};

#ifdef _U
#undef _U
#endif

#define _U(M, L) \
	if (!m_printCallgraphScore) \
			__iuprintf M; \
	else \
		m_callStack.push(L);

#ifdef _D
#undef _D
#endif

#define _D(M, L) \
	if (!m_printCallgraphScore) \
			__idprintf M; \
	else \
	{ \
		PainterCallback lc = m_callStack.top(); \
		if (lc != L) \
			m_callbackMisses++; \
		m_callStack.pop(); \
	}

class RawPainter : public libwpg::WPGPaintInterface
{
public:
  RawPainter(bool printCallgraphScore);

  ~RawPainter();

  void startGraphics(const ::WPXPropertyList &propList);
  void endGraphics();
  void startLayer(const ::WPXPropertyList &propList);
  void endLayer();
  void startEmbeddedGraphics(const ::WPXPropertyList &propList);
  void endEmbeddedGraphics();

  void setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient);

  void drawRectangle(const ::WPXPropertyList &propList);
  void drawEllipse(const ::WPXPropertyList &propList);
  void drawPolyline(const ::WPXPropertyListVector &vertices);
  void drawPolygon(const ::WPXPropertyListVector &vertices);
  void drawPath(const ::WPXPropertyListVector &path);
  void drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  void startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path);
  void endTextObject();
  void startTextLine(const ::WPXPropertyList &propList);
  void endTextLine();
  void startTextSpan(const ::WPXPropertyList &propList);
  void endTextSpan();
  void insertText(const ::WPXString &str);

private:
  int m_indent;
  int m_callbackMisses;
  bool m_printCallgraphScore;
  std::stack<PainterCallback> m_callStack;

  void __indentUp()
  {
    m_indent++;
  }
  void __indentDown()
  {
    if (m_indent > 0) m_indent--;
  }

  void __iprintf(const char *format, ...);
  void __iuprintf(const char *format, ...);
  void __idprintf(const char *format, ...);
};

WPXString getPropString(const WPXPropertyList &propList)
{
  WPXString propString;
  WPXPropertyList::Iter i(propList);
  if (!i.last())
  {
    propString.append(i.key());
    propString.append(": ");
    propString.append(i()->getStr().cstr());
    for (; i.next(); )
    {
      propString.append(", ");
      propString.append(i.key());
      propString.append(": ");
      propString.append(i()->getStr().cstr());
    }
  }

  return propString;
}

WPXString getPropString(const WPXPropertyListVector &itemList)
{
  WPXString propString;

  propString.append("(");
  WPXPropertyListVector::Iter i(itemList);

  if (!i.last())
  {
    propString.append("(");
    propString.append(getPropString(i()));
    propString.append(")");

    for (; i.next();)
    {
      propString.append(", (");
      propString.append(getPropString(i()));
      propString.append(")");
    }

  }
  propString.append(")");

  return propString;
}

RawPainter::RawPainter(bool printCallgraphScore):
  libwpg::WPGPaintInterface(),
  m_indent(0),
  m_callbackMisses(0),
  m_printCallgraphScore(printCallgraphScore),
  m_callStack()
{
}

RawPainter::~RawPainter()
{
  if (m_printCallgraphScore)
    printf("%d\n", (int)(m_callStack.size() + m_callbackMisses));
}

void RawPainter::__iprintf(const char *format, ...)
{
  if (m_printCallgraphScore) return;

  va_list args;
  va_start(args, format);
  for (int i=0; i<m_indent; i++)
    printf("  ");
  vprintf(format, args);
  va_end(args);
}

void RawPainter::__iuprintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  for (int i=0; i<m_indent; i++)
    printf("  ");
  vprintf(format, args);
  __indentUp();
  va_end(args);
}

void RawPainter::__idprintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  __indentDown();
  for (int i=0; i<m_indent; i++)
    printf("  ");
  vprintf(format, args);
  va_end(args);
}

void RawPainter::startGraphics(const ::WPXPropertyList &propList)
{
  _U(("RawPainter::startGraphics(%s)\n", getPropString(propList).cstr()), PC_START_GRAPHICS);
}

void RawPainter::endGraphics()
{
  _D(("RawPainter::endGraphics\n"), PC_START_GRAPHICS);
}

void RawPainter::startLayer(const ::WPXPropertyList &propList)
{
  _U(("RawPainter::startLayer (%s)\n", getPropString(propList).cstr()), PC_START_LAYER);
}

void RawPainter::endLayer()
{
  _D(("RawPainter::endLayer\n"), PC_START_LAYER);
}

void RawPainter::startEmbeddedGraphics(const ::WPXPropertyList &propList)
{
  _U(("RawPainter::startEmbeddedGraphics (%s)\n", getPropString(propList).cstr()), PC_START_EMBEDDED_GRAPHICS);
}

void RawPainter::endEmbeddedGraphics()
{
  _D(("RawPainter::endEmbeddedGraphics \n"), PC_START_EMBEDDED_GRAPHICS);
}

void RawPainter::setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::setStyle(%s, gradient: (%s))\n", getPropString(propList).cstr(), getPropString(gradient).cstr());
}

void RawPainter::drawRectangle(const ::WPXPropertyList &propList)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawRectangle (%s)\n", getPropString(propList).cstr());
}

void RawPainter::drawEllipse(const ::WPXPropertyList &propList)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawEllipse (%s)\n", getPropString(propList).cstr());
}

void RawPainter::drawPolyline(const ::WPXPropertyListVector &vertices)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawPolyline (%s)\n", getPropString(vertices).cstr());
}

void RawPainter::drawPolygon(const ::WPXPropertyListVector &vertices)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawPolygon (%s)\n", getPropString(vertices).cstr());
}

void RawPainter::drawPath(const ::WPXPropertyListVector &path)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawPath (%s)\n", getPropString(path).cstr());
}

void RawPainter::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData & /*binaryData*/)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::drawGraphicObject (%s)\n", getPropString(propList).cstr());
}

void RawPainter::startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path)
{
  _U(("RawPainter::startTextObject (%s, path: (%s))\n", getPropString(propList).cstr(), getPropString(path).cstr()), PC_START_TEXT_OBJECT);
}

void RawPainter::endTextObject()
{
  _D(("RawPainter::endTextObject\n"), PC_START_TEXT_OBJECT);
}

void RawPainter::startTextLine(const ::WPXPropertyList &propList)
{
  _U(("RawPainter::startTextLine (%s)\n", getPropString(propList).cstr()), PC_START_TEXT_LINE);
}

void RawPainter::endTextLine()
{
  _D(("RawPainter::endTextLine\n"), PC_START_TEXT_LINE);
}

void RawPainter::startTextSpan(const ::WPXPropertyList &propList)
{
  _U(("RawPainter::startTextSpan (%s)\n", getPropString(propList).cstr()), PC_START_TEXT_SPAN);
}

void RawPainter::endTextSpan()
{
  _D(("RawPainter::endTextSpan\n"), PC_START_TEXT_SPAN);
}

void RawPainter::insertText(const ::WPXString &str)
{
  if (m_printCallgraphScore)
    return;

  __iprintf("RawPainter::insertText (%s)\n", str.cstr());
}


namespace
{

int printUsage()
{
  printf("Usage: vsd2raw [OPTION] <Visio Stencils File>\n");
  printf("\n");
  printf("Options:\n");
  printf("--callgraph           Display the call graph nesting level\n");
  printf("--help                Shows this help message\n");
  return -1;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  bool printIndentLevel = false;
  char *file = 0;

  if (argc < 2)
    return printUsage();

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--callgraph"))
      printIndentLevel = true;
    else if (!file && strncmp(argv[i], "--", 2))
      file = argv[i];
    else
      return printUsage();
  }

  if (!file)
    return printUsage();

  WPXFileStream input(file);

  if (!libvisio::VisioDocument::isSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
    return 1;
  }

  RawPainter painter(printIndentLevel);
  if (!libvisio::VisioDocument::parseStencils(&input, &painter))
  {
    fprintf(stderr, "ERROR: Parsing of document failed!\n");
    return 1;
  }

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
