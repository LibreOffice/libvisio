#ifndef _SVG_SPAN_H_INCLUDED_
#define _SVG_SPAN_H_INCLUDED_


#include <string>
#include <vector>


namespace libvisio
{
class VSDOpenSpanOutputElement;
}


/**
 * Represents an SVG text span and stores its properties required for laying out/wrapping the text
 * within the parent text box.
 */
class SpanC
{
public:
  SpanC(
    const libvisio::VSDOpenSpanOutputElement *pSpan, const std::wstring &text,
    double offsetInch, double widthInch, double lastCharWidthInch, bool hasNewLine);

  const libvisio::VSDOpenSpanOutputElement *GetSpan() const;
  const std::wstring GetText(bool exclTrailWhiteSpace = true) const;
  double GetSpanOffsetInch() const;
  double GetSpanWidthInch(bool exclTrailWhiteSpace = true) const;
  double GetRowTotalWidthInch() const;
  bool EndsWithNewLine() const;
  void SetNewLine();
  void SetRowTotalWidthInch(double width);

private:
  const libvisio::VSDOpenSpanOutputElement *m_pSpan; ///< the original span element
  std::wstring m_text; ///< span text (UTF-8 encoded)
  double m_offsetInch; ///< horizontal offset of the span counted from the end of the previous span
  double m_widthInch; ///< width of the span in inches
  double m_lastCharWidthInch; ///< width in inches of the last span character
  double m_rowTotalWidthInch; ///< width of all spans in inches on the same row of text
  bool m_endsWithNewLine; ///< indicates a new line after this span
};


typedef std::vector<SpanC> SpansTp;
typedef SpansTp::iterator SpansItTp;
typedef SpansTp::const_iterator SpansContItTp;


#endif // _SVG_SPAN_H_INCLUDED_H_
