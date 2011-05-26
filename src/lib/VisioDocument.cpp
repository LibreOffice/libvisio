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

#include <libwpd-stream/libwpd-stream.h>
#include <sstream>
#include <string>
#include "libvisio.h"
#include "VSDSVGGenerator.h"
#include "VSDXParser.h"


/**
Analyzes the content of an input stream to see if it can be parsed
\param input The input stream
\return A value that indicates whether the content from the input
stream is a Visio Document that libvisio able to parse
*/
bool libvisio::VisioDocument::isSupported(WPXInputStream* input)
{
  WPXInputStream* tmpDocStream = input->getDocumentOLEStream("VisioDocument");
  if (!tmpDocStream)
  {
    return false;
  }

  tmpDocStream->seek(0x1A, WPX_SEEK_SET);
  unsigned long bytesRead;
  const unsigned char *data = tmpDocStream->read(1, bytesRead);

  if (bytesRead != 1)
  {
    delete [] data;
    return false;
  }

  unsigned char version = data[0];
  delete [] data;

  // Versions 2k (6) and 2k3 (11)
  if (version == 6 || version == 11)
  {
    return true;
  }

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
  WPXInputStream* docStream = input->getDocumentOLEStream("VisioDocument");
  if (!docStream)
  {
    return false;
  }

  docStream->seek(0x1A, WPX_SEEK_SET);
  unsigned long bytesRead;
  const unsigned char *data = docStream->read(1, bytesRead);

  if (bytesRead != 1)
  {
    delete [] data;
    delete docStream;
    return false;
  }

  unsigned char version = data[0];
  VSDXParser* parser;
  switch(version)
  {
  case 6: parser = new VSD6Parser(docStream); break;
  case 11: parser = new VSD11Parser(docStream); break;
  default: return false;
  }
  if (parser)
    parser->parse(painter);

	return true;
}

/**
Parses the input stream content and generates a valid Scalable Vector Graphics
Provided as a convenience function for applications that support SVG internally.
\param input The input stream
\param output The output string whose content is the resulting SVG
\return A value that indicates whether the SVG generation was successful.
*/
bool libvisio::VisioDocument::generateSVG(::WPXInputStream* input, WPXString& output)
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
