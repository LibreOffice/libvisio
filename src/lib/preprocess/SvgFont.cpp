#include "SvgFont.h"

using namespace std;


SvgFontC::SvgFontC(
  unsigned int id, double heightInch, unsigned int weight, bool isItalic, const string &faceName
)
  : m_id(id),
    m_heightInch(heightInch),
    m_weight(weight),
    m_isItalic(isItalic),
    m_faceName(faceName)
{
}

unsigned int SvgFontC::GetId() const
{
  return m_id;
}

double SvgFontC::GetHeightInch() const
{
  return m_heightInch;
}

unsigned int SvgFontC::GetWeight() const
{
  return m_weight;
}

bool SvgFontC::IsItalic() const
{
  return m_isItalic;
}

string SvgFontC::GetFaceName() const
{
  return m_faceName;
}


bool SvgFontCompareS::operator()(const SvgFontC *pFont1, const SvgFontC *pFont2) const
{
  if (pFont1->GetHeightInch() != pFont2->GetHeightInch())
  {
    return pFont1->GetHeightInch() < pFont2->GetHeightInch();
  }

  if (pFont1->GetWeight() != pFont2->GetWeight())
  {
    return pFont1->GetWeight() < pFont2->GetWeight();
  }

  if (pFont1->IsItalic() != pFont2->IsItalic())
  {
    return !pFont1->IsItalic();
  }

  return pFont1->GetFaceName().compare(pFont2->GetFaceName()) < 0;
}
