/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>

using namespace librevenge;
using namespace libvisio;
using namespace std;


#ifndef VERSION
#define VERSION "UNKNOWN VERSION"
#endif


namespace
{

string UnquoteString(const string &str)
{
  unsigned int strSize = str.size();
  unsigned int pos1 = strSize > 0 && str[0] == '"' ? 1 : 0;
  unsigned int pos2 = strSize > 1 && str[strSize - 1] == '"' ? strSize - 1 : strSize;
  return str.substr(pos1, pos2 - pos1);
}

bool ExpandFileNameList(const string &listFileName, vector<string> &fileNames)
{
  if (listFileName.find(".lst") == listFileName.size() - 4)
  {
    ifstream in(listFileName);

    if (in.is_open())
    {
      while (in && !in.eof())
      {
        string line;
        getline(in, line);

        if (in)
        {
          fileNames.push_back(UnquoteString(line));
        }
      }

      return true;
    }
  }

  return false;
}

void WriteXhtmlHeader(ostream &out)
{
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
  out << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << endl;
  out << "<body>" << endl;
  out << "<?import namespace=\"svg\" urn=\"http://www.w3.org/2000/svg\"?>" << endl;
}

void WriteXhtmlFooter(ostream &out)
{
  out << "</body>" << endl;
  out << "</html>" << endl;
}

void WriteSvg(const char *svgStr, ostream &out)
{
  out << "<!-- " << endl;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
  out << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"";
  out << " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
  out << " -->"  << endl;
  out << svgStr << endl;
}

int printUsage()
{
  cout << "'vsd2xhtml' converts Microsoft Visio documents to SVG." << endl;
  cout << endl;
  cout << "Usage: vsd2xhtml [OPTION] INPUT {INPUT}" << endl;
  cout << endl;
  cout << "OPTIONS:" << endl;
  cout << "    --help       show this help message" << endl;
  cout << "    --version    show version information" << endl;
  cout << "    --onedoc     merge all visio pages into one output document" << endl;
  cout << endl;
  cout << "INPUT:           VSD file path or a path to a text file containing list" << endl;
  cout << "                 of VSD file paths, each on a separate line" << endl;
  cout << endl;
  cout << "Report bugs to <https://bugs.documentfoundation.org/>." << endl;
  return -1;
}

int printVersion()
{
  cout << "vsd2xhtml " << VERSION << endl;
  return 0;
}

} // anonymous namespace

/**
 * Carries out conversion of specified VSD files in SVG.
 *
 * @param argc
 *        number of supplied arguments
 *
 * @param argv
 *        an array of arguments:
 *
 *        --version     will return the version of this tool
 *
 *        --onedoc      all pages of a particular VSD document will be included in a single SVG file,
 *                      otherwise a SVG file is generated for each page, while appending the page
 *                      number to its name
 *
 *         [fileName]+  a name of the VSD file to convert or a name of a file with LST extension that
 *                      contains a list of paths to VSD files to process, each on a separate line.
 *                      Generated output documents will have the same name as the respective input but
 *                      SVG extension, plus the name will be appended by the page number if --onedoc
 *                      option was not specified
 *
 * @return 0 if conversion (all conversions) was successful
 *
 *         1 if some (all conversion) failed for any reason, such as because of not recognized
 *           format of the input file or parsing failure due to invalid contents
 *
 *        -1 if invalid command line arguments were supplied
 */
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    return printUsage();
  }

  vector<string> fileNames;
  bool oneDoc = false;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--version") == 0)
    {
      return printVersion();
    }
    else if (strcmp(argv[i], "--onedoc") == 0)
    {
      oneDoc = true;
    }
    else if (strncmp(argv[i], "--", 2) != 0)
    {
      string fileName = UnquoteString(argv[i]);

      if (!ExpandFileNameList(fileName, fileNames))
      {
        fileNames.push_back(fileName);
      }
    }
    else
    {
      return printUsage();
    }
  }

  if (fileNames.size() == 0)
  {
    return printUsage();
  }

  bool wasError = false;

  for (unsigned int i = 0; i < fileNames.size(); i++)
  {
    cout << "#" << (i + 1) << "/" << fileNames.size() << endl;
    cout << "Parsing \"" << fileNames[i] << "\"...";

    {
      ifstream in(fileNames[i].c_str());

      if (!in.is_open())
      {
        cerr << "ERROR: file not found" << endl;
        wasError = true;
        continue;
      }
    }

    RVNGFileStream input(fileNames[i].c_str());

    if (!VisioDocument::isSupported(&input))
    {
      cerr << "ERROR: Unsupported file format (unsupported version) or file is encrypted!" << endl;
      wasError = true;
      continue;
    }

    RVNGStringVector output;
    RVNGSVGDrawingGenerator generator(output, ""); // do not use SVG namespace

    if (!VisioDocument::parse(&input, &generator))
    {
      cerr << "ERROR: SVG Generation failed!" << endl;
      wasError = true;
      continue;
    }

    if (output.empty())
    {
      cerr << "ERROR: No SVG document generated!" << endl;
      wasError = true;
      continue;
    }

    cout << "done" << endl;

    string fileName = fileNames[i];
    boost::algorithm::to_lower(fileName);

    fileName = fileName.find(".vsd") == fileName.size() - 4
               ? fileNames[i].substr(0, fileName.size() - 4) : fileNames[i];

    if (oneDoc) // render all pages to one file
    {
      fileName += ".svg";
      ofstream out(fileName.c_str());

      cout << "Writing \"" << fileName.c_str() << "\"...";
      WriteXhtmlHeader(out);

      for (unsigned k = 0; k < output.size(); k++)
      {
        if (k > 0)
        {
          out << "<hr/>" << endl;
        }

        WriteSvg(output[k].cstr(), out);
      }

      WriteXhtmlFooter(out);
      cout << "done" << endl;
    }
    else // render each page to a separate file
    {
      for (unsigned k = 0; k < output.size(); k++)
      {
        ostringstream oss;
        oss << fileName << "-" << (k + 1) << ".svg";
        ofstream out(oss.str().c_str());

        cout << "Writing \"" << oss.str().c_str() << "\"...";
        WriteXhtmlHeader(out);
        WriteSvg(output[k].cstr(), out);
        WriteXhtmlFooter(out);
        cout << "done" << endl;
      }
    }

    cout << endl;
  }

  return wasError ? 1 : 0;
}
