#ifdef WIN32 // Windows API-dependent unit


#include "SvgUtils.h"
#include "WindowsSvgDC.h"
#include "WindowsSvgFont.h"

using namespace std;


SvgDC &GetSystemDC()
{
  static WindowsSvgDC dc;
  return dc;
}


WindowsSvgDC::WindowsSvgDC(HDC hDC)
  : m_pixelPerInchX(-1),
    m_pixelPerInchY(-1)
{
  if (hDC == NULL)
  {
    m_ownDC = true;
    m_hDC = GetWindowDC(NULL);
  }
  else
  {
    m_ownDC = false;
  }
}

WindowsSvgDC::~WindowsSvgDC()
{
  if (m_ownDC)
  {
    ReleaseDC(NULL, m_hDC);
  }

  for (FontsItTp it = m_fonts.begin(); it != m_fonts.end(); it++)
  {
    delete *it;
  }
}

double WindowsSvgDC::GetConversionPrecisionFactor() const
{
  return 1000.0;
}

double WindowsSvgDC::PixelsToInches(unsigned int pixels, bool horizontal) const
{
  return static_cast<double>(pixels)
         / (horizontal ? GetPixelsPerInchX() : GetPixelsPerInchY());
}

unsigned int WindowsSvgDC::InchesToPixels(double inches, bool horizontal) const
{
  return static_cast<unsigned int>(SvgUtilsC::Round(inches
                                                    * (horizontal ? GetPixelsPerInchX() : GetPixelsPerInchY())));
}

int WindowsSvgDC::GetPixelsPerInchX() const
{
  if (m_pixelPerInchX < 0)
  {
    m_pixelPerInchX =
      SvgUtilsC::Round(GetConversionPrecisionFactor() * GetDeviceCaps(m_hDC, LOGPIXELSX));
  }

  return m_pixelPerInchX;
}

int WindowsSvgDC::GetPixelsPerInchY() const
{
  if (m_pixelPerInchY < 0)
  {
    m_pixelPerInchY =
      SvgUtilsC::Round(GetConversionPrecisionFactor() * GetDeviceCaps(m_hDC, LOGPIXELSY));
  }

  return m_pixelPerInchY;
}

const SvgFontC *WindowsSvgDC::GetFont(
  double heightInch, unsigned int weight, bool isItalic, const string &name) const
{
  WindowsSvgFontC font(0, heightInch, weight, isItalic, name);
  FontsItTp it = m_fonts.find(&font);
  WindowsSvgFontC *pFont;

  if (it == m_fonts.end())
  {
    pFont = new WindowsSvgFontC(GetNextFontId(), heightInch, weight, isItalic, name);
    pFont->Create(GetPixelsPerInchX());
    m_fonts.insert(pFont);
    m_fontIds[pFont->GetId()] = pFont;
  }
  else
  {
    pFont = *it;
  }

  return pFont;
}

double WindowsSvgDC::GetFontBaseLineHeightRatio(unsigned int fontId) const
{
  TEXTMETRIC tm;
  GetFontMetric(fontId, tm);
  return static_cast<double>(tm.tmAscent) / tm.tmHeight;
}

bool WindowsSvgDC::GetFontMetric(unsigned int fontId, TEXTMETRIC &metric) const
{
  FontIdsItTp it = m_fontIds.find(fontId);

  if (it == m_fontIds.end())
  {
    return false;
  }

  HGDIOBJ oldObj = SelectObject(m_hDC, *it->second);
  BOOL result = GetTextMetrics(m_hDC, &metric);
  SelectObject(m_hDC, oldObj);

  return result == TRUE;
}

double WindowsSvgDC::GetTextPartialExtents(
  const wstring &text, unsigned int fontId, double offsetInch, vector<double> &extentsInch) const
{
  unsigned int textLen = text.length();

  if (textLen > 0)
  {
    FontIdsItTp it = m_fontIds.find(fontId);

    if (it == m_fontIds.end())
    {
      return -1.0;
    }

    HGDIOBJ oldObj = SelectObject(m_hDC, *it->second);

    vector<int> extentsPx;
    extentsPx.resize(textLen);

    SIZE strSize;
    GetTextExtentExPointW(m_hDC, text.c_str(), textLen, 0, NULL, &extentsPx.front(), &strSize);

    extentsInch.resize(textLen);

    for (unsigned int i = 0; i < textLen; i++)
    {
      extentsInch[i] = PixelsToInches(extentsPx[i], true) + offsetInch;
    }

    SelectObject(m_hDC, oldObj);
    return offsetInch + PixelsToInches(strSize.cx, true);
  }

  return offsetInch;
}

double WindowsSvgDC::GetTabCharExtent(unsigned int fontId, double offsetInch) const
{
  FontIdsItTp it = m_fontIds.find(fontId);

  if (it == m_fontIds.end())
  {
    return -1.0;
  }

  HGDIOBJ oldObj = SelectObject(m_hDC, *it->second);
  DWORD extent = GetTabbedTextExtentW(m_hDC, L"\t", 1, 0, NULL);
  SelectObject(m_hDC, oldObj);
  return PixelsToInches(extent & 0xFFFF, true) + offsetInch;
}

unsigned int WindowsSvgDC::GetNextFontId() const
{
  static unsigned int id = 0;
  return ++id;
}


#endif // Windows API-dependent unit
