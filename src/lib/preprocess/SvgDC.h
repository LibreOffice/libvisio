#ifndef _SVG_DC_H_INCLUDED_
#define _SVG_DC_H_INCLUDED_


#include <string>
#include <vector>


class SvgFontC;

class SvgDC
{
public:
  virtual ~SvgDC() {}

  virtual const SvgFontC *GetFont(
    double heightInch, unsigned int weight, bool isItalic, const std::string &name) const = 0;

  virtual double GetFontBaseLineHeightRatio(unsigned int fontId) const = 0;

  virtual double GetTextPartialExtents(
    const std::wstring &text, unsigned int fontId, double offsetInch,
    std::vector<double> &extentsInch) const = 0;

  virtual double GetTabCharExtent(unsigned int fontId, double offsetInch) const = 0;
};


extern SvgDC &GetSystemDC();


#endif // _SVG_DC_H_
