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

#ifndef BSTREAM_OMBSTREAM_H
#define BSTREAM_OMBSTREAM_H

#include <bstream/buffer/sink.h>
#include <bstream/obstream.h>


namespace bstream
{

class ombstream : public obstream
{
public:
	ombstream()                 = delete;
	ombstream(ombstream const&) = delete;
	ombstream(ombstream&&)      = delete;

	ombstream(std::unique_ptr<buffer::sink> sink, context_base const& context = get_default_context())
		: obstream{std::move(sink), context}
	{}

	ombstream(util::mutable_buffer&& buf, context_base const& context = get_default_context())
		: obstream{std::make_unique<buffer::sink>(std::move(buf), context.byte_order()), context}
	{}

	ombstream(util::size_type size, context_base const& context = get_default_context())
		: ombstream(std::make_unique<buffer::sink>(size, context.byte_order()), context)
	{}

	ombstream(context_base const& context = get_default_context())
		: ombstream{std::make_unique<buffer::sink>(
							util::mutable_buffer{
									context.buffer_size(),
							},
							context.byte_order()),
					context}
	{}

	util::const_buffer
	get_buffer()
	{
		return get_sink().get_buffer();
	}

	util::mutable_buffer&
	get_buffer_ref()
	{
		return get_sink().get_buffer_ref();
	}

	util::const_buffer
	release_buffer()
	{
		return get_sink().release_buffer();
	}

	void
	clear()
	{
		get_sink().clear();
	}

	util::mutable_buffer
	release_mutable_buffer()
	{
		return get_sink().release_mutable_buffer();
	}

	buffer::sink&
	get_sink()
	{
		return reinterpret_cast<buffer::sink&>(*m_sink);
	}
};

}    // namespace bstream

#endif    // BSTREAM_OMBSTREAM_H