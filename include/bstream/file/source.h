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

#ifndef BSTREAM_FILE_SOURCE_H
#define BSTREAM_FILE_SOURCE_H

#include <bstream/source.h>

#ifndef BSTREAM_DEFAULT_FILE_BUFFER_SIZE
#define BSTREAM_DEFAULT_FILE_BUFFER_SIZE 16384UL
#endif

namespace bstream
{
namespace file
{

namespace detail
{
class source_test_probe;
}

class source : public bstream::source
{
public:
	using base = bstream::source;

	friend class file::detail::source_test_probe;

	source(util::size_type  buffer_size = BSTREAM_DEFAULT_FILE_BUFFER_SIZE,
		   byte_order order       = byte_order::big_endian);

	source(std::string const& filename,
		   std::error_code&   err,
		   int                flag_overrides = 0,
		   util::size_type          buffer_size    = BSTREAM_DEFAULT_FILE_BUFFER_SIZE,
		   byte_order         order          = byte_order::big_endian);

	source(std::string const& filename,
		   int                flag_overrides = 0,
		   util::size_type          buffer_size    = BSTREAM_DEFAULT_FILE_BUFFER_SIZE,
		   byte_order         order          = byte_order::big_endian);

	void
	open(std::string const& filename, std::error_code& err, int flag_overrides = 0);

	void
	open(std::string const& filename, int flag_overrides = 0);

	bool
	is_open() const noexcept
	{
		return m_is_open;
	}

	void
	close(std::error_code& err);

	void
	close();

protected:
	virtual util::size_type
	really_underflow(std::error_code& err) override;

	virtual util::size_type
	really_get_size() const override;

	virtual util::position_type
	really_seek(util::position_type pos, std::error_code& err) override;

	virtual util::position_type
	really_get_position() const override;

protected:
	util::size_type
	load_buffer(std::error_code& err);

	void
	reset_ptrs()
	{
		const util::byte_type* base = m_buf.data();
		set_ptrs(base, base, base);
	}

	void
	really_open(std::error_code& err);

	util::mutable_buffer m_buf;
	std::string          m_filename;
	bool                 m_is_open;
	int                  m_flags;
	int                  m_fd;
	util::size_type            m_size;
};

}    // namespace file
}    // namespace bstream

#endif    // BSTREAM_FILE_SOURCE_H