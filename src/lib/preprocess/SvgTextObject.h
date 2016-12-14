#ifndef _SVG_TEXT_OBJECT_H_INCLUDED_
#define _SVG_TEXT_OBJECT_H_INCLUDED_


#include "SvgParagraph.h"
#include "SvgSpan.h"

#include <string>


namespace libvisio
{
class VSDStartTextObjectOutputElement;
class VSDOpenParagraphOutputElement;
class VSDOpenUnorderedListLevelOutputElement;
class VSDOpenListElementOutputElement;
class VSDOpenSpanOutputElement;
class VSDInsertTextOutputElement;
class VSDInsertTabOutputElement;
}

class SvgDC;
class SvgFontC;


/**
 * Represents an SVG text object and provides methods that reflect its particular structure and
 * calculate additional properties of contained elements that are required for proper rendering.
 */
class SvgTextObjectC
{
public:
  enum ProcessingStageE
  {
    PS_NONE,
    PS_CALCULATE,
    PS_LAYOUT
  };

  SvgTextObjectC(libvisio::VSDStartTextObjectOutputElement *pStartText, SvgDC &dc);

  SvgDC &GetDC();
  ProcessingStageE GetProcessingStage() const;

  double GetOrigXInch() const;
  double GetOrigYInch() const;
  double GetWidthInch() const;
  double GetHeightInch() const;
  double GetPaddingLeftInch() const;
  double GetPaddingRightInch() const;
  double GetPaddingTopInch() const;
  double GetPaddingBottomInch() const;

  double GetCurrentLineYInch() const;
  double RowOfTextAdded();

  const std::string &GetCurrentHorizontalAlignment() const;
  const SvgFontC *GetCurrentFont() const;

  const std::string &GetBackgroundColor() const;
  void SetCurrentTextLeftBoundInch(double x);
  void SetCurrentTextRightBoundInch(double x);
  void GetCurrentTextBoundsInch(double &x1, double &y1, double &x2, double &y2) const;
  double GetCurrentTextBoundTopInch() const;
  std::wstring GetCurrentBulletCharacter() const;

  void OpenParagraph(const libvisio::VSDOpenParagraphOutputElement *pOpenParagraph);
  void OpenUnorderedList(const libvisio::VSDOpenUnorderedListLevelOutputElement *pList);
  void OpenListElement(const libvisio::VSDOpenListElementOutputElement *pOpenListElem);
  void OpenSpan(const libvisio::VSDOpenSpanOutputElement *pOpenSpan);
  void InsertText(const libvisio::VSDInsertTextOutputElement *pInsertText);
  void InsertTab(const libvisio::VSDInsertTabOutputElement *pInsertTab);
  void CalculateCurrentParagraphSpans(SpansTp &spans);

  void StartCalculationStage();
  void StartLayoutStage();

private:
  void AddSpan(
    SpansTp &spans, const libvisio::VSDOpenSpanOutputElement *pSvgSpan, const std::wstring &text,
    double offsetInch, int firstCharIdx, int lastCharIdx, bool endWithNewLine);

  double m_origXInch; ///< X coordinate of the top left corner in inches
  double m_origYInch; ///< Y coordinate of the top left corner in inches
  double m_widthInch; ///< box width in inches
  double m_heightInch; ///< box height in inches
  double m_padLeftInch; ///< left padding in inches
  double m_padRightInch; ///< right padding in inches
  double m_padTopInch; ///<< top padding in inches
  double m_padBottomInch; ///< bottom padding in inches

  SvgDC &m_dc; ///< device context to use for text size-related calculations
  mutable const SvgFontC *m_pCurrentFont; ///< font that applies to the currently processed text

  ProcessingStageE m_stage; ///< stage of processing of textbox contents
  double m_firstLineOffsetYInch; ///< vertical offset of the first (base)line in the text box
  double m_currentLineYInch; ///< vertical position of the current line of text
  std::string m_verticalAlign; ///< vertical alignment of the text box

  double m_textLeftBoundInch; ///< the left bound of the left-most line of the text
  double m_textRightBoundInch; ///< the right bound of the right-most line of the text
  std::string m_bkgColor; ///< text box background color or empty string for transparent background

  ParagraphC m_currentParagraph; ///< paragraph currently being processed
  std::wstring m_currentBulletChar; ///< the character to use for the subsequest list item bullet
};


#endif // _SVG_TEXT_OBJECT_H_INCLUDED_
