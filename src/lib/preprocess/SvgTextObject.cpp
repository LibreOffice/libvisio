#include "librevenge/SvgConstants.h"
#include "SvgDC.h"
#include "SvgFont.h"
#include "SvgTextObject.h"
#include "SvgUtils.h"
#include "VSDOutputElementList.h"

#include <assert.h>

using namespace librevenge;
using namespace libvisio;
using namespace std;
using namespace svgconstants;


SvgTextObjectC::SvgTextObjectC(VSDStartTextObjectOutputElement *pStartText, SvgDC &dc)
  : m_dc(dc),
    m_pCurrentFont(NULL),
    m_stage(PS_NONE),
    m_firstLineOffsetYInch(0.0),
    m_currentLineYInch(0.0),
    m_textLeftBoundInch(DBL_MAX),
    m_textRightBoundInch(DBL_MIN)
{
  RVNGPropertyList &props = pStartText->GetPropertyList();

  m_origXInch = props[PROP_SVG_X] ? SvgUtilsC::GetInchValue(*props[PROP_SVG_X]) : 0.0;
  m_origYInch = props[PROP_SVG_Y] ? SvgUtilsC::GetInchValue(*props[PROP_SVG_Y]) : 0.0;
  m_widthInch = props[PROP_SVG_WIDTH] ? SvgUtilsC::GetInchValue(*props[PROP_SVG_WIDTH]) : 0.0;
  m_heightInch = props[PROP_SVG_HEIGHT] ? SvgUtilsC::GetInchValue(*props[PROP_SVG_HEIGHT]) : 0.0;

  m_padLeftInch = props[PROP_FO_PADDING_LEFT]
                  ? SvgUtilsC::GetInchValue(*props[PROP_FO_PADDING_LEFT]) : 0.0;

  m_padRightInch = props[PROP_FO_PADDING_RIGHT]
                   ? SvgUtilsC::GetInchValue(*props[PROP_FO_PADDING_RIGHT]) : 0.0;

  m_padTopInch = props[PROP_FO_PADDING_TOP]
                 ? SvgUtilsC::GetInchValue(*props[PROP_FO_PADDING_TOP]) : 0.0;

  m_padBottomInch = props[PROP_FO_PADDING_BOTTOM]
                    ? SvgUtilsC::GetInchValue(*props[PROP_FO_PADDING_BOTTOM]) : 0.0;

  m_verticalAlign = props[PROP_DRAW_TEXTAREA_VERTICAL_ALIGN] ?
                    props[PROP_DRAW_TEXTAREA_VERTICAL_ALIGN]->getStr().cstr() : PVAL_DRAW_VERTICAL_ALIGN_MIDDLE;

  m_bkgColor = props[PROP_FO_BACKGROUND_COLOR]
               ? props[PROP_FO_BACKGROUND_COLOR]->getStr().cstr() : "";
}

SvgDC &SvgTextObjectC::GetDC()
{
  return m_dc;
}

SvgTextObjectC::ProcessingStageE SvgTextObjectC::GetProcessingStage() const
{
  return m_stage;
}

void SvgTextObjectC::StartCalculationStage()
{
  if (m_stage == PS_NONE)
  {
    m_stage = PS_CALCULATE;
    m_currentLineYInch = 0.0;
    m_firstLineOffsetYInch = 0.0;
  }
}

void SvgTextObjectC::StartLayoutStage()
{
  if (m_stage == PS_CALCULATE)
  {
    m_currentLineYInch = GetCurrentTextBoundTopInch() + m_firstLineOffsetYInch;
    m_stage = PS_LAYOUT;
  }
}

double SvgTextObjectC::GetOrigXInch() const
{
  return m_origXInch;
}

double SvgTextObjectC::GetOrigYInch() const
{
  return m_origYInch;
}

double SvgTextObjectC::GetWidthInch() const
{
  return m_widthInch;
}

double SvgTextObjectC::GetHeightInch() const
{
  return m_heightInch;
}

double SvgTextObjectC::GetPaddingLeftInch() const
{
  return m_padLeftInch;
}

double SvgTextObjectC::GetPaddingRightInch() const
{
  return m_padRightInch;
}

double SvgTextObjectC::GetPaddingTopInch() const
{
  return m_padTopInch;
}

double SvgTextObjectC::GetPaddingBottomInch() const
{
  return m_padBottomInch;
}

double SvgTextObjectC::GetCurrentLineYInch() const
{
  return m_currentLineYInch;
}

