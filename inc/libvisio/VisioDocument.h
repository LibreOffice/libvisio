/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VISIODOCUMENT_H__
#define __VISIODOCUMENT_H__

#include <librevenge/librevenge.h>

#ifdef DLL_EXPORT
#ifdef LIBVISIO_BUILD
#define VSDAPI __declspec(dllexport)
#else
#define VSDAPI __declspec(dllimport)
#endif
#else // !DLL_EXPORT
#ifdef LIBVISIO_VISIBILITY
#define VSDAPI __attribute__((visibility("default")))
#else
#define VSDAPI
#endif
#endif

namespace libvisio
{

class VisioDocument
{
public:

  static VSDAPI bool isSupported(librevenge::RVNGInputStream *input);

  static VSDAPI bool parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);

  static VSDAPI bool parseStencils(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
};

} // namespace libvisio

#endif //  __VISIODOCUMENT_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
