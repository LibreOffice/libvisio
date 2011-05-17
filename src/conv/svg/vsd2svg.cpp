/* libvisio
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libvisio.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "libvisio.h"
#include <libwpd-stream/libwpd-stream.h>
#include <libwpd/libwpd.h>

namespace {

int printUsage()
{
	printf("Usage: vsd2svg [OPTION] <WordPerfect Graphics File>\n");
	printf("\n");
	printf("Options:\n");
	printf("--help                Shows this help message\n");
	return -1;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
	if (argc < 2)
		return printUsage();
	
	char *file = 0;

	for (int i = 1; i < argc; i++)
	{
		if (!file && strncmp(argv[i], "--", 2))
			file = argv[i];
		else
			return printUsage();
	}

	if (!file)
		return printUsage();
		
	WPXFileStream input(file);

	if (!libvisio::VisioDocument::isSupported(&input))
	{
		std::cerr << "ERROR: Unsupported file format (unsupported version) or file is encrypted!" << std::endl;
		return 1;
	}

	::WPXString output;
	if (!libvisio::VisioDocument::generateSVG(&input, output))
	{
		std::cerr << "ERROR: SVG Generation failed!" << std::endl;
		return 1;
	}		

	std::cout << output.cstr() << std::endl;
	return 0;
}
