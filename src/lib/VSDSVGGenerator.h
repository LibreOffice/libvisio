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

#ifndef __VISIOSVGGENERATOR_H__
#define __VISIOSVGGENERATOR_H__

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>
#include <libvisio/libvisio.h>

namespace libvisio
{
struct VSDSVGGeneratorPrivate;

class VSDSVGGenerator : public libwpg::WPGPaintInterface
{
public:
  VSDSVGGenerator(VSDStringVector &vec, const WPXString &nmspace="svg");
  ~VSDSVGGenerator();

  void startGraphics(const ::WPXPropertyList &propList);
  void endGraphics();
  void startLayer(const ::WPXPropertyList &propList);
  void endLayer();
  void startEmbeddedGraphics(const ::WPXPropertyList & /*propList*/) {}
  void endEmbeddedGraphics() {}

  void setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient);

  void drawRectangle(const ::WPXPropertyList &propList);
  void drawEllipse(const ::WPXPropertyList &propList);
  void drawPolyline(const ::WPXPropertyListVector &vertices);
  void drawPolygon(const ::WPXPropertyListVector &vertices);
  void drawPath(const ::WPXPropertyListVector &path);
  void drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  void startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path);
  void endTextObject();
  void startTextLine(const ::WPXPropertyList & /* propList */) {}
  void endTextLine() {}
  void startTextSpan(const ::WPXPropertyList &propList);
  void endTextSpan();
  void insertText(const ::WPXString &str);

private:
  VSDSVGGenerator(const VSDSVGGenerator &);
  VSDSVGGenerator &operator=(const VSDSVGGenerator &);
  VSDSVGGeneratorPrivate *m_pImpl;
};

} // namespace libvisio

#endif // __VISIOSVGGENERATOR_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
