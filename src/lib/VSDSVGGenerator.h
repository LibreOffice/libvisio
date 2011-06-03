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

#ifndef __VISIOSVGGENERATOR_H__
#define __VISIOSVGGENERATOR_H__

#include <stdio.h>
#include <iostream>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

namespace libvisio
{

class VSDSVGGenerator : public libwpg::WPGPaintInterface {
public:
	VSDSVGGenerator(std::ostream & output_sink);
	~VSDSVGGenerator();

	void startGraphics(const ::WPXPropertyList &propList);
	void endGraphics();
	void startLayer(const ::WPXPropertyList& propList);
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
	::WPXPropertyListVector m_gradient;
	::WPXPropertyList m_style;
	int m_gradientIndex;
	bool m_isFirstPage;
	void writeStyle(bool isClosed=true);
	void drawPolySomething(const ::WPXPropertyListVector& vertices, bool isClosed);

	std::ostream & m_outputSink;
};

} // namespace libvisio

#endif // __VISIOSVGGENERATOR_H__
