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

#ifndef LOGICMILL_ASYNC_TCP_UV_H
#define LOGICMILL_ASYNC_TCP_UV_H

#include "uv_error.h"
#include <boost/endian/conversion.hpp>
#include <logicmill/async/endpoint.h>
#include <logicmill/async/options.h>
#include <logicmill/async/tcp.h>
#include <uv.h>

class tcp_channel_uv;
class tcp_acceptor_uv;

using logicmill::async::ip::endpoint;

class connect_request_uv
{
public:
	template<
			class Handler,
			class = std::enable_if_t<std::is_convertible<Handler, logicmill::async::channel::connect_handler>::value>>
	connect_request_uv(Handler&& handler) : m_handler{std::forward<Handler>(handler)}
	{
		assert(reinterpret_cast<uv_connect_t*>(this) == &m_uv_connect_request);
	}

	uv_connect_t*
	get_uv_connect_request()
	{
		return &m_uv_connect_request;
	}

	void
	clear_handler()
	{
		m_handler = nullptr;
	}

	static void
	on_connect(uv_connect_t* req, int status);

private:
	uv_connect_t                               m_uv_connect_request;
	logicmill::async::channel::connect_handler m_handler;

	static std::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_connect_t* req);
};

class tcp_write_buf_req_uv
{
public:
	template<
			class Handler,
			class = std::enable_if_t<
					std::is_convertible<Handler, logicmill::async::channel::write_buffer_handler>::value>>
	tcp_write_buf_req_uv(logicmill::bstream::mutable_buffer&& buf, Handler&& handler)
		: m_write_handler{std::forward<Handler>(handler)},
		  m_buffer{std::move(buf)},
		  m_uv_buffer{reinterpret_cast<char*>(m_buffer.data()), m_buffer.size()}
	{
		assert(reinterpret_cast<uv_write_t*>(this) == &m_uv_write_request);
	}

	int
	start(uv_stream_t* chan)
	{
		return uv_write(&m_uv_write_request, chan, &m_uv_buffer, 1, on_write);
	}

private:
	~tcp_write_buf_req_uv() {}

	static std::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_write_t* req);

	static void
	on_write(uv_write_t* req, int status);

	uv_write_t                                      m_uv_write_request;
	logicmill::bstream::mutable_buffer              m_buffer;
	uv_buf_t                                        m_uv_buffer;
	logicmill::async::channel::write_buffer_handler m_write_handler;
};


class tcp_write_bufs_req_uv
{
public:
	template<
			class Handler,
			class = std::enable_if_t<
					std::is_convertible<Handler, logicmill::async::channel::write_buffers_handler>::value>>
	tcp_write_bufs_req_uv(std::deque<logicmill::bstream::mutable_buffer>&& bufs, Handler&& handler)
		: m_write_handler{std::forward<Handler>(handler)},
		  m_buffers{std::move(bufs)},
		  m_uv_buffers{new uv_buf_t[m_buffers.size()]}
	{
		assert(reinterpret_cast<uv_write_t*>(this) == &m_uv_write_request);

		std::size_t i = 0;
		for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it)
		{
			m_uv_buffers[i].base = reinterpret_cast<char*>(it->data());
			m_uv_buffers[i].len  = it->size();
			++i;
		}
	}

	int
	start(uv_stream_t* chan)
	{
		return uv_write(&m_uv_write_request, chan, m_uv_buffers, m_buffers.size(), on_write);
	}

private:
	~tcp_write_bufs_req_uv()
	{
		if (m_uv_buffers)
		{
			delete[] m_uv_buffers;
		}
	}

	static std::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_write_t* req);

	static void
	on_write(uv_write_t* req, int status);

	uv_write_t                                       m_uv_write_request;
	std::deque<logicmill::bstream::mutable_buffer>   m_buffers;
	uv_buf_t*                                        m_uv_buffers;
	logicmill::async::channel::write_buffers_handler m_write_handler;
};

class tcp_base_uv
{
public:
	static std::shared_ptr<tcp_base_uv>
	get_base_shared_ptr(uv_stream_t* stream_handle)
	{
		return get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(stream_handle));
	}

	static void
	on_close(uv_handle_t* handle)
	{
		assert(uv_handle_get_type(handle) == uv_handle_type::UV_TCP);
		auto tcp_base = get_base_raw_ptr(handle);
		assert(tcp_base->get_handle() == handle);
		tcp_base->clear();
	}

	const uv_stream_t*
	get_stream_handle() const
	{
		return reinterpret_cast<const uv_stream_t*>(&m_tcp_handle);
	}

	uv_stream_t*
	get_stream_handle()
	{
		return reinterpret_cast<uv_stream_t*>(&m_tcp_handle);
	}

	endpoint
	really_get_endpoint(std::error_code& err);

