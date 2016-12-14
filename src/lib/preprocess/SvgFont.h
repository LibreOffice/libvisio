#ifndef _SVG_FONT_H_INCLUDED_
#define _SVG_FONT_H_INCLUDED_


#include <string>


class SvgFontC
{
public:
  SvgFontC(
    unsigned int id, double heightInch, unsigned int weight, bool isItalic,
    const std::string &faceName);

  virtual ~SvgFontC() {}

  unsigned int GetId() const;
  double GetHeightInch() const;
  unsigned int GetWeight() const;
  bool IsItalic() const;
  std::string GetFaceName() const;

protected:
  unsigned int m_id;
  double m_heightInch;
  unsigned int m_weight;
  bool m_isItalic;
  const std::string m_faceName;
};


struct SvgFontCompareS
{
  bool operator()(const SvgFontC *pFont1, const SvgFontC *pFont2) const;
};


#endif // _SVG_FONT_H_
