/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXMLTokenMap.h"

#include <string.h>

namespace
{

#include "tokenhash.h"

} // anonymous namespace

int libvisio::VSDXMLTokenMap::getTokenId(const xmlChar *name)
{
  const xmltoken *token = Perfect_Hash::in_word_set((const char *)name, xmlStrlen(name));
  if (token)
    return token->tokenId;
  else
    return XML_TOKEN_INVALID;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
