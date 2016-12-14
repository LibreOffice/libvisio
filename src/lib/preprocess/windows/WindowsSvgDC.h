#ifdef WIN32 // Windows API-dependent unit


#ifndef _WINDOWS_SVG_DC_H_INCLUDED_
#define _WINDOWS_SVG_DC_H_INCLUDED_

#include "SvgDC.h"
#include "SvgFont.h"

#include <set>
#include <map>
#include <Windows.h>


class WindowsSvgFontC;

class WindowsSvgDC : public SvgDC
{
public:
  WindowsSvgDC(HDC hDC = NULL);
  ~WindowsSvgDC();

  const SvgFontC *GetFont(
    double heightInch, unsigned int weight, bool isItalic, const std::string &name) const;

  double GetFontBaseLineHeightRatio(unsigned int fontId) const;

  double GetTextPartialExtents(
    const std::wstring &text, unsigned int fontId, double offsetInch,
    std::vector<double> &extentsInch) const;

  double GetTabCharExtent(unsigned int fontId, double offsetInch) const;

private:
  typedef std::set<WindowsSvgFontC *, SvgFontCompareS> FontsTp;
  typedef FontsTp::iterator FontsItTp;

  typedef std::map<unsigned int, WindowsSvgFontC *> FontIdsTp;
  typedef FontIdsTp::iterator FontIdsItTp;

  bool GetFontMetric(unsigned int fontId, TEXTMETRIC &metric) const;
  int GetPixelsPerInchX() const;
  int GetPixelsPerInchY() const;
  double PixelsToInches(unsigned int pixels, bool horizontal) const;
  unsigned int InchesToPixels(double inches, bool horizontal) const;
  double GetConversionPrecisionFactor() const;
  unsigned int GetNextFontId() const;

  HDC m_hDC;
  bool m_ownDC;

  mutable int m_pixelPerInchX;
  mutable int m_pixelPerInchY;
  mutable FontsTp m_fonts;
  mutable FontIdsTp m_fontIds;
};


#endif // _WINDOWS_SVG_DC_H_INCLUDED_

#endif WIN32 // Windows API-dependent unit
