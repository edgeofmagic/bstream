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

#ifndef BSTREAM_BUFFER_SOURCE_H
#define BSTREAM_BUFFER_SOURCE_H

#include <bstream/source.h>
#include <bstream/types.h>
#include <util/buffer.h>

namespace bstream
{
namespace buffer
{

namespace detail
{
template<class Buffer>
class source_test_probe;
}

template<class Buffer>
class source : public bstream::source
{
public:
	friend class detail::source_test_probe<Buffer>;

	using bstream::source::getn;
	using base = bstream::source;

	source(byte_order order = byte_order::big_endian) : base{order}, m_buf{} {}

	source(util::shared_buffer const& buf, byte_order order = byte_order::big_endian);

	source(util::shared_buffer&& buf, byte_order order = byte_order::big_endian);

	source(util::const_buffer const& buf, byte_order order = byte_order::big_endian) : base{order}, m_buf{buf}
	{
		set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
	}

	source(util::const_buffer&& buf, byte_order order = byte_order::big_endian) : base{order}, m_buf{std::move(buf)}
	{
		set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
	}

	source(util::mutable_buffer&& buf, byte_order order = byte_order::big_endian) : base{order}, m_buf{std::move(buf)}
	{
		set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
	}

	source(util::buffer const& buf, byte_order order = byte_order::big_endian) : base{order}, m_buf{buf}
	{
		set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
	}

	util::const_buffer
	get_buffer()
	{
		return m_buf;
	}

	util::const_buffer&
	get_buffer_ref()
	{
		return m_buf;
	}

	Buffer
	release_buffer()
	{
		set_ptrs(nullptr, nullptr, nullptr);
		return std::move(m_buf);
	}

	virtual util::shared_buffer
	get_shared_slice(util::size_type n, std::error_code& err) override;

	virtual util::shared_buffer
	get_shared_slice(util::size_type n) override;

	virtual util::const_buffer
	get_slice(util::size_type n, std::error_code& err) override;

	virtual util::const_buffer
	get_slice(util::size_type n) override;

	virtual util::size_type
	really_get_size() const override
	{
		return m_buf.size();
	}

	virtual util::position_type
	really_seek(util::position_type pos, std::error_code& err) override
	{
		err.clear();
		m_next = m_base + pos;
		return pos;
	}

	virtual util::position_type
	really_get_position() const override
	{
		return gpos();
	}

protected:
	Buffer m_buf;
};


template<>
inline source<util::const_buffer>::source(util::shared_buffer const& buf, bstream::byte_order order)
	: base{order}, m_buf{buf}
{
	set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
}

template<>
inline source<util::const_buffer>::source(util::shared_buffer&& buf, bstream::byte_order order)
	: base{order},
	  m_buf{buf}    // construct by copy (as base type buffer); can't move util::shared_buffer to util::const_buffer
{
	set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
}

template<>
inline source<util::shared_buffer>::source(util::shared_buffer const& buf, bstream::byte_order order)
	: base{order}, m_buf{buf}
{
	set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
}

template<>
inline source<util::shared_buffer>::source(util::shared_buffer&& buf, bstream::byte_order order)
	: base{order}, m_buf{std::move(buf)}
{
	set_ptrs(m_buf.data(), m_buf.data(), m_buf.data() + m_buf.size());
}

template<>
inline util::shared_buffer
source<util::const_buffer>::get_shared_slice(util::size_type n, std::error_code& err)
{
	util::shared_buffer result{m_buf, gpos(), n, err};
	if (!err)
	{
		gbump(n);
	}
	return result;
}

template<>
inline util::shared_buffer
source<util::const_buffer>::get_shared_slice(util::size_type n)
{
	util::shared_buffer result{m_buf, gpos(), n};
	gbump(n);
	return result;
}

template<>
inline util::shared_buffer
source<util::shared_buffer>::get_shared_slice(util::size_type n, std::error_code& err)
{
	util::shared_buffer result{m_buf, gpos(), n, err};
	if (!err)
	{
		gbump(n);
	}
	return result;
}

template<>
inline util::shared_buffer
source<util::shared_buffer>::get_shared_slice(util::size_type n)
{
	util::shared_buffer result{m_buf, gpos(), n};
	gbump(n);
	return result;
}

template<>
inline util::const_buffer
source<util::const_buffer>::get_slice(util::size_type n, std::error_code& err)
{
	util::const_buffer result{m_buf, gpos(), n, err};
	if (!err)
	{
		gbump(n);
	}
	return result;
}

template<>
inline util::const_buffer
source<util::const_buffer>::get_slice(util::size_type n)
{
	util::const_buffer result{m_buf, gpos(), n};
	gbump(n);
	return result;
}

template<>
inline util::const_buffer
source<util::shared_buffer>::get_slice(util::size_type n, std::error_code& err)
{
	util::const_buffer result{m_buf, gpos(), n, err};
	if (!err)
	{
		gbump(n);
	}
	return result;
}

template<>
inline util::const_buffer
source<util::shared_buffer>::get_slice(util::size_type n)
{
	util::const_buffer result{m_buf, gpos(), n};
	gbump(n);
	return result;
}

}    // namespace buffer
}    // namespace bstream

#endif    // BSTREAM_BUFFER_SOURCE_H