/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2007 Fridrich Strba (fridrich.strba@bluewin.ch)
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
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <libwpd/libwpd-stream.h>
#include "VisioDocument.h"


/**
Analyzes the content of an input stream to see if it can be parsed
\param input The input stream
\return A value that indicates whether the content from the input
stream is a Visio Document that libvisio able to parse
*/
bool libvisio::VisioDocument::isSupported(WPXInputStream* input)
{
	return false;
}

/**
Parses the input stream content. It will make callbacks to the functions provided by a
WPGPaintInterface class implementation when needed. This is often commonly called the
'main parsing routine'.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
bool libvisio::VisioDocument::parse(::WPXInputStream* input, libwpg::WPGPaintInterface* painter)
{
	return false;
}

bool libvisio::VisioDocument::parse(const unsigned char* data, unsigned long size, libwpg::WPGPaintInterface* painter)
{
	WPXStringStream tmpStream(data, size);
	return libvisio::VisioDocument::parse(&tmpStream, painter);
} 
/**
Parses the input stream content and generates a valid Scalable Vector Graphics
Provided as a convenience function for applications that support SVG internally.
\param input The input stream
\param output The output string whose content is the resulting SVG
\return A value that indicates whether the SVG generation was successful.
*/
bool libvisio::VisioDocument::generateSVG(::WPXInputStream* input, WPXString& output, libwpg::WPGFileFormat fileFormat)
{
	std::ostringstream tmpOutputStream;
	libvisio::VSDSVGGenerator generator(tmpOutputStream);
	bool result = libvisio::VisioDocument::parse(input, &generator);
	if (result)
		output = WPXString(tmpOutputStream.str().c_str());
	else
		output = WPXString("");
	return result;
}

bool libvisio::VisioDocument::generateSVG(const unsigned char* data, unsigned long size, WPXString& output)
{
	WPXStringStream tmpStream(data, size);
	return libvisio::VisioDocument::generateSVG(&tmpStream, output);
}
