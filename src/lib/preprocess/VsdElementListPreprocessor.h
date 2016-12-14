#ifndef _VSD_ELEMENT_LIST_PREPROCESSOR_H_INCLUDED_
#define _VSD_ELEMENT_LIST_PREPROCESSOR_H_INCLUDED_

#include "SvgSpan.h"
#include "VSDOutputElementList.h"

#include <vector>


class SvgDC;
class SvgTextObjectC;

namespace libvisio
{
class VSDStyleOutputElement;
class VSDPathOutputElement;
}

/**
 * Pre-processes a specified list of VSD output elements before they are rendered to the output
 * SVG document. The purpose is to calculate additional data related to the elements and make it
 * available to the renderer, so that the resulting SVG is the closest possible match of the
 * original. In particular, text aligment and wrapping within parent text boxes are addressed here.
 * Preprocessing produces a new element list, which in comparison to the input one may add new
 * elements, remove existing ones, or add or modify properties of other elements.
 */
class VsdElementListPreprocessorC
{
public:
  typedef std::vector<libvisio::VSDOutputElement *> ElementListTp;
  typedef ElementListTp::iterator ElementListItTp;
  typedef ElementListTp::const_iterator ElementListConstItTp;

  void Process(const ElementListTp &elements, ElementListTp &outElements);

private:
  ElementListConstItTp ProcessTextObject(
    ElementListConstItTp startIt, ElementListConstItTp endIt, ElementListTp &outElements,
    SvgDC &dc);

  ElementListConstItTp ProcessTextObjectOnePass(
    ElementListConstItTp startIt, ElementListConstItTp endIt,
    ElementListTp &outElements, SvgTextObjectC *pTextObject);

  ElementListConstItTp ProcessParagraph(
    ElementListConstItTp startIt, ElementListConstItTp endIt,
    ElementListTp &outElements, SvgTextObjectC *pTextObject);

  ElementListConstItTp ProcessUnorderedList(
    ElementListConstItTp startIt, ElementListConstItTp endIt,
    ElementListTp &outElements, SvgTextObjectC *pTextObject);

  ElementListConstItTp ProcessListElement(
    ElementListConstItTp startIt, ElementListConstItTp endIt,
    ElementListTp &outElements, SvgTextObjectC *pTextObject);

  ElementListConstItTp ProcessSpan(
    ElementListConstItTp startIt, ElementListConstItTp endIt,
    ElementListTp &outElements, SvgTextObjectC *pTextObject);

  void GenerateSpans(
    const SpansTp &spans, ElementListTp &outElements, SvgTextObjectC *pTextObject);

  libvisio::VSDStyleOutputElement *CreateStyleOutput(const std::string &fillColor);

  libvisio::VSDPathOutputElement *CreateRectPathOutput(
    double x1Inch, double y1Inch, double x2Inch, double y2Inch);
};


#endif // _VSD_ELEMENT_LIST_PREPROCESSOR_H_INCLUDED_
