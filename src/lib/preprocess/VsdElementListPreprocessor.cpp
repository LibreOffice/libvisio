#include "librevenge/SvgConstants.h"
#include "SvgDC.h"
#include "SvgUtils.h"
#include "SvgTextObject.h"
#include "VsdElementListPreprocessor.h"

#include <assert.h>

using namespace librevenge;
using namespace libvisio;
using namespace std;
using namespace svgconstants;


void VsdElementListPreprocessorC::Process(
  const ElementListTp &elements, ElementListTp &outElements)
{
  SvgDC &dc = GetSystemDC();

  for (ElementListConstItTp it = elements.begin(); it != elements.end(); it++)
  {
    ElementListConstItTp oldIt = it;
    it = ProcessTextObject(it, elements.end(), outElements, dc);

    if (it == oldIt)
    {
      outElements.push_back((*it)->clone());
    }
    else if (it == elements.end())
    {
      break;
    }
  }
}

VsdElementListPreprocessorC::ElementListConstItTp VsdElementListPreprocessorC::ProcessTextObject(
  ElementListConstItTp startIt, ElementListConstItTp endIt, ElementListTp &outElements, SvgDC &dc)
{
  ElementListConstItTp it = startIt;

  VSDStartTextObjectOutputElement *pStartText =
    dynamic_cast<VSDStartTextObjectOutputElement *>(*it);

  if (pStartText == NULL) // not StartTextObject element
  {
    return it;
  }

  pStartText = static_cast<VSDStartTextObjectOutputElement *>((*it)->clone()); // work with the copy!
  SvgTextObjectC textObjContext(pStartText, dc);
  it++; // move to the next element

  // 1st pass
  ElementListTp elementsPass1;
  textObjContext.StartCalculationStage();
  it = ProcessTextObjectOnePass(it, endIt, elementsPass1, &textObjContext);

  if (!textObjContext.GetBackgroundColor().empty())
  {
    double x1, y1, x2, y2;
    textObjContext.GetCurrentTextBoundsInch(x1, y1, x2, y2);

    outElements.push_back(CreateStyleOutput(textObjContext.GetBackgroundColor()));
    outElements.push_back(CreateRectPathOutput(x1, y1, x2, y2));
  }

  outElements.push_back(pStartText);

  // 2nd pass
  textObjContext.StartLayoutStage();

  ProcessTextObjectOnePass(
    elementsPass1.begin(), elementsPass1.end(), outElements, &textObjContext);

  return it;
}

VsdElementListPreprocessorC::ElementListConstItTp
VsdElementListPreprocessorC::ProcessTextObjectOnePass(
  ElementListConstItTp startIt, ElementListConstItTp endIt,
  ElementListTp &outElements, SvgTextObjectC *pTextObject)
{
  assert(pTextObject);

  ElementListConstItTp it;

  for (it = startIt; it != endIt; it++)
  {
    VSDEndTextObjectOutputElement *pEndTextObject =
      dynamic_cast<VSDEndTextObjectOutputElement *>(*it);

    if (pEndTextObject != NULL) // EndTextObject element found
    {
      outElements.push_back(pEndTextObject->clone()); // make a copy!
      break;
    }

    ElementListConstItTp oldIt = it;
    it = ProcessParagraph(it, endIt, outElements, pTextObject);

    if (it == oldIt)
    {
      it = ProcessUnorderedList(it, endIt, outElements, pTextObject);

      if (it == oldIt)
      {
        outElements.push_back((*it)->clone());
      }
    }

    if (it == endIt)
    {
      break;
    }
  }

  return it;
}

