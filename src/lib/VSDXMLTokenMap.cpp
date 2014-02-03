/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libvisio
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
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

const xmlChar *libvisio::VSDXMLTokenMap::getTokenName(int tokenId)
{
  if (tokenId >= XML_TOKEN_COUNT)
    return 0;

  const xmltoken *currentToken = wordlist;
  while (currentToken != wordlist+sizeof(wordlist)/sizeof(*wordlist))
  {
    if (currentToken->tokenId == tokenId)
      return BAD_CAST(currentToken->name);
    ++currentToken;
  }

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
