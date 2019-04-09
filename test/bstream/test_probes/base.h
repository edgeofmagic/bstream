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

#ifndef TEST_BSTREAM_TEST_PROBES_BASE_H
#define TEST_BSTREAM_TEST_PROBES_BASE_H

#include <bstream/sink.h>
#include <bstream/source.h>


namespace bstream
{

namespace detail
{

class source_test_probe
{
public:
	source_test_probe(bstream::source& target) : m_target{target} {}

	util::position_type
	base_offset()
	{
		return m_target.m_base_offset;
	}

	util::position_type
	pos()
	{
		return m_target.gpos();
	}

	const util::byte_type*
	base()
	{
		return m_target.m_base;
	}

	const util::byte_type*
	next()
	{
		return m_target.m_next;
	}

	const util::byte_type*
	end()
	{
		return m_target.m_end;
	}

private:
	bstream::source& m_target;
};

class sink_test_probe
{
public:
	sink_test_probe(bstream::sink& target) : m_target{target} {}

	util::position_type
	base_offset()
	{
		return m_target.m_base_offset;
	}

	util::position_type
	hwm()
	{
		return m_target.m_high_watermark;
	}

	bool
	dirty()
	{
		return m_target.m_dirty;
	}

	util::byte_type*
	dirty_start()
	{
		return m_target.m_dirty_start;
	}

	util::size_type
	dirty_start_position()
	{
		return m_target.m_dirty_start - m_target.m_base;
	}

	bool
	did_jump()
	{
		return m_target.m_did_jump;
	}

	util::size_type
	jump_to()
	{
		return m_target.m_jump_to;
	}

	util::position_type
	pos()
	{
		return m_target.ppos();
	}

	util::byte_type*
	base()
	{
		return m_target.m_base;
	}

	util::byte_type*
	next()
	{
		return m_target.m_next;
	}

	util::byte_type*
	end()
	{
		return m_target.m_end;
	}

private:
	bstream::sink& m_target;
};

}    // namespace detail
}    // namespace bstream

#endif    // TEST_BSTREAM_TEST_PROBES_BASE_H