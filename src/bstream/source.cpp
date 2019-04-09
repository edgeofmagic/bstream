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

#include <logicmill/bstream/error.h>
#include <logicmill/bstream/source.h>

using namespace logicmill;
using namespace bstream;

util::position_type
source::really_seek(util::position_type pos, std::error_code& err)
{
	err.clear();
	m_next = m_base + pos;
	return pos;
}

util::position_type
source::really_get_position() const
{
	return gpos();
}

util::size_type
source::really_underflow(std::error_code& err)
{
	err.clear();
	m_next = m_end;
	return 0UL;
}

void
source::really_rewind()
{
	m_next = m_base;
}

util::size_type
source::really_get_size() const
{
	return m_end - m_base;
}

util::byte_type
source::get(std::error_code& err)
{
	err.clear();
	util::byte_type result = 0;
	if (m_next >= m_end)
	{
		assert(m_next == m_end);
		auto available = underflow(err);
		if (err)
			goto exit;
		if (available < 1)
		{
			err = make_error_code(bstream::errc::read_past_end_of_stream);
			goto exit;
		}
	}
	assert(m_next < m_end);
	result = *m_next++;
exit:
	return result;
}

util::byte_type
source::get()
{
	if (m_next >= m_end)
	{
		assert(m_next == m_end);
		if (underflow() < 1)
		{
			throw std::system_error{make_error_code(bstream::errc::read_past_end_of_stream)};
		}
	}
	assert(m_next < m_end);
	return *m_next++;
}

util::byte_type
source::peek(std::error_code& err)
{
	err.clear();
	util::byte_type result = 0;
	if (m_next >= m_end)
	{
		assert(m_next == m_end);
		auto available = underflow(err);
		if (err)
			goto exit;
		if (available < 1)
		{
			err = make_error_code(bstream::errc::read_past_end_of_stream);
			goto exit;
		}
	}
	assert(m_next < m_end);
	result = *m_next;
exit:
	return result;
}

util::byte_type
source::peek()
{
	if (m_next >= m_end)
	{
		assert(m_next == m_end);
		if (underflow() < 1)
		{
			throw std::system_error{make_error_code(bstream::errc::read_past_end_of_stream)};
		}
	}
	assert(m_next < m_end);
	return *m_next;
}

util::shared_buffer
source::get_shared_slice(util::size_type n, std::error_code& err)
{
	err.clear();
	if (n < 1)
	{
		return util::shared_buffer{};
	}

	util::mutable_buffer buf{n};
	auto                 got = getn(buf.data(), n, err);
	if (err)
	{
		return util::shared_buffer{};
	}
	else
	{
		assert(n == got);
		buf.size(got);
		return util::shared_buffer{std::move(buf)};
	}
}

util::shared_buffer
source::get_shared_slice(util::size_type n)
{
	if (n < 1)
	{
		return util::shared_buffer{};
	}

	util::mutable_buffer buf{n};
	auto                 got = getn(buf.data(), n);
	buf.size(got);
	return util::shared_buffer{std::move(buf)};
}

util::const_buffer
source::get_slice(util::size_type n, std::error_code& err)
{
	err.clear();
	if (n < 1)
	{
		return util::const_buffer{};
	}

	util::mutable_buffer buf{n};
	auto                 got = getn(buf.data(), n, err);
	if (err)
	{
		return util::const_buffer{};
	}
	else
	{
		assert(n == got);
		buf.size(got);
		return util::const_buffer{std::move(buf)};
	}
}

util::const_buffer
source::get_slice(util::size_type n)
{
	if (n < 1)
	{
		return util::const_buffer{};
	}

	util::mutable_buffer buf{n};
	auto                 got = getn(buf.data(), n);
	buf.size(got);
	return util::const_buffer{std::move(buf)};
}

util::size_type
source::getn(util::byte_type* dst, util::size_type n, std::error_code& err)
{
	err.clear();
	util::size_type result = 0;
	if (n < 1)
	{
		goto exit;
	}

	// optimize for the available case
	if (n < m_end - m_next)
	{
		::memcpy(dst, m_next, n);
		m_next += n;
		result = n;
	}
	else
	{
		util::byte_type* p    = dst;
		util::byte_type* endp = dst + n;
		while (p < endp)
		{
			if (m_next >= m_end)
			{
				assert(m_next == m_end);
				auto available = underflow(err);
				if (err)
					goto exit;
				if (available < 1)
				{
					err = make_error_code(bstream::errc::read_past_end_of_stream);
					goto exit;
				}
			}
			util::size_type chunk_size = std::min(static_cast<util::size_type>(m_end - m_next), static_cast<util::size_type>(endp - p));
			if (chunk_size < 1)
				break;
			::memcpy(p, m_next, chunk_size);
			p += chunk_size;
			m_next += chunk_size;
		}
		result = static_cast<util::size_type>(p - dst);
	}
exit:
	return result;
}

util::size_type
source::getn(util::byte_type* dst, util::size_type n)
{
	util::size_type result = 0;
	if (n < 1)
	{
		goto exit;
	}
	// optimize for the available case
	if (n < m_end - m_next)
	{
		::memcpy(dst, m_next, n);
		m_next += n;
		result = n;
	}
	else
	{
		util::byte_type* p    = dst;
		util::byte_type* endp = dst + n;
		while (p < endp)
		{
			if (m_next >= m_end)
			{
				assert(m_next == m_end);
				if (underflow() < 1)
				{
					throw std::system_error{make_error_code(bstream::errc::read_past_end_of_stream)};
				}
			}
			util::size_type chunk_size = std::min(static_cast<util::size_type>(m_end - m_next), static_cast<util::size_type>(endp - p));
			if (chunk_size < 1)
				break;
			::memcpy(p, m_next, chunk_size);
			p += chunk_size;
			m_next += chunk_size;
		}
		result = static_cast<util::size_type>(p - dst);
	}
exit:
	return result;
}

util::size_type
source::underflow()
{
	std::error_code err;
	auto            available = really_underflow(err);
	if (err)
	{
		throw std::system_error{err};
	}
	return available;
}

util::position_type
source::new_position(util::offset_type offset, seek_anchor where) const
{
	util::position_type result = util::npos;

	switch (where)
	{
		case seek_anchor::current:
			result = really_get_position() + offset;
			break;

		case seek_anchor::end:
			result = really_get_size() + offset;
			break;

		case seek_anchor::begin:
			result = offset;
			break;
	}

	return result;
}

util::position_type
source::position(util::offset_type offset, seek_anchor where, std::error_code& err)
{
	err.clear();
	util::position_type result = new_position(offset, where);

	if (result < 0 || result > (really_get_size()))
	{
		err    = make_error_code(std::errc::invalid_seek);
		result = util::npos;
		goto exit;
	}

	result = really_seek(result, err);
	if (err)
		goto exit;

exit:
	return result;
}

util::position_type
source::position(util::position_type pos)
{
	std::error_code err;
	auto            result = position(static_cast<util::offset_type>(pos), seek_anchor::begin, err);
	if (err)
	{
		throw std::system_error{err};
	}
	return result;
}

util::position_type
source::position(util::offset_type offset, seek_anchor where)
{
	std::error_code err;
	auto            result = position(offset, where, err);
	if (err)
	{
		throw std::system_error{err};
	}
	return result;
}

util::position_type
source::position(util::position_type pos, std::error_code& err)
{
	return position(static_cast<util::offset_type>(pos), seek_anchor::begin, err);
}

util::position_type
source::position() const
{
	return really_get_position();
}
