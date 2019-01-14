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

#ifndef LOGICMILL_ASYNC_CHANNEL_H
#define LOGICMILL_ASYNC_CHANNEL_H

#include <chrono>
#include <deque>
#include <functional>
#include <logicmill/buffer.h>
#include <logicmill/async/endpoint.h>
#include <logicmill/util/shared_ptr.h>
#include <memory>
#include <system_error>

namespace logicmill
{
namespace async
{

class loop;

class channel
{
public:
	using ptr = util::shared_ptr<channel>;

	using read_handler
			= std::function<void(channel::ptr const& chan, const_buffer&& buf, std::error_code err)>;

	using write_buffer_handler
			= std::function<void(channel::ptr const& chan, mutable_buffer&& buf, std::error_code err)>;

	using write_buffers_handler = std::function<
			void(channel::ptr const& chan, std::deque<mutable_buffer>&& bufs, std::error_code err)>;

	using connect_handler = std::function<void(channel::ptr const& chan, std::error_code err)>;

	using close_handler = std::function<void(channel::ptr const& chan)>;

	virtual ~channel() {}

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, read_handler>::value>
	start_read(std::error_code& err, Handler&& handler)
	{
		really_start_read(err, std::forward<Handler>(handler));
	}

	virtual void
	stop_read() = 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, write_buffer_handler>::value>
	write(mutable_buffer&& buf, std::error_code& err, Handler&& handler)
	{
		really_write(std::move(buf), err, std::forward<Handler>(handler));
	}

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, write_buffers_handler>::value>
	write(std::deque<mutable_buffer>&& bufs, std::error_code& err, Handler&& handler)
	{
		really_write(std::move(bufs), err, std::forward<Handler>(handler));
	}

	void
	write(mutable_buffer&& buf, std::error_code& err)
	{
		really_write(std::move(buf), err, nullptr);
	}

	void
	write(std::deque<mutable_buffer>&& bufs, std::error_code& err)
	{
		really_write(std::move(bufs), err, nullptr);
	}

	template<class H>
	typename std::enable_if_t<std::is_convertible<H, close_handler>::value, bool>
	close(H&& handler)
	{
		return really_close(std::forward<H>(handler));
	}

	bool
	close()
	{
		return really_close(nullptr);
	}

	virtual bool
	is_closing() = 0;

	virtual ip::endpoint
	get_endpoint(std::error_code& err) = 0;

	virtual ip::endpoint
	get_peer_endpoint(std::error_code& err) = 0;

	virtual std::size_t
	get_queue_size() const = 0;

protected:
	virtual void
	really_write(mutable_buffer&& buf, std::error_code& err, write_buffer_handler&& handler) = 0;

	virtual void
	really_write(mutable_buffer&& buf, std::error_code& err, write_buffer_handler const& handler) = 0;

	virtual void
	really_write(std::deque<mutable_buffer>&& bufs, std::error_code& err, write_buffers_handler&& handler) = 0;

	virtual void
	really_write(std::deque<mutable_buffer>&& bufs, std::error_code& err, write_buffers_handler const& handler) = 0;

	virtual bool
	really_close(close_handler&& handler) = 0;

	virtual bool
	really_close(close_handler const& handler) = 0;

	virtual void
	really_start_read(std::error_code& err, read_handler&& handler) = 0;

	virtual void
	really_start_read(std::error_code& err, read_handler const& handler) = 0;
};

class acceptor
{
public:
	using ptr = util::shared_ptr<acceptor>;
	using connection_handler
			= std::function<void(acceptor::ptr const& sp, channel::ptr const& chan, std::error_code err)>;
	using close_handler = std::function<void(acceptor::ptr const& lp)>;

	virtual ~acceptor() {}

	template<class H>
	typename std::enable_if_t<std::is_convertible<H, close_handler>::value, bool>
	close(H&& handler)
	{
		return really_close(std::forward<H>(handler));
	}

	bool
	close()
	{
		return really_close(nullptr);
	}

	virtual ip::endpoint
	get_endpoint(std::error_code& err) = 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

protected:
	virtual bool
	really_close(close_handler&& handler) = 0;

	virtual bool
	really_close(close_handler const& handler) = 0;
};

}    // namespace async
}    // namespace logicmill

#endif    // LOGICMILL_ASYNC_CHANNEL_H