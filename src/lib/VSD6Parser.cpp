/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
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
#include <locale.h>
#include <sstream>
#include <string>
#include "libvisio_utils.h"
#include "VSD6Parser.h"
#include "VSDInternalStream.h"
#include "VSDXDocumentStructure.h"

const libvisio::VSD6Parser::StreamHandler libvisio::VSD6Parser::handlers[] = {
  {VSD_PAGE, "Page", &libvisio::VSD6Parser::handlePage},
  {VSD_PAGES, "Pages", &libvisio::VSD6Parser::handlePages},
  {0, 0, 0}
};

libvisio::VSD6Parser::VSD6Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDXParser(input, painter)
{}

libvisio::VSD6Parser::~VSD6Parser()
{}

/** Parses VSD 2000 input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSD6Parser::parse()
{
  const unsigned int SHIFT = 4;
  if (!m_input)
  {
    return false;
  }
  // Seek to trailer stream pointer
  m_input->seek(0x24, WPX_SEEK_SET);

  m_input->seek(8, WPX_SEEK_CUR);
  unsigned int offset = readU32(m_input);
  unsigned int length = readU32(m_input);
  unsigned short format = readU16(m_input);
  bool compressed = ((format & 2) == 2);

  m_input->seek(offset, WPX_SEEK_SET);
  VSDInternalStream trailerStream(m_input, length, compressed);

  // Parse out pointers to other streams from trailer
  trailerStream.seek(SHIFT, WPX_SEEK_SET);
  offset = readU32(&trailerStream);
  trailerStream.seek(offset+SHIFT, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(&trailerStream);
  trailerStream.seek(SHIFT, WPX_SEEK_CUR);

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(&trailerStream);
    trailerStream.seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(&trailerStream);
    ptrLength = readU32(&trailerStream);
    ptrFormat = readU16(&trailerStream);

    int index = -1;
    for (int j = 0; (index < 0) && handlers[j].type; j++)
    {
      if (handlers[j].type == ptrType)
        index = j;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in trailer at %li\n",
                     ptrType, trailerStream.tell() - 18));
    }
    else
    {
      Method streamHandler = handlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       trailerStream.tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       trailerStream.tell() - 18));

        compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        WPXInputStream *input = new VSDInternalStream(m_input, ptrLength, compressed);
        (this->*streamHandler)(input);
        delete input;
      }
    }
  }

  // End page if one is started
  if (m_isPageStarted)
    m_painter->endGraphics();
  return true;
}

void libvisio::VSD6Parser::handlePages(WPXInputStream *input)
{
  unsigned int offset = readU32(input);
  input->seek(offset, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    int index = -1;
    for (int j = 0; (index < 0) && handlers[j].type; j++)
    {
      if (handlers[j].type == ptrType)
        index = j;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in pages at %li\n",
                     ptrType, input->tell() - 18));
    }
    else
    {
      Method streamHandler = handlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       input->tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       input->tell() - 18));

        bool compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        WPXInputStream *tmpInput = new VSDInternalStream(m_input, ptrLength, compressed);
        (this->*streamHandler)(tmpInput);
        delete tmpInput;
      }
    }
  }
}

void libvisio::VSD6Parser::handlePage(WPXInputStream *input)
{
  WPXPropertyList pageProps;
  XForm xform; // Tracks current xform data
  unsigned int foreignType = 0; // Tracks current foreign data type
  unsigned int foreignFormat = 0; // Tracks foreign data format
  unsigned long tmpBytesRead = 0;

  while (!input->atEOS())
  {
    unsigned int chunkType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip id field

    // Certain chunk types seem to always have a trailer
    unsigned int list = readU32(input);
    unsigned int trailer = 0;
    if (list != 0 || chunkType == 0x71 || chunkType == 0x70 ||
        chunkType == 0x6b || chunkType == 0x6a || chunkType == 0x69 || 
        chunkType == 0x66 || chunkType == 0x65 || chunkType == 0x2c)
      trailer += 8; // 8 byte trailer

    unsigned int dataLength = readU32(input);
    input->seek(3, WPX_SEEK_CUR); // Skip level (word) and ? (byte) fields

    VSD_DEBUG_MSG(("Parsing chunk type %02x with trailer (%d) and length %x\n",
                   chunkType, trailer, dataLength));

    if (chunkType == VSD_XFORM_DATA) // XForm data
    {
      input->seek(1, WPX_SEEK_CUR);
      xform.pinX = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.pinY = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.width = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.height = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.pinLocX = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.pinLocY = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      xform.angle = readDouble(input);

      xform.flipX = (readU8(input) != 0);
      xform.flipY = (readU8(input) != 0);

      input->seek(dataLength+trailer-65, WPX_SEEK_CUR);
    }
    else if (chunkType == VSD_FOREIGN_DATA) // Foreign data (binary)
    {
      if (foreignType == 1 || foreignType == 4) // Image
      {
        const unsigned char *buffer = input->read(dataLength, tmpBytesRead);
        WPXBinaryData binaryData;

        if (foreignType == 1)
        {        
          // v6 always uses bmp for images which needs header reconstruction
          binaryData.append(0x42);
          binaryData.append(0x4d);

          binaryData.append((unsigned char)((tmpBytesRead + 14) & 0x000000ff));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0x0000ff00) >> 8));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0x00ff0000) >> 16));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0xff000000) >> 24));

          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);

          binaryData.append(0x36);
          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);

        }
        binaryData.append(buffer, tmpBytesRead);

        WPXPropertyList foreignProps;
        foreignProps.insert("svg:width", m_scale*xform.width);
        foreignProps.insert("svg:height", m_scale*xform.height);
        foreignProps.insert("svg:x", m_scale*(xform.pinX - xform.pinLocX));
        // Y axis starts at the bottom not top
        foreignProps.insert("svg:y", m_scale*(pageProps["svg:height"]->getDouble() 
        - xform.pinY + xform.pinLocY - xform.height));

        if (foreignType == 1)
        {
          foreignProps.insert("libwpg:mime-type", "image/bmp");
        }
        else if (foreignType == 4)
        {
          const unsigned char *tmpBinData = binaryData.getDataBuffer();
          // Check for EMF signature
          if (tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
          {
            foreignProps.insert("libwpg:mime-type", "image/emf");
          }
          else
          {
            foreignProps.insert("libwpg:mime-type", "image/wmf");
          }
        }

        m_painter->drawGraphicObject(foreignProps, binaryData);
      }
      else
      {
        input->seek(dataLength+trailer, WPX_SEEK_CUR);
      }
    }
    else if (chunkType == VSD_PAGE_PROPS) // Page properties
    {
      // Skip bytes representing unit to *display* (value is always inches)
      input->seek(1, WPX_SEEK_CUR);
      m_pageWidth = readDouble(input);
      input->seek(1, WPX_SEEK_CUR);
      m_pageHeight = readDouble(input);

      pageProps.insert("svg:width", m_scale*m_pageWidth);
      pageProps.insert("svg:height", m_scale*m_pageHeight);
      if (m_isPageStarted)
        m_painter->endGraphics();
      m_painter->startGraphics(pageProps);
      m_isPageStarted = true;

      input->seek(dataLength+trailer-18, WPX_SEEK_CUR);
    }
    else if (chunkType == VSD_FOREIGN_DATA_TYPE) // Foreign data type
    {
      input->seek(0x24, WPX_SEEK_CUR);
      foreignType = readU16(input);
      input->seek(0xb, WPX_SEEK_CUR);
      foreignFormat = readU32(input);

      input->seek(dataLength+trailer-0x35, WPX_SEEK_CUR);
    }
    else // Skip chunk
    {
      dataLength += trailer;
      input->seek(dataLength, WPX_SEEK_CUR);
    }
  }
}