protected:
	using ptr = std::shared_ptr<tcp_base_uv>;

	struct handle_data
	{
		ptr m_self_ptr;
	};

	virtual ~tcp_base_uv() {}

	tcp_base_uv*
	get_self_raw_ptr() const
	{
		return m_data.m_self_ptr.get();
	}

	void
	set_self_ptr(ptr const& self)
	{
		m_data.m_self_ptr = self;
	}

	handle_data*
	get_handle_data()
	{
		return &m_data;
	}

	static handle_data*
	get_handle_data(uv_handle_t* handle)
	{
		return reinterpret_cast<handle_data*>(handle->data);
	}

	static ptr
	get_base_shared_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr;
	}

	static uv_handle_t*
	get_handle_indirect(uv_handle_t* handle)
	{
		return reinterpret_cast<uv_handle_t*>(&(get_handle_data(handle)->m_self_ptr->m_tcp_handle));
	}

	static tcp_base_uv*
	get_base_raw_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr.get();
	}

	uv_tcp_t*
	get_tcp_handle()
	{
		return &m_tcp_handle;
	}

	uv_handle_t*
	get_handle()
	{
		return reinterpret_cast<uv_handle_t*>(&m_tcp_handle);
	}

	std::shared_ptr<logicmill::async::loop>
	get_loop();

	virtual void
	clear_handler()
			= 0;

	void
	clear()
	{
		clear_handler();
		m_data.m_self_ptr.reset();
	}

	handle_data m_data;
	uv_tcp_t    m_tcp_handle;
};

class tcp_channel_uv : public tcp_base_uv, public logicmill::async::tcp_channel
{
public:
	using ptr = std::shared_ptr<tcp_channel_uv>;

	void
	init(uv_loop_t* lp, ptr const& self, std::error_code& err);

	template<class Handler>
	typename std::enable_if_t<std::is_convertible<Handler, logicmill::async::channel::connect_handler>::value>
	connect(logicmill::async::ip::endpoint const& ep, std::error_code& err, Handler&& handler)
	{
		err.clear();
		sockaddr_storage saddr;
		ep.to_sockaddr(saddr);
		auto req  = new connect_request_uv(std::forward<Handler>(handler));
		int  stat = uv_tcp_connect(
                req->get_uv_connect_request(),
                get_tcp_handle(),
                reinterpret_cast<sockaddr*>(&saddr),
                connect_request_uv::on_connect);
		if (stat < 0)
		{
			err = map_uv_error(stat);
			if (!uv_is_active(get_handle()))
			{
				req->clear_handler();
				delete req;
			}
		}
	}

	virtual endpoint
	get_endpoint(std::error_code& err) override
	{
		return really_get_endpoint(err);
	}

	virtual endpoint
	get_peer_endpoint(std::error_code& err) override;

	virtual std::size_t
	get_queue_size() const override
	{
		return get_stream_handle()->write_queue_size;
	}

protected:
	virtual void
	clear_handler() override;

	static void
	on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

	static void
	on_allocate(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

	static void
	on_close(uv_handle_t* handle);

	virtual void
	really_start_read(std::error_code& err, logicmill::async::channel::read_handler&& handler) override;

	virtual void
	really_start_read(std::error_code& err, logicmill::async::channel::read_handler const& handler) override;

	virtual void
	stop_read() override;

	virtual std::shared_ptr<logicmill::async::loop>
	loop() override
	{
		return get_loop();
	}

	virtual bool
	is_closing() override;

	virtual void
	really_write(
			logicmill::bstream::mutable_buffer&&              buf,
			std::error_code&                                  err,
			logicmill::async::channel::write_buffer_handler&& handler) override;

	virtual void
	really_write(
			logicmill::bstream::mutable_buffer&&                   buf,
			std::error_code&                                       err,
			logicmill::async::channel::write_buffer_handler const& handler) override;

	virtual void
	really_write(
			std::deque<logicmill::bstream::mutable_buffer>&&   bufs,
			std::error_code&                                   err,
			logicmill::async::channel::write_buffers_handler&& handler) override;

	virtual void
	really_write(
			std::deque<logicmill::bstream::mutable_buffer>&&        bufs,
			std::error_code&                                        err,
			logicmill::async::channel::write_buffers_handler const& handler) override;

	virtual bool
	really_close(logicmill::async::channel::close_handler&& handler) override;

	virtual bool
	really_close(logicmill::async::channel::close_handler const& handler) override;

	logicmill::async::channel::read_handler  m_read_handler;
	logicmill::async::channel::close_handler m_close_handler;
};

class tcp_framed_channel_uv : public tcp_channel_uv
{
public:
	tcp_framed_channel_uv() : m_header_byte_count{0}, m_frame_size{-1} {}

private:
	using ptr                           = std::shared_ptr<tcp_framed_channel_uv>;
	using frame_size_type               = std::int64_t;
	static constexpr bool reverse_order = boost::endian::order::native != boost::endian::order::big;

