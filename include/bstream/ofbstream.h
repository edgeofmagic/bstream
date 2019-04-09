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

#ifndef BSTREAM_OFBSTREAM_H
#define BSTREAM_OFBSTREAM_H

#include <fstream>
#include <bstream/file/sink.h>
#include <bstream/memory.h>
#include <bstream/obstream.h>
#include <system_error>


namespace bstream
{

// configure, don't open
class ofbstream : public obstream
{
public:
	ofbstream(open_mode mode = file::sink::default_mode, context_base const& context = get_default_context())
		: obstream{std::make_unique<file::sink>(mode, context.buffer_size(), context.byte_order()), context}
	{}

	ofbstream(ofbstream const&) = delete;
	ofbstream(ofbstream&&)      = delete;

	ofbstream(std::unique_ptr<file::sink> fbuf, context_base const& context = get_default_context())
		: obstream{std::move(fbuf), context}
	{}

	ofbstream(std::string const& filename)
		: obstream{std::make_unique<file::sink>(
						   filename,
						   file::sink::default_mode,
						   get_default_context().buffer_size(),
						   get_default_context().byte_order()),
				   get_default_context()}
	{}

	ofbstream(std::string const& filename, std::error_code& err)
		: obstream{std::make_unique<file::sink>(
						   filename,
						   file::sink::default_mode,
						   get_default_context().buffer_size(),
						   get_default_context().byte_order(),
						   err),
				   get_default_context()}
	{}

	ofbstream(std::string const& filename, open_mode mode)
		: obstream{std::make_unique<file::sink>(
						   filename,
						   mode,
						   get_default_context().buffer_size(),
						   get_default_context().byte_order()),
				   get_default_context()}
	{}

	ofbstream(std::string const& filename, open_mode mode, std::error_code& err)
		: obstream{std::make_unique<file::sink>(
						   filename,
						   mode,
						   get_default_context().buffer_size(),
						   get_default_context().byte_order(),
						   err),
				   get_default_context()}
	{}

	ofbstream(std::string const& filename, open_mode mode, context_base const& context)
		: obstream{std::make_unique<file::sink>(filename, mode, context.buffer_size(), context.byte_order()), context}
	{}

	ofbstream(std::string const& filename, open_mode mode, context_base const& context, std::error_code& err)
		: obstream{std::make_unique<file::sink>(filename, mode, context.buffer_size(), context.byte_order(), err), context}
	{}

	void
	open(std::string const& filename, open_mode mode)
	{
		get_filebuf().open(filename, mode);
	}

	void
	open(std::string const& filename, open_mode mode, std::error_code& err)
	{
		get_filebuf().open(filename, mode, err);
	}

	bool
	is_open() const
	{
		return get_filebuf().is_open();
	}

	void
	flush()
	{
		get_filebuf().flush();
	}

	void
	flush(std::error_code& err)
	{
		get_filebuf().flush(err);
	}

	void
	close()
	{
		get_filebuf().close();
	}

	void
	close(std::error_code& err)
	{
		get_filebuf().close(err);
	}

	file::sink&
	get_filebuf()
	{
		return reinterpret_cast<file::sink&>(get_sink());
	}

	file::sink const&
	get_filebuf() const
	{
		return reinterpret_cast<file::sink const&>(get_sink());
	}

	std::unique_ptr<file::sink>
	release_filebuf()
	{
		return bstream::static_unique_ptr_cast<file::sink>(release_sink());
	}

	util::position_type
	truncate(std::error_code& err)
	{
		return get_filebuf().truncate(err);
	}

	util::position_type
	truncate()
	{
		return get_filebuf().truncate();
	}

protected:
};

}    // namespace bstream

#endif    // BSTREAM_OFBSTREAM_H