double SvgTextObjectC::RowOfTextAdded()
{
  double fontHeightInch = GetCurrentFont()->GetHeightInch();
  double lineHeightInch = m_currentParagraph.GetLineHeightInch(fontHeightInch);

  m_currentLineYInch += lineHeightInch;

  if (m_stage == PS_CALCULATE && m_firstLineOffsetYInch == 0.0)
  {
    m_firstLineOffsetYInch = fontHeightInch
                             * (m_dc.GetFontBaseLineHeightRatio(GetCurrentFont()->GetId()))
                             + (lineHeightInch - fontHeightInch) / 2.0;
  }

  return m_currentLineYInch;
}

void SvgTextObjectC::OpenParagraph(const VSDOpenParagraphOutputElement *pOpenParagraph)
{
  m_currentParagraph.Reset(pOpenParagraph);
}

void SvgTextObjectC::OpenUnorderedList(const VSDOpenUnorderedListLevelOutputElement *pList)
{
  const RVNGPropertyList &props = pList->GetPropertyList();

  m_currentBulletChar = props[PROP_TEXT_BULLET_CHAR]
                        ? SvgUtilsC::StrToWstr(props[PROP_TEXT_BULLET_CHAR]->getStr().cstr()) : L"";
}

void SvgTextObjectC::OpenListElement(const VSDOpenListElementOutputElement *pOpenListElem)
{
  m_currentParagraph.Reset(pOpenListElem);
}

void SvgTextObjectC::OpenSpan(const VSDOpenSpanOutputElement *pOpenSpan)
{
  const RVNGPropertyList &props = pOpenSpan->GetPropertyList();

  double fontSizeInch = props[PROP_FO_FONT_SIZE]
                        ? SvgUtilsC::GetInchValue(*props[PROP_FO_FONT_SIZE]) : 12.0 * POINT_SIZE_INCH;

  unsigned int fontWeight =
    props[PROP_FO_FONT_WEIGHT]
    && strcmp(props[PROP_FO_FONT_WEIGHT]->getStr().cstr(), PVAL_FO_FONT_WEIGHT_BOLD) == 0
    ? FW_BOLD : FW_NORMAL;

  bool isItalic =
    props[PROP_FO_FONT_STYLE]
    && strcmp(props[PROP_FO_FONT_STYLE]->getStr().cstr(), PVAL_FO_FONT_STYLE_ITALIC) == 0;

  string fontName = props[PROP_STYLE_FONT_NAME]
                    ? props[PROP_STYLE_FONT_NAME]->getStr().cstr() : "Arial";

  m_pCurrentFont = m_dc.GetFont(fontSizeInch, fontWeight, isItalic, fontName);
  m_currentParagraph.AddSpan(pOpenSpan);

  if (props[PROP_FO_COLOR])
  {
    // addresses the issue when the text box background is wrongly indicated as 'filled'
    if (!m_bkgColor.empty() && m_bkgColor == props[PROP_FO_COLOR]->getStr().cstr())
    {
      m_bkgColor.clear(); // do not fill the background as (some) text has the same color
    }
  }
}

void SvgTextObjectC::InsertText(const VSDInsertTextOutputElement *pInsertText)
{
  string text = pInsertText->GetText().cstr();
  wstring wtext = SvgUtilsC::StrToWstr(text);
  double curExtent = m_currentParagraph.GetCurrentTextExtentInch();
  ParagraphC::TextExtentsTp textExtents;

  m_dc.GetTextPartialExtents(wtext, GetCurrentFont()->GetId(), curExtent, textExtents);
  m_currentParagraph.AddText(wtext, textExtents);
}

void SvgTextObjectC::InsertTab(const VSDInsertTabOutputElement *pInsertTab)
{
  ParagraphC::TextExtentsTp textExtents;
  double curExtent = m_currentParagraph.GetCurrentTextExtentInch();

  textExtents.push_back(m_dc.GetTabCharExtent(GetCurrentFont()->GetId(), curExtent));
  m_currentParagraph.AddText(L"\t", textExtents);
}

const string &SvgTextObjectC::GetCurrentHorizontalAlignment() const
{
  return m_currentParagraph.GetHorizontalAlignment();
}

const SvgFontC *SvgTextObjectC::GetCurrentFont() const
{
  if (m_pCurrentFont == NULL)
  {
    m_pCurrentFont = m_dc.GetFont(12.0 * POINT_SIZE_INCH, FW_NORMAL, false, "Arial"); // default font if not yet defined
  }

  return m_pCurrentFont;
}

const string &SvgTextObjectC::GetBackgroundColor() const
{
  return m_bkgColor;
}

