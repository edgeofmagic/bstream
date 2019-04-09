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

#ifndef LOGICMILL_ARMI_ASYNC_ADAPTER_BSTREAM_BRIDGE_H
#define LOGICMILL_ARMI_ASYNC_ADAPTER_BSTREAM_BRIDGE_H

#include <logicmill/armi/adapters/bstream/traits.h>
#include <logicmill/armi/error.h>
#include <logicmill/armi/types.h>
#include <logicmill/async/loop.h>
#include <logicmill/bstream.h>
#include <logicmill/bstream/imbstream.h>
#include <logicmill/bstream/ombstream.h>

namespace logicmill
{
namespace armi
{
namespace adapters
{

template<class SerializationTraits, class AsyncIOTraits>
struct bridge;

template<class StreamContext>
struct bridge<bstream::serialization_traits<StreamContext>, async::async_io_traits>
{
	using serialization_traits = logicmill::bstream::serialization_traits<StreamContext>;
	using async_io_traits     = logicmill::async::async_io_traits;

	using mutable_buffer_type       = util::mutable_buffer;
	using mutable_buffer_param_type = util::mutable_buffer&&;
	using const_buffer_type         = util::const_buffer;
	using const_buffer_param_type   = util::const_buffer&&;

	using stream_context_type = StreamContext;

	using serializer_param_type   = typename serialization_traits::serializer_param_type;
	using deserializer_param_type = typename serialization_traits::deserializer_param_type;

	static constexpr std::size_t default_mutable_buffer_size = 65536;

	using mutable_buffer_callback = std::function<void(mutable_buffer_param_type)>;
	using deserializer_callback   = std::function<void(deserializer_param_type)>;
	using serializer_callback     = std::function<void(serializer_param_type)>;

	static void
	serializer_from_mutable_buffer(mutable_buffer_param_type buf, serializer_callback cb)
	{
		logicmill::bstream::ombstream os{std::move(buf), stream_context_type::get()};
		cb(os);
	}

	static void
	new_serializer(serializer_callback cb)
	{
		logicmill::bstream::ombstream os{default_mutable_buffer_size, stream_context_type::get()};
		cb(os);
	}

	static void
	mutable_buffer_from_serializer(serializer_param_type os, mutable_buffer_callback cb)
	{
		cb(os.release_mutable_buffer());
	}

	static void
	deserializer_from_const_buffer(const_buffer_param_type buf, deserializer_callback cb)
	{
		logicmill::bstream::imbstream is{std::move(buf), stream_context_type::get()};
		cb(is);
	}
};

}    // namespace adapters
}    // namespace armi
}    // namespace logicmill

#endif    // LOGICMILL_ARMI_ASYNC_ADAPTER_BSTREAM_BRIDGE_H
