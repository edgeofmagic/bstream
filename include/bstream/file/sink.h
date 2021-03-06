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

#ifndef BSTREAM_FILE_SINK_H
#define BSTREAM_FILE_SINK_H

#include <bstream/sink.h>
#include <util/buffer.h>

#ifndef BSTREAM_DEFAULT_FILE_BUFFER_SIZE
#define BSTREAM_DEFAULT_FILE_BUFFER_SIZE 16384UL
#endif

namespace bstream
{
namespace file
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

	static constexpr open_mode default_mode = open_mode::at_begin;

	sink(sink&& rhs);

	sink(std::string const& filename, open_mode mode, util::size_type buffer_size, byte_order order, std::error_code& err);

	sink(std::string const& filename, open_mode mode, util::size_type buffer_size, byte_order order);

	sink(std::string const& filename, open_mode mode, util::size_type buffer_size, std::error_code& err);

	sink(std::string const& filename, open_mode mode, util::size_type buffer_size);


	sink(open_mode  mode        = default_mode,
		 util::size_type  buffer_size = BSTREAM_DEFAULT_FILE_BUFFER_SIZE,
		 byte_order order       = byte_order::big_endian);


	void
	open(std::string const& filename);

	void
	open(std::string const& filename, std::error_code& err);

	void
	open(std::string const& filename, open_mode mode);

	void
	open(std::string const& filename, open_mode mode, std::error_code& err);

	void
	open(std::string const& filename, open_mode mode, int flags_override, std::error_code& err);

	void
	open(std::string const& filename, open_mode mode, int flags_override);

	bool
	is_open() const noexcept
	{
		return m_is_open;
	}

	void
	close(std::error_code& err);

	void
	close();

	void
	open(std::error_code& err)
	{
		really_open(err);
	}

	void
	open();

	open_mode
	mode() const noexcept
	{
		return m_mode;
	}

	void
	mode(open_mode m)
	{
		m_mode  = m;
		m_flags = to_flags(m);
	}

	int
	flags() const noexcept
	{
		return m_flags;
	}

	void
	flags(int flags)
	{
		m_flags = flags;
	}

	void
	filename(std::string const& filename)
	{
		m_filename = filename;
	}

	std::string const&
	filename() const noexcept
	{
		return m_filename;
	}

	util::position_type
	truncate(std::error_code& err);

	util::position_type
	truncate();

protected:
	virtual void
	really_flush(std::error_code& err) override;

	virtual bool
	is_valid_position(util::position_type pos) const override;

	virtual void
	really_jump(std::error_code& err) override;

	virtual void
	really_overflow(util::size_type, std::error_code& err) override;

private:
	static bool
	is_truncate(int flags);

	static bool
	is_append(int flags);

	void
	reset_ptrs()
	{
		util::byte_type* base = m_buf.data();
		set_ptrs(base, base, base + m_buf.capacity());
	}

	void
	really_open(std::error_code& err);

	static int
	to_flags(open_mode mode);

	util::mutable_buffer m_buf;
	std::string          m_filename;
	bool                 m_is_open;
	open_mode            m_mode;
	int                  m_flags;
	int                  m_fd;
};

}    // namespace file
}    // namespace bstream

#endif    // BSTREAM_FILE_RANDOM_SINK_H