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

#ifndef BSTREAM_BUFFER_SINK_H
#define BSTREAM_BUFFER_SINK_H

#include <bstream/sink.h>
#include <util/buffer.h>

#ifndef BSTREAM_MEMORY_DEFAULT_BUFFER_SIZE
#define BSTREAM_MEMORY_DEFAULT_BUFFER_SIZE 16384UL
#endif

namespace bstream
{
namespace buffer
{

namespace detail
{
class sink_test_probe;
}

class sink : public bstream::sink
{
public:
	using base = bstream::sink;

	friend class detail::sink_test_probe;

	template<class _Alloc>
	sink(util::size_type size, _Alloc&& alloc, byte_order order = byte_order::big_endian)
		: base{order}, m_buf{size, std::forward<_Alloc>(alloc)}
	{
		reset_ptrs();
	}

	sink(util::size_type size, byte_order order = byte_order::big_endian) : base{order}, m_buf{size}
	{
		reset_ptrs();
	}

	sink(byte_order order) : base{order}, m_buf{BSTREAM_MEMORY_DEFAULT_BUFFER_SIZE} {}

	sink(util::mutable_buffer&& buf, byte_order order = byte_order::big_endian) : base{order}, m_buf{std::move(buf)}
	{
		reset_ptrs();
	}

	sink(sink&&)      = delete;
	sink(sink const&) = delete;
	sink&
	operator=(sink&&)
			= delete;
	sink&
	operator=(sink const&)
			= delete;

	sink&
	clear() noexcept;

	util::const_buffer
	get_buffer();

	util::mutable_buffer&
	get_buffer_ref();

	util::const_buffer
	release_buffer();

	util::mutable_buffer
	release_mutable_buffer();

protected:
	virtual bool
	is_valid_position(util::position_type pos) const override;

	virtual void
	really_overflow(util::size_type, std::error_code& err) override;

	void
	resize(util::size_type size, std::error_code& err);

	void
	reset_ptrs()
	{
		util::byte_type* base = m_buf.data();
		set_ptrs(base, base, base + m_buf.capacity());
	}

	util::mutable_buffer m_buf;
};

}    // namespace buffer
}    // namespace bstream

#endif    // BSTREAM_BUFFER_SINK_H