/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FUZZERLIMITS_HXX
#define INCLUDED_FUZZERLIMITS_HXX

#include <cstddef>

// The largest input the fuzzers parse, the same as the max_len of most of the LibreOffice
// fuzzers. A slow parse of a larger input is mostly the cost of its size.
constexpr std::size_t maxFuzzInputSize = 65536;

#endif

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
