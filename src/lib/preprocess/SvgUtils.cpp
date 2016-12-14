#include "SvgUtils.h"
#include <librevenge/librevenge.h>

using namespace librevenge;
using namespace std;


int SvgUtilsC::Round(double x)
{
  if (x < 0.0)
  {
    return static_cast<int>(x - 0.5);
  }

  return static_cast<int>(x + 0.5);
}

double SvgUtilsC::GetInchValue(const RVNGProperty &prop)
{
  double value = prop.getDouble();

  switch (prop.getUnit())
  {
  case RVNG_GENERIC: // assume inch
  case RVNG_INCH:
    return value;

  case RVNG_POINT:
    return value / 72.0;

  case RVNG_TWIP:
    return value / 1440.0;

  default :
    return value; // non-conversible
  }
}

wstring SvgUtilsC::StrToWstr(const string &str)
{
  int wCharCnt = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  wchar_t *wstr = new wchar_t[wCharCnt];

  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, wCharCnt);
  wstring outWstr = wstr;

  delete wstr;
  return outWstr;
}

string SvgUtilsC::WstrToStr(const wstring &wstr)
{
  int charCnt = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
  char *str = new char[charCnt];

  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], charCnt, NULL, NULL);
  string outStr = str;

  delete str;
  return outStr;
}
