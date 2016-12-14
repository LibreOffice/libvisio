#ifndef _SVG_UTILS_H_INCLUDED_
#define _SVG_UTILS_H_INCLUDED_


#include <string>
#include <Windows.h>


namespace librevenge
{
class RVNGProperty;
}


class SvgUtilsC
{
public:
  static int Round(double x);
  static double GetInchValue(const librevenge::RVNGProperty &prop);
  static std::wstring StrToWstr(const std::string &str);
  static std::string WstrToStr(const std::wstring &wstr);

private:
  SvgUtilsC();
};


#endif // _SVG_UTILS_H_INCLUDED_