VsdElementListPreprocessorC::ElementListConstItTp VsdElementListPreprocessorC::ProcessParagraph(
  ElementListConstItTp startIt, ElementListConstItTp endIt, ElementListTp &outElements,
  SvgTextObjectC *pTextObject)
{
  assert(pTextObject);

  ElementListConstItTp it = startIt;

  VSDOpenParagraphOutputElement *pOpenParagraph =
    dynamic_cast<VSDOpenParagraphOutputElement *>(*it);

  if (pOpenParagraph == NULL)
  {
    return it; // not OpenParagraph element
  }

  pOpenParagraph = static_cast<VSDOpenParagraphOutputElement *>((*it)->clone()); // work with the copy!
  outElements.push_back(pOpenParagraph);

  SvgTextObjectC::ProcessingStageE stage = pTextObject->GetProcessingStage();

  if (stage == SvgTextObjectC::PS_CALCULATE)
  {
    pTextObject->OpenParagraph(pOpenParagraph);
  }

  it++; // move to the next element

  VSDCloseParagraphOutputElement *pCloseParagraph = NULL;

  for (; it != endIt; it++)
  {
    pCloseParagraph = dynamic_cast<VSDCloseParagraphOutputElement *>(*it);

    if (pCloseParagraph != NULL)
    {
      break;
    }

    ElementListConstItTp oldIt = it;
    it = ProcessSpan(it, endIt, outElements, pTextObject);

    if (it == oldIt)
    {
      outElements.push_back((*it)->clone());
    }
    else if (it == endIt)
    {
      break;
    }
  }

  if (stage == SvgTextObjectC::PS_CALCULATE)
  {
    SpansTp spans;
    pTextObject->CalculateCurrentParagraphSpans(spans);
    GenerateSpans(spans, outElements, pTextObject);
  }

  if (pCloseParagraph != NULL)
  {
    outElements.push_back(pCloseParagraph->clone()); // make a copy!
  }

  return it;
}

VsdElementListPreprocessorC::ElementListConstItTp VsdElementListPreprocessorC::ProcessUnorderedList(
  ElementListConstItTp startIt, ElementListConstItTp endIt,
  ElementListTp &outElements, SvgTextObjectC *pTextObject)
{
  assert(pTextObject);

  ElementListConstItTp it = startIt;

  VSDOpenUnorderedListLevelOutputElement *pOpenList =
    dynamic_cast<VSDOpenUnorderedListLevelOutputElement *>(*it);

  if (pOpenList == NULL)
  {
    return it; // not OpenUnorderedList element
  }

  pOpenList = static_cast<VSDOpenUnorderedListLevelOutputElement *>((*it)->clone()); // work with the copy!
  outElements.push_back(pOpenList);

  if (pTextObject->GetProcessingStage() == SvgTextObjectC::PS_CALCULATE)
  {
    pTextObject->OpenUnorderedList(pOpenList);
  }

  it++; // move to the next element

  for (; it != endIt; it++)
  {
    VSDCloseUnorderedListLevelOutputElement *pCloseList =
      dynamic_cast<VSDCloseUnorderedListLevelOutputElement *>(*it);

    if (pCloseList != NULL)
    {
      outElements.push_back(pCloseList->clone()); // make a copy!
      break;
    }

    ElementListConstItTp oldIt = it;
    it = ProcessListElement(it, endIt, outElements, pTextObject);

    if (it == oldIt)
    {
      outElements.push_back((*it)->clone());
    }
    else if (it == endIt)
    {
      break;
    }
  }

  return it;
}

VsdElementListPreprocessorC::ElementListConstItTp VsdElementListPreprocessorC::ProcessListElement(
  ElementListConstItTp startIt, ElementListConstItTp endIt,
  ElementListTp &outElements, SvgTextObjectC *pTextObject)
{
  assert(pTextObject);

  ElementListConstItTp it = startIt;

  VSDOpenListElementOutputElement *pOpenListElem =
    dynamic_cast<VSDOpenListElementOutputElement *>(*it);

  if (pOpenListElem == NULL)
  {
    return it; // not OpenListElement element
  }

  pOpenListElem = static_cast<VSDOpenListElementOutputElement *>((*it)->clone()); // work with the copy!
  outElements.push_back(pOpenListElem);

  SvgTextObjectC::ProcessingStageE stage = pTextObject->GetProcessingStage();

  if (stage == SvgTextObjectC::PS_CALCULATE)
  {
    pTextObject->OpenListElement(pOpenListElem);
  }

  it++; // move to the next element

  VSDCloseListElementOutputElement *pCloseListElem = NULL;

  for (; it != endIt; it++)
  {
    pCloseListElem = dynamic_cast<VSDCloseListElementOutputElement *>(*it);

    if (pCloseListElem != NULL)
    {
      break;
    }

    ElementListConstItTp oldIt = it;
    it = ProcessSpan(it, endIt, outElements, pTextObject);

    if (it == oldIt)
    {
      outElements.push_back((*it)->clone());
    }
    else if (it == endIt)
    {
      break;
    }
  }

  if (stage == SvgTextObjectC::PS_CALCULATE)
  {
    SpansTp spans;
    pTextObject->CalculateCurrentParagraphSpans(spans);

    if (spans.size() > 0) // add a span for the list item bullet
    {
      SpanC span = spans[0];
      double spanOffsetInch = span.GetSpanOffsetInch();
      RVNGPropertyList &props = pOpenListElem->GetPropertyList();

      double textIndentInch = props[PROP_FO_TEXT_INDENT]
                              ? SvgUtilsC::GetInchValue(*props[PROP_FO_TEXT_INDENT]) : -spanOffsetInch;

      if (textIndentInch >= 0)
      {
        textIndentInch = -spanOffsetInch;
      }

      spans.insert(
        spans.begin(),
        SpanC(span.GetSpan(), pTextObject->GetCurrentBulletCharacter(),
              spanOffsetInch + textIndentInch, -textIndentInch, -textIndentInch, false));
    }

    GenerateSpans(spans, outElements, pTextObject);
  }

  if (pCloseListElem != NULL)
  {
    outElements.push_back(pCloseListElem->clone()); // make a copy!
  }

  return it;
}

