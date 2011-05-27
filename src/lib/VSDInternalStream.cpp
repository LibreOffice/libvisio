/* libvisio
 * Copyright (C) 2011 Fridrich Strba (fridrich.strba@bluewin.ch)
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include "VSDInternalStream.h"


VSDInternalStream::VSDInternalStream(WPXInputStream *input, unsigned long size, bool compressed) :
	WPXInputStream(),
	m_offset(0),
	m_size(size),
	m_buffer()
{
	unsigned long tmpNumBytesRead = 0;

	const unsigned char *tmpBuffer = input->read(size, tmpNumBytesRead);
	
	if (size != tmpNumBytesRead)
		return;
	
	if (!compressed)
	{
		for (unsigned long i=0; i<size; i++)
			m_buffer.push_back(tmpBuffer[i]);
	}
	else
	{
		unsigned char buffer[4096] = { 0 };
		unsigned pos = 0;
		unsigned long i = 0;
		unsigned offset = 0;

		while (i < size)
		{
			unsigned flag = tmpBuffer[offset++];
			i++;

			unsigned mask = 1;
			for (unsigned bit = 0; bit < 8; ++bit)
			{
				if (flag & mask)
				{
					buffer[pos&4095] = tmpBuffer[offset++];
					m_buffer.push_back(buffer[pos&4095]);
					pos++;
					i++;
				}
				else
				{
					if (offset > size-2)
						break;
					unsigned char addr1 = tmpBuffer[offset++];
					unsigned char addr2 = tmpBuffer[offset++];

					unsigned length = (addr2&15) + 3;
					unsigned pointer = (((unsigned)addr2 & 0xF0) << 4) | addr1;
					i += 2;
					if (pointer > 4078)
						pointer -= 4078;
					else
						pointer += 18;
        
					for (unsigned j = 0; j < length; ++j)
					{
						buffer[(pos+j) & 4095] = buffer[(pointer+j) & 4095];
						m_buffer.push_back(buffer[(pointer+j) & 4095]);
					}
					pos += length;
        
				}

				mask = mask << 1;
			}
		}
    m_size = m_buffer.size();
	}
}

const unsigned char * VSDInternalStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
	numBytesRead = 0;

	if (numBytes == 0)
		return 0;
	
	int numBytesToRead;

	if ((m_offset+numBytes) < m_size)
		numBytesToRead = numBytes;
	else
		numBytesToRead = m_size - m_offset;
	
	numBytesRead = numBytesToRead; // about as paranoid as we can be..

	if (numBytesToRead == 0)
		return 0;

	long oldOffset = m_offset;
	m_offset += numBytesToRead;
	
	return &m_buffer[oldOffset];
}

int VSDInternalStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
	if (seekType == WPX_SEEK_CUR)
		m_offset += offset;
	else if (seekType == WPX_SEEK_SET)
		m_offset = offset;

	if (m_offset < 0)
	{
		m_offset = 0;
		return 1;
	}
	if ((long)m_offset > (long)m_size)
	{
		m_offset = m_size;
		return 1;
	}

	return 0;
}

long VSDInternalStream::tell()
{
	return m_offset;
}

bool VSDInternalStream::atEOS()
{
	if ((long)m_offset == (long)m_size) 
		return true; 

	return false;
}