	static logicmill::bstream::mutable_buffer
	pack_frame_header(std::uint64_t frame_size)
	{
		std::uint64_t packed_frame_size = boost::endian::native_to_big(frame_size);
		return logicmill::bstream::mutable_buffer{&packed_frame_size, sizeof(frame_size)};
	}

	static std::uint64_t
	unpack_frame_header(const void* buf_ptr)
	{
		std::uint64_t packed_frame_size{0};
		::memcpy(&packed_frame_size, buf_ptr, sizeof(packed_frame_size));
		return boost::endian::big_to_native(packed_frame_size);
	}

	static void
	on_read(uv_stream_t* stream_handle, ssize_t nread, const uv_buf_t* buf);

	virtual void
	really_start_read(std::error_code& err, logicmill::async::channel::read_handler&& handler) override;

	virtual void
	really_start_read(std::error_code& err, logicmill::async::channel::read_handler const& handler) override;

	virtual void
	really_write(
			logicmill::bstream::mutable_buffer&&              buf,
			std::error_code&                                  err,
			logicmill::async::channel::write_buffer_handler&& handler) override;

	virtual void
	really_write(
			logicmill::bstream::mutable_buffer&&                   buf,
			std::error_code&                                       err,
			logicmill::async::channel::write_buffer_handler const& handler) override;

	virtual void
	really_write(
			std::deque<logicmill::bstream::mutable_buffer>&&   bufs,
			std::error_code&                                   err,
			logicmill::async::channel::write_buffers_handler&& handler) override;

	virtual void
	really_write(
			std::deque<logicmill::bstream::mutable_buffer>&&        bufs,
			std::error_code&                                        err,
			logicmill::async::channel::write_buffers_handler const& handler) override;


	void
	read_to_frame(ptr channel_ptr, logicmill::bstream::const_buffer&& buf);

	bool
	is_frame_size_valid() const
	{
		return m_frame_size >= 0;
	}

	bool
	is_header_complete() const
	{
		assert(m_header_byte_count <= sizeof(frame_size_type));
		return m_header_byte_count == sizeof(frame_size_type);
	}

	std::size_t                        m_header_byte_count;
	frame_size_type                    m_frame_size;
	logicmill::bstream::byte_type      m_header_buf[sizeof(frame_size_type)];
	logicmill::bstream::mutable_buffer m_payload_buffer;
};

class tcp_acceptor_uv : public tcp_base_uv, public logicmill::async::tcp_acceptor
{
public:
	using ptr = std::shared_ptr<tcp_acceptor_uv>;

	template<
			class Handler,
			class = std::enable_if_t<
					std::is_convertible<Handler, logicmill::async::acceptor::connection_handler>::value>>
	tcp_acceptor_uv(logicmill::async::ip::endpoint const& ep, Handler&& handler)
		: m_endpoint{ep}, m_connection_handler{std::forward<Handler>(handler)}
	{}

	void
	init(uv_loop_t* lp, ptr const& self, logicmill::async::options const& opt, std::error_code& err);

	virtual void
	clear_handler() override
	{
		if (m_close_handler)
		{
			m_close_handler(std::dynamic_pointer_cast<tcp_acceptor_uv>(m_data.m_self_ptr));
			m_close_handler = nullptr;
		}
		m_connection_handler = nullptr;
	}

	virtual endpoint
	get_endpoint(std::error_code& err) override
	{
		return really_get_endpoint(err);
	}

private:
	static ptr
	get_shared_acceptor(uv_stream_t* handle)
	{
		return std::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(handle)));
	}

	static ptr
	get_shared_acceptor(uv_tcp_t* handle)
	{
		return std::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(handle)));
	}

	static ptr
	get_shared_acceptor(uv_handle_t* handle)
	{
		return std::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(handle));
	}

	static tcp_acceptor_uv*
	get_acceptor_impl_raw(uv_handle_t* handle)
	{
		return std::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(handle)).get();
	}

	static void
	on_connection(uv_stream_t* handle, int stat);

	static void
	on_framing_connection(uv_stream_t* handle, int stat);

	virtual std::shared_ptr<logicmill::async::loop>
	loop() override
	{
		return get_loop();
	}

	virtual bool
	really_close(logicmill::async::acceptor::close_handler&& handler) override;

	virtual bool
	really_close(logicmill::async::acceptor::close_handler const& handler) override;

	logicmill::async::ip::endpoint                 m_endpoint;
	logicmill::async::acceptor::connection_handler m_connection_handler;
	logicmill::async::acceptor::close_handler      m_close_handler;
};

#endif /* LOGICMILL_ASYNC_TCP_UV_H */