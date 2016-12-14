#ifdef WIN32 // Windows API-dependent unit


#include "WindowsSvgFont.h"

using namespace std;


WindowsSvgFontC::WindowsSvgFontC(
  unsigned int id, double heightInch, unsigned int weight, bool isItalic, const string &faceName
)
  : SvgFontC(id, heightInch, weight, isItalic, faceName),
    m_hFont(NULL)
{
}

WindowsSvgFontC::~WindowsSvgFontC()
{
  DeleteObject(m_hFont);
}

bool WindowsSvgFontC::Create(double pixelsPerInch)
{
  LOGFONT logFont;
  logFont.lfHeight = -static_cast<int>(m_heightInch * pixelsPerInch);
  logFont.lfWidth = 0;
  logFont.lfEscapement = 0;
  logFont.lfOrientation = 0;
  logFont.lfWeight = m_weight;
  logFont.lfItalic = m_isItalic;
  logFont.lfUnderline = false;
  logFont.lfStrikeOut = false;
  logFont.lfCharSet = DEFAULT_CHARSET;
  logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  logFont.lfQuality = DEFAULT_QUALITY;
  logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  strncpy(logFont.lfFaceName, m_faceName.c_str(), LF_FACESIZE - 1);
  m_hFont = CreateFontIndirect(&logFont);
  return m_hFont != NULL;
}

WindowsSvgFontC::operator HFONT()
{
  return m_hFont;
}

WindowsSvgFontC::operator HFONT() const
{
  return m_hFont;
}


#endif WIN32 // Windows API-dependent unit
