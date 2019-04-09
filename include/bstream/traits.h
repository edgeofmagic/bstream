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

#ifndef BSTREAM_TRAITS_H
#define BSTREAM_TRAITS_H

#include <armi/serialization_traits.h>
#include <armi/types.h>
#include <bstream.h>


namespace bstream
{
template<class StreamContext>
struct serialization
{
	using stream_context_type = StreamContext > ;
};
}    // namespace bstream

template<class StreamContext>
struct armi::serialization::traits<bstream::serialization<StreamContext>>
{
	using serializer_type     = bstream::obstream;
	using deserializer_type   = bstream::ibstream;
	using serializer_param_type     = serializer_type&;
	using deserializer_param_type   = deserializer_type&;
	using stream_context_type = StreamContext;

	template<class T>
	static T
	read(deserializer_param_type is)
	{
		return is.read_as<T>();
	}

	template<class T>
	static void
	read(deserializer_param_type is, T& value)
	{
		is.read_as(value);
	}

	template<class T>
	static T
	write(serializer_param_type os, T const& value)
	{
		os << value;
	}

	static void
	write_sequence_prefix(serializer_param_type os, std::size_t count)
	{
		os.write_array_header(count);
	}

	static std::size_t
	read_sequence_prefix(deserializer_param_type is)
	{
		return is.read_array_header();
	}

	static std::unique_ptr<serializer_type>
	create_serializer()
	{
		return std::make_unique<serializer_type>(stream_context_type::get());
	}

	static void
	clear(deserializer_param_type is)
	{
		is.clear();
	}
};

#endif    // BSTREAM_TRAITS_H