VsdElementListPreprocessorC::ElementListConstItTp VsdElementListPreprocessorC::ProcessSpan(
  ElementListConstItTp startIt, ElementListConstItTp endIt,
  ElementListTp &outElements, SvgTextObjectC *pTextObject)
{
  assert(pTextObject);

  ElementListConstItTp it = startIt;
  VSDOpenSpanOutputElement *pOpenSpan = dynamic_cast<VSDOpenSpanOutputElement *>(*it);

  if (pOpenSpan == NULL)
  {
    return it; // not OpenSpan element
  }

  SvgTextObjectC::ProcessingStageE stage = pTextObject->GetProcessingStage();
  it++; // move to the next element

  if (stage == SvgTextObjectC::PS_CALCULATE)
  {
    pTextObject->OpenSpan(pOpenSpan);

    for (; it != endIt; it++)
    {
      VSDCloseSpanOutputElement *pCloseSpan = dynamic_cast<VSDCloseSpanOutputElement *>(*it);

      if (pCloseSpan != NULL)
      {
        break;
      }

      VSDInsertTextOutputElement *pInsertText = dynamic_cast<VSDInsertTextOutputElement *>(*it);

      if (pInsertText != NULL)
      {
        pTextObject->InsertText(pInsertText);
      }

      VSDInsertTabOutputElement *pInsertTab = dynamic_cast<VSDInsertTabOutputElement *>(*it);

      if (pInsertTab != NULL)
      {
        pTextObject->InsertTab(pInsertTab);
      }
    }
  }
  else if (stage == SvgTextObjectC::PS_LAYOUT)
  {
    RVNGPropertyList &props = pOpenSpan->GetPropertyList();
    double posYInch = SvgUtilsC::GetInchValue(*props[PROP_SVG_Y]);

    posYInch += pTextObject->GetCurrentLineYInch();
    props.insert(PROP_SVG_Y, posYInch, RVNG_INCH);

    outElements.push_back(pOpenSpan->clone());

    for (; it != endIt; it++)
    {
      VSDCloseSpanOutputElement *pCloseSpan = dynamic_cast<VSDCloseSpanOutputElement *>(*it);

      if (pCloseSpan != NULL)
      {
        outElements.push_back((*it)->clone());
        break;
      }

      VSDInsertTextOutputElement *pInsertText = dynamic_cast<VSDInsertTextOutputElement *>(*it);

      if (pInsertText != NULL)
      {
        outElements.push_back((*it)->clone());
      }
    }

  }

  return it;
}

