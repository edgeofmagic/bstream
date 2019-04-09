/*
 * The MIT License
 *
 * Copyright 2017 David Curtis.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BSTREAM_TYPES_H
#define BSTREAM_TYPES_H

#include <array>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <limits>
#include <util/types.h>
#include <system_error>


namespace bstream
{

template<std::size_t N>
using byte_block = std::array<std::uint8_t, N>;

using poly_tag_type = int;

static constexpr poly_tag_type invalid_tag = -1;

enum class seek_anchor
{
	begin,
	current,
	end
};

enum class open_mode
{
	truncate,
	append,
	at_end,
	at_begin,
};

enum class byte_order
{
	big_endian,
	little_endian
};

constexpr bool
is_reverse(byte_order order)
{
	return boost::endian::order::native
		   != ((order == byte_order::big_endian) ? boost::endian::order::big : boost::endian::order::little);
}

struct as_shared_buffer
{};
struct as_const_buffer
{};
struct as_mutable_buffer
{};

namespace detail
{

template<int N>
struct canonical_type;

template<>
struct canonical_type<1>
{
	using type = std::uint8_t;
};

template<>
struct canonical_type<2>
{
	using type = std::uint16_t;
};

template<>
struct canonical_type<4>
{
	using type = std::uint32_t;
};

template<>
struct canonical_type<8>
{
	using type = std::uint64_t;
};

}    // namespace detail

}    // namespace bstream

#endif    // BSTREAM_TYPES_H