void SvgTextObjectC::SetCurrentTextLeftBoundInch(double x)
{
  if (x < m_textLeftBoundInch)
  {
    m_textLeftBoundInch = x;
  }
}

void SvgTextObjectC::SetCurrentTextRightBoundInch(double x)
{
  if (x > m_textRightBoundInch)
  {
    m_textRightBoundInch = x;
  }
}

void SvgTextObjectC::GetCurrentTextBoundsInch(
  double &x1, double &y1, double &x2, double &y2) const
{
  x1 = m_textLeftBoundInch;
  x2 = m_textRightBoundInch;
  y1 = GetCurrentTextBoundTopInch();
  y2 = y1 + GetCurrentLineYInch();
}

double SvgTextObjectC::GetCurrentTextBoundTopInch() const
{
  if (m_verticalAlign.compare(PVAL_DRAW_VERTICAL_ALIGN_TOP) == 0)
  {
    return GetOrigYInch() + m_padTopInch;
  }

  if (m_verticalAlign.compare(PVAL_DRAW_VERTICAL_ALIGN_BOTTOM) == 0)
  {
    return GetOrigYInch() + (GetHeightInch() - GetCurrentLineYInch()) - m_padBottomInch;
  }

  return GetOrigYInch() + (GetHeightInch() - GetCurrentLineYInch()) / 2.0;
}

wstring SvgTextObjectC::GetCurrentBulletCharacter() const
{
  return m_currentBulletChar;
}

/**
 * Takes the currently stored paragraph text and its partial extents and breaks it down into spans
 * while adding line breaks when needed, so that each span fits in this text box width.
 *
 * @param spans (out)
 *        a list to be filled (appended) by calculated spans of text
 */