void VsdElementListPreprocessorC::GenerateSpans(
  const SpansTp &spans, ElementListTp &outElements, SvgTextObjectC *pTextObject)
{
  if (spans.size() == 0)
  {
    // @TODO Revise: More accurate results are sometimes achieved when empty paragraph
    // is ignorred in the result document, rather than if the a new line is added
    pTextObject->RowOfTextAdded();
  }
  else
  {
    double posXInch = 0.0;
    bool wasNewRow = true;

    for (SpansContItTp spanIt = spans.begin(); spanIt != spans.end(); spanIt++)
    {
      VSDOpenSpanOutputElement *pOpenSpan = static_cast<VSDOpenSpanOutputElement *>(
                                              const_cast<VSDOpenSpanOutputElement *>(spanIt->GetSpan())->clone()); // const_cast needed as clone() is not const (but should be)

      if (wasNewRow)
      {
        wasNewRow = false;
        string horAlign = pTextObject->GetCurrentHorizontalAlignment();

        if (horAlign.compare(PVAL_FO_TEXT_ALIGN_LEFT) == 0)
        {
          posXInch = pTextObject->GetOrigXInch() + pTextObject->GetPaddingLeftInch();
        }
        else if (horAlign.compare(PVAL_FO_TEXT_ALIGN_RIGHT) == 0)
        {
          posXInch = pTextObject->GetOrigXInch() + pTextObject->GetWidthInch()
                     - pTextObject->GetPaddingRightInch() - spanIt->GetRowTotalWidthInch();
        }
        else // PVAL_FO_TEXT_ALIGN_CENTER
        {
          posXInch = pTextObject->GetOrigXInch()
                     + (pTextObject->GetWidthInch() - spanIt->GetRowTotalWidthInch()) / 2.0;
        }

        posXInch += spanIt->GetSpanOffsetInch();
      }

      pTextObject->SetCurrentTextLeftBoundInch(posXInch);
      pTextObject->SetCurrentTextRightBoundInch(posXInch + spanIt->GetSpanWidthInch());

      RVNGPropertyList &props = pOpenSpan->GetPropertyList();
      props.insert(PROP_SVG_X, posXInch, RVNG_INCH);
      props.insert(PROP_SVG_Y, pTextObject->GetCurrentLineYInch(), RVNG_INCH); // relative to the textbox top

      outElements.push_back(pOpenSpan);

      outElements.push_back(
        new VSDInsertTextOutputElement(SvgUtilsC::WstrToStr(spanIt->GetText()).c_str()));

      outElements.push_back(new VSDCloseSpanOutputElement());

      if (spanIt->EndsWithNewLine())
      {
        wasNewRow = true;
        pTextObject->RowOfTextAdded();
      }
      else
      {
        posXInch += spanIt->GetSpanWidthInch();
      }
    }
  }
}

VSDStyleOutputElement *VsdElementListPreprocessorC::CreateStyleOutput(const string &fillColor)
{
  RVNGPropertyList props;
  props.insert(PROP_SVG_STROKE_WIDTH, 0.0, RVNG_INCH);
  props.insert(PROP_DRAW_FILL, "solid");
  props.insert(PROP_DRAW_FILL_COLOR, fillColor.c_str());
  return new VSDStyleOutputElement(props);
}

VSDPathOutputElement *VsdElementListPreprocessorC::CreateRectPathOutput(
  double x1Inch, double y1Inch, double x2Inch, double y2Inch)
{
  const char *TAG_SVG_D = "svg:d";
  RVNGPropertyListVector actions;

  RVNGPropertyList actionM;
  actionM.insert(PROP_LIBREV_ACTION, "M");
  actionM.insert(PROP_SVG_X, x1Inch, RVNG_INCH);
  actionM.insert(PROP_SVG_Y, y1Inch, RVNG_INCH);
  actions.append(actionM);

  RVNGPropertyList actionL1;
  actionL1.insert(PROP_LIBREV_ACTION, "L");
  actionL1.insert(PROP_SVG_X, x2Inch, RVNG_INCH);
  actionL1.insert(PROP_SVG_Y, y1Inch, RVNG_INCH);
  actions.append(actionL1);

  RVNGPropertyList actionL2;
  actionL2.insert(PROP_LIBREV_ACTION, "L");
  actionL2.insert(PROP_SVG_X, x2Inch, RVNG_INCH);
  actionL2.insert(PROP_SVG_Y, y2Inch, RVNG_INCH);
  actions.append(actionL2);

  RVNGPropertyList actionL3;
  actionL3.insert(PROP_LIBREV_ACTION, "L");
  actionL3.insert(PROP_SVG_X, x1Inch, RVNG_INCH);
  actionL3.insert(PROP_SVG_Y, y2Inch, RVNG_INCH);
  actions.append(actionL3);

  RVNGPropertyList actionZ;
  actionZ.insert(PROP_LIBREV_ACTION, "Z");
  actions.append(actionZ);

  RVNGPropertyList props;
  props.insert(TAG_SVG_D, actions);
  return new VSDPathOutputElement(props);
}
