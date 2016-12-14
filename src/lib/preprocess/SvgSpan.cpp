#include "SvgSpan.h"

using namespace libvisio;
using namespace std;


SpanC::SpanC(
  const VSDOpenSpanOutputElement *pSpan, const wstring &text, double offsetInch,
  double widthInch, double lastCharWidthInch, bool hasNewLine)
  : m_pSpan(pSpan),
    m_text(text),
    m_offsetInch(offsetInch),
    m_widthInch(widthInch),
    m_lastCharWidthInch(lastCharWidthInch),
    m_rowTotalWidthInch(-1.0),
    m_endsWithNewLine(hasNewLine)
{
}

const VSDOpenSpanOutputElement *SpanC::GetSpan() const
{
  return m_pSpan;
}

const wstring SpanC::GetText(bool exclTrailWhiteSpace) const
{
  unsigned int textLen = m_text.size();

  if (m_endsWithNewLine && exclTrailWhiteSpace
      && textLen > 0 && iswspace(m_text[textLen - 1]))
  {
    return m_text.substr(0, textLen - 1);
  }

  return m_text;
}

double SpanC::GetSpanOffsetInch() const
{
  return m_offsetInch;
}

double SpanC::GetSpanWidthInch(bool exclTrailWhiteSpace) const
{
  unsigned int textLen = m_text.size();

  if (m_endsWithNewLine && exclTrailWhiteSpace
      && textLen > 0 && iswspace(m_text[textLen - 1]))
  {
    return m_widthInch - m_lastCharWidthInch;
  }

  return m_widthInch;
}

double SpanC::GetRowTotalWidthInch() const
{
  return m_rowTotalWidthInch;
}

bool SpanC::EndsWithNewLine() const
{
  return m_endsWithNewLine;
}

void SpanC::SetNewLine()
{
  m_endsWithNewLine = true;
}

void SpanC::SetRowTotalWidthInch(double width)
{
  m_rowTotalWidthInch = width;
}