void SvgTextObjectC::CalculateCurrentParagraphSpans(SpansTp &spans)
{
  const wstring &text = m_currentParagraph.GetText();
  const ParagraphC::TextExtentsTp &extents = m_currentParagraph.GetTextExtents();
  const ParagraphC::SpanMarksTp &spanMarks = m_currentParagraph.GetSpanMarks();

  if (text.size() > 0)
  {
    assert(text.size() == extents.size());
    assert(spanMarks.size() > 0);
    assert(spanMarks.begin()->first == 0);

    ParagraphC::SpanMarksConstItTp spanMarkIt = spanMarks.begin();
    const VSDOpenSpanOutputElement *pCurOpenSpan = spanMarks.begin()->second;
    spanMarkIt++;

    double leftMargInch = m_currentParagraph.GetMarginLeftInch();
    double rightMargInch = m_currentParagraph.GetMarginRightInch();

    double rowWidthInch = m_widthInch - m_padLeftInch - m_padRightInch
                          - leftMargInch - rightMargInch;// + 0.5 / 72.0; // @TODO Revise: 0.5pt correction - empirical only!

    double curSpanIndentInch = leftMargInch; // relative indent of the current (not-yet-written) span
    double curRowLeftExtentInch = 0; // left edge of the bounding box mapped to 1-D contiguous coordinate
    double lastNonTabCharExtentInch = 0; // extent of the last row character that was not tab
    int lastWhiteIdx = -1; // white-space or hyphen break candidate index
    bool whiteSpanEnd = false; // the last row span ended with white space
    wstring curSpanText;

    for (unsigned int i = 0, charCnt = text.size(); i < charCnt; i++)
    {
      bool addCurChar = true;
      int createNewSpan = 0; // <1 - do not create, 1 - without new line, >1 - with new line
      const VSDOpenSpanOutputElement *pNextOpenSpan = pCurOpenSpan;

      if (spanMarkIt != spanMarks.end() && spanMarkIt->first == i) // new span becomes active
      {
        if (curSpanText.size() > 0) // the text is not empty (shall always be true)
        {
          createNewSpan = 1;
        }

        pNextOpenSpan = spanMarkIt->second;
        spanMarkIt++; // advance to the next span to match in subsequent iterations
      }

      if (text[i] == L'\t')
      {
        addCurChar = false; // do not add tabs in the resulting text

        if (curSpanText.size() == 0) // row-leading tab
        {
          curSpanIndentInch = leftMargInch + extents[i] - lastNonTabCharExtentInch;
        }
        else // row-interleaved tab
        {
          createNewSpan = 1;
        }
      }
      else if (text[i] == 0x2028) // unicode line separator
      {
        addCurChar = false;
        createNewSpan = 2;
        curRowLeftExtentInch = extents[i];
        lastNonTabCharExtentInch = extents[i];
      }
      else // non-tab character
      {
        lastNonTabCharExtentInch = extents[i];
      }

      if (createNewSpan > 0)
      {
        AddSpan(
          spans, pCurOpenSpan, curSpanText, curSpanIndentInch, i - curSpanText.size(), i - 1,
          createNewSpan > 1);

        curSpanIndentInch =
          text[i] == L'\t' ? leftMargInch + extents[i] - extents[i - 1] : leftMargInch;

        wchar_t lastChar = curSpanText[curSpanText.size() - 1];
        whiteSpanEnd = !!iswspace(lastChar) || lastChar == L'-';
        lastWhiteIdx = -1;
        curSpanText.clear();
      }

      if (curSpanText.size() > 0 // must not be empty, i.e. at least one character per row even it does not fit entirely
          && extents[i] > curRowLeftExtentInch + rowWidthInch) // available space exceeded by the current character
      {
        unsigned int fitCharCnt; // number of characters to fit in the rest of the available space

        if (iswspace(text[i])) // a white space exceeded the boundary
        {
          fitCharCnt = curSpanText.size();
          addCurChar = false; // do not include the white space in the new row
        }
        else if (lastWhiteIdx >= 0)
        {
          fitCharCnt = lastWhiteIdx + 1; // up to last white-space character including
        }
        else if (whiteSpanEnd) // break after white space end of the last span
        {
          fitCharCnt = 0;
        }
        else // no text-break candidate exist
        {
          fitCharCnt = curSpanText.size(); // up to last fitting character
        }

        if (fitCharCnt == 0) // the row breaks right after the previous span
        {
          curRowLeftExtentInch = extents[i - curSpanText.size() - 1]; // update the row start position
          spans[spans.size() - 1].SetNewLine(); // update the previous span
        }
        else
        {
          AddSpan(
            spans, pCurOpenSpan, curSpanText.substr(0, fitCharCnt), curSpanIndentInch,
            i - curSpanText.size(), i - curSpanText.size() + fitCharCnt - 1, true);

          curRowLeftExtentInch = extents[i - curSpanText.size() + fitCharCnt - 1]; // adjust the new beginning of the current row
          curSpanText = curSpanText.substr(fitCharCnt); // go on with the remaining part

          if (lastWhiteIdx >= 0)
          {
            lastWhiteIdx -= fitCharCnt;
          }
        }

        curSpanIndentInch = leftMargInch;
        lastNonTabCharExtentInch = extents[i];
      }
      else // current character still fits in
      {
        if (iswspace(text[i]) || text[i] == L'-')
        {
          lastWhiteIdx = curSpanText.size();
        }
      }

      if (addCurChar)
      {
        curSpanText += text[i];
      }

      pCurOpenSpan = pNextOpenSpan;
    }

    if (curSpanText.size() > 0) // the rest of the text not yet included
    {
      AddSpan(
        spans, pCurOpenSpan, curSpanText, curSpanIndentInch,
        text.size() - curSpanText.size(), text.size() - 1, true);
    }

    if (spans.size() > 0)
    {
      spans[spans.size() - 1].SetNewLine(); // assure the line break after the last span

      double totalWidthInch = 0.0; // set sum of spans widths to first spans on each row (used for hor. alignment)

      for (int i = spans.size() - 1; i >= 0; i--)
      {
        totalWidthInch += spans[i].GetSpanOffsetInch() + spans[i].GetSpanWidthInch();

        if (i == 0 || spans[i - 1].EndsWithNewLine())
        {
          spans[i].SetRowTotalWidthInch(totalWidthInch);
          totalWidthInch = 0.0;
        }
      }
    }
  }
}

void SvgTextObjectC::AddSpan(
  SpansTp &spans, const VSDOpenSpanOutputElement *pSvgSpan, const wstring &text,
  double offsetInch, int firstCharIdx, int lastCharIdx, bool endWithNewLine)
{
  const ParagraphC::TextExtentsTp &extents = m_currentParagraph.GetTextExtents();
  double leftExtentInch = firstCharIdx > 0 ? extents[firstCharIdx - 1] : 0;
  double rightExtentInch = extents[lastCharIdx];

  double lastCharExtentInch =
    lastCharIdx > 0 ? extents[lastCharIdx] - extents[lastCharIdx - 1] : extents[lastCharIdx];

  spans.push_back(SpanC(
                    pSvgSpan, text, offsetInch, rightExtentInch - leftExtentInch, lastCharExtentInch,
                    endWithNewLine));
}
