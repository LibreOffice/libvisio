#ifdef WIN32 // Windows API-dependent unit

#ifndef _WINDOWS_SVG_FONT_H_INCLUDED_
#define _WINDOWS_SVG_FONT_H_INCLUDED_

#include "SvgFont.h"
#include <Windows.h>


class WindowsSvgFontC : public SvgFontC
{
public:
  WindowsSvgFontC(
    unsigned int id, double heightInch, unsigned int weight, bool isItalic,
    const std::string &faceName);

  ~WindowsSvgFontC();

  bool Create(double pixelsPerInch);
  operator HFONT();
  operator HFONT() const;

private:
  HFONT m_hFont;
};


#endif // _WINDOWS_SVG_FONT_H_INCLUDED_

#endif WIN32 // Windows API-dependent unit
