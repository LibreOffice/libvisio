#ifndef _SVG_PARAGRAPH_H_INCLUDED_
#define _SVG_PARAGRAPH_H_INCLUDED_


#include <map>
#include <string>
#include <vector>


namespace librevenge
{
class RVNGPropertyList;
}

namespace libvisio
{
class VSDOpenParagraphOutputElement;
class VSDOpenListElementOutputElement;
class VSDOpenSpanOutputElement;
}

/**
 * Represents a paragraph of text within the original text box. Manages individual spans of text
 * and stores information about partial text extents.
 */
class ParagraphC
{
public:
  typedef std::vector<double> TextExtentsTp;
  typedef std::map<int, const libvisio::VSDOpenSpanOutputElement *> SpanMarksTp;
  typedef SpanMarksTp::const_iterator SpanMarksConstItTp;

  ParagraphC();

  double GetMarginLeftInch() const;
  double GetMarginRightInch() const;
  double GetMarginTopInch() const;
  double GetMarginBottomInch() const;
  double GetLineHeightInch(double fontSizeInch) const;

  void Reset(const libvisio::VSDOpenParagraphOutputElement *pOpenParagraph);
  void Reset(const libvisio::VSDOpenListElementOutputElement *pOpenListElem);
  void Reset(const librevenge::RVNGPropertyList &properties);
  void AddSpan(const libvisio::VSDOpenSpanOutputElement *pSpan);
  void AddText(const std::wstring &text, const TextExtentsTp &extents);

  double GetCurrentTextExtentInch() const;
  const std::string &GetHorizontalAlignment() const;
  const std::wstring &GetText() const;
  const TextExtentsTp &GetTextExtents() const;
  const SpanMarksTp &GetSpanMarks() const;
  const libvisio::VSDOpenSpanOutputElement *GetSpanFromIndex(unsigned int index);

private:
  double m_margLeftInch; ///< left margin in inches
  double m_margRightInch; ///< right margin in inches
  double m_margTopInch; ///<< top margin in inches
  double m_margBottomInch; ///< bottom margin in inches

  double m_lineHeight; ///< line height (relative or absolute in inches)
  bool m_isLineHeightRel; ///< indicates if m_lineHeight is a relative value
  std::string m_horizontalAlignment; ///< horizontal alignment of the text
  std::wstring m_text; ///< paragraph text
  TextExtentsTp m_textExtents; ///< contiguous line of extents of characters within the paragraph
  SpanMarksTp m_spanMarks; ///< maps indices of characters within paragraph text to Span elements
};


#endif // _SVG_PARAGRAPH_H_INCLUDED_
