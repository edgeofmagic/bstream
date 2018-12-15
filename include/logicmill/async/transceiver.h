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

#ifndef LOGICMILL_ASYNC_TRANSCEIVER_H
#define LOGICMILL_ASYNC_TRANSCEIVER_H

#include <chrono>
#include <deque>
#include <functional>
#include <logicmill/async/endpoint.h>
#include <logicmill/bstream/buffer.h>
#include <memory>
#include <system_error>

#ifndef LOGICMILL_ASYNC_TRANSCEIVER_MAX_MSG_SIZE
#define LOGICMILL_ASYNC_TRANSCEIVER_MAX_MSG_SIZE (9216)
#endif

namespace logicmill
{
namespace async
{

class loop;

class transceiver
{
public:
	using ptr                  = std::shared_ptr<transceiver>;

	using receive_handler      = std::function<void(
            transceiver::ptr const& chan,
            bstream::const_buffer&& buf,
            ip::endpoint const&     ep,
            std::error_code         err)>;

	using send_buffer_handler  = std::function<void(
            transceiver::ptr const&   trans,
            bstream::mutable_buffer&& buf,
            ip::endpoint const&       ep,
            std::error_code           err)>;

	using send_buffers_handler = std::function<void(
			transceiver::ptr const&               trans,
			std::deque<bstream::mutable_buffer>&& bufs,
			ip::endpoint const&                   ep,
			std::error_code                       err)>;

	using close_handler        = std::function<void(transceiver::ptr const& chan)>;

	static constexpr std::size_t payload_size_limit = LOGICMILL_ASYNC_TRANSCEIVER_MAX_MSG_SIZE;

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, transceiver::receive_handler>::value>
	start_receive(std::error_code& err, Handler&& handler)
	{
		really_start_receive(err, std::forward<Handler>(handler));
	}

	virtual void
	stop_receive()
			= 0;

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, send_buffer_handler>::value>
	send(bstream::mutable_buffer&& buf, ip::endpoint const& dest, std::error_code& err, Handler&& handler)
	{
		really_send(std::move(buf), dest, err, std::forward<Handler>(handler));
	}

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, send_buffers_handler>::value>
	send(std::deque<bstream::mutable_buffer>&& bufs, ip::endpoint const& dest, std::error_code& err, Handler&& handler)
	{
		really_send(std::move(bufs), dest, err, std::forward<Handler>(handler));
	}

	void
	send(bstream::mutable_buffer&& buf, ip::endpoint const& dest, std::error_code& err)
	{
		really_send(std::move(buf), dest, err, nullptr);
	}

	void
	write(std::deque<bstream::mutable_buffer>&& bufs, ip::endpoint const& dest, std::error_code& err)
	{
		really_send(std::move(bufs), dest, err, nullptr);
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
	is_closing()
			= 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

protected:
	virtual void
	really_start_receive(std::error_code& err, receive_handler&& handler)
			= 0;

	virtual void
	really_start_receive(std::error_code& err, receive_handler const& handler)
			= 0;

	virtual void
	really_send(
			bstream::mutable_buffer&& buf,
			ip::endpoint const&       dest,
			std::error_code&          err,
			send_buffer_handler&&     handler)
			= 0;

	virtual void
	really_send(
			bstream::mutable_buffer&&  buf,
			ip::endpoint const&        dest,
			std::error_code&           err,
			send_buffer_handler const& handler)
			= 0;

	virtual void
	really_send(
			std::deque<bstream::mutable_buffer>&& bufs,
			ip::endpoint const&                   dest,
			std::error_code&                      err,
			send_buffers_handler&&                handler)
			= 0;

	virtual void
	really_send(
			std::deque<bstream::mutable_buffer>&& bufs,
			ip::endpoint const&                   dest,
			std::error_code&                      err,
			send_buffers_handler const&           handler)
			= 0;

	virtual bool
	really_close(close_handler&& handler)
			= 0;

	virtual bool
	really_close(close_handler const& handler)
			= 0;

	// virtual ip::endpoint
	// get_endpoint(std::error_code& err) = 0;

	// virtual ip::endpoint
	// get_peer_endpoint(std::error_code& err) = 0;
};

}    // namespace async
}    // namespace logicmill

#endif    // LOGICMILL_ASYNC_TRANSCEIVER_H