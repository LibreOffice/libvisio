#include "librevenge/SvgConstants.h"
#include "SvgParagraph.h"
#include "SvgUtils.h"
#include "VSDOutputElementList.h"

#include <assert.h>

using namespace librevenge;
using namespace libvisio;
using namespace std;
using namespace svgconstants;


ParagraphC::ParagraphC()
  : m_margLeftInch(0.0),
    m_margRightInch(0.0),
    m_margTopInch(0.0),
    m_margBottomInch(0.0),
    m_lineHeight(1.0),
    m_isLineHeightRel(true)
{
}

double ParagraphC::GetMarginLeftInch() const
{
  return m_margLeftInch;
}

double ParagraphC::GetMarginRightInch() const
{
  return m_margRightInch;
}

double ParagraphC::GetMarginTopInch() const
{
  return m_margTopInch;
}

double ParagraphC::GetMarginBottomInch() const
{
  return m_margBottomInch;
}

double ParagraphC::GetLineHeightInch(double fontSizeInch) const
{
  if (m_isLineHeightRel)
  {
    return fontSizeInch * m_lineHeight;
  }

  return m_lineHeight;
}

void ParagraphC::Reset(const VSDOpenParagraphOutputElement *pOpenParagraph)
{
  Reset(pOpenParagraph->GetPropertyList());
}

void ParagraphC::Reset(const VSDOpenListElementOutputElement *pOpenListElem)
{
  Reset(pOpenListElem->GetPropertyList());
}

void ParagraphC::Reset(const RVNGPropertyList &properties)
{
  m_margLeftInch = properties[PROP_FO_MARGIN_LEFT]
                   ? SvgUtilsC::GetInchValue(*properties[PROP_FO_MARGIN_LEFT]) : 0.0;

  m_margRightInch = properties[PROP_FO_MARGIN_RIGHT]
                    ? SvgUtilsC::GetInchValue(*properties[PROP_FO_MARGIN_RIGHT]) : 0.0;

  m_margTopInch = properties[PROP_FO_MARGIN_TOP]
                  ? SvgUtilsC::GetInchValue(*properties[PROP_FO_MARGIN_TOP]) : 0.0;

  m_margBottomInch = properties[PROP_FO_MARGIN_BOTTOM]
                     ? SvgUtilsC::GetInchValue(*properties[PROP_FO_MARGIN_BOTTOM]) : 0.0;

  m_horizontalAlignment = properties[PROP_FO_TEXT_ALIGN]
                          ? (*properties[PROP_FO_TEXT_ALIGN]).getStr().cstr() : PVAL_FO_TEXT_ALIGN_CENTER;

  if (properties[PROP_FO_LINE_HEIGHT])
  {
    switch (properties[PROP_FO_LINE_HEIGHT]->getUnit())
    {
    case RVNG_UNIT_ERROR :
      m_lineHeight = 1.0;
      m_isLineHeightRel = true;
      break;

    case RVNG_PERCENT :
      m_lineHeight = properties[PROP_FO_LINE_HEIGHT]->getDouble();
      m_isLineHeightRel = true;
      break;

    default :
      m_lineHeight = SvgUtilsC::GetInchValue(*properties[PROP_FO_LINE_HEIGHT]);
      m_isLineHeightRel = false;
      break;
    }
  }
  else
  {
    m_lineHeight = 1.0;
    m_isLineHeightRel = true;
  }

  m_text.clear();
  m_textExtents.clear();
  m_spanMarks.clear();
}

void ParagraphC::AddSpan(const VSDOpenSpanOutputElement *pSpan)
{
  m_spanMarks[m_text.size()] = pSpan;
}

void ParagraphC::AddText(const wstring &text, const TextExtentsTp &extents)
{
  assert(text.size() == extents.size());
  m_text += text;
  m_textExtents.insert(m_textExtents.end(), extents.begin(), extents.end());
}

double ParagraphC::GetCurrentTextExtentInch() const
{
  if (m_textExtents.size() > 0)
  {
    return m_textExtents[m_textExtents.size() - 1];
  }

  return 0;
}

const string &ParagraphC::GetHorizontalAlignment() const
{
  return m_horizontalAlignment;
}

const wstring &ParagraphC::GetText() const
{
  return m_text;
}

const ParagraphC::TextExtentsTp &ParagraphC::GetTextExtents() const
{
  return m_textExtents;
}

const ParagraphC::SpanMarksTp &ParagraphC::GetSpanMarks() const
{
  return m_spanMarks;
}
