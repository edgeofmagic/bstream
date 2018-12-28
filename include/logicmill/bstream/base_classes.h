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

#ifndef LOGICMILL_BSTREAM_BASE_CLASSES_H
#define LOGICMILL_BSTREAM_BASE_CLASSES_H

#include <logicmill/bstream/ibstream.h>
#include <logicmill/bstream/obstream.h>
#include <sstream>

namespace logicmill
{
namespace bstream
{

template<class Derived>
class streaming_base
{
protected:
	streaming_base(ibstream& is)
	{
		auto length = is.read_array_header();
		if (length != _item_count())
		{
			throw std::system_error{make_error_code(bstream::errc::member_count_error)};
		}
	}

	streaming_base(){};

	obstream&
	_serialize(obstream& os) const
	{
		return os.write_array_header(_item_count());
	}

	ibstream&
	_deserialize(ibstream& is)
	{
		auto length = is.read_array_header();
		if (length != _item_count())
		{
			throw std::system_error{make_error_code(bstream::errc::member_count_error)};
		}
		return is;
	}

	std::size_t
	_item_count() const
	{
		return static_cast<const Derived&>(*this)._streamed_item_count();
	}
};

}    // namespace bstream
}    // namespace logicmill

#endif    // LOGICMILL_BSTREAM_BASE_CLASSES_H
