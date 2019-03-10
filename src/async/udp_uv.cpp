#include "udp_uv.h"
#include "loop_uv.h"
#include <memory>

using namespace logicmill;

udp_transceiver_uv::ptr
udp_send_buf_req_uv::get_transceiver_shared_ptr(uv_udp_send_t* req)
{
	return logicmill::util::dynamic_pointer_cast<udp_transceiver_uv>(udp_transceiver_uv::get_shared_ptr(req->handle));
}

void
udp_send_buf_req_uv::on_send(uv_udp_send_t* req, int status)
{
	auto target = reinterpret_cast<udp_send_buf_req_uv*>(req);
	if (target->m_send_handler)
	{
		std::error_code err = map_uv_error(status);
		target->m_send_handler(get_transceiver_shared_ptr(req), std::move(target->m_buffer), target->m_endpoint, err);
	}
	delete target;
}

/* udp_send_bufs_req_uv */

udp_transceiver_uv::ptr
udp_send_bufs_req_uv::get_transceiver_shared_ptr(uv_udp_send_t* req)
{
	return logicmill::util::dynamic_pointer_cast<udp_transceiver_uv>(udp_transceiver_uv::get_shared_ptr(req->handle));
}

void
udp_send_bufs_req_uv::on_send(uv_udp_send_t* req, int status)
{
	auto target = reinterpret_cast<udp_send_bufs_req_uv*>(req);
	if (target->m_send_handler)
	{
		std::error_code err = map_uv_error(status);
		target->m_send_handler(get_transceiver_shared_ptr(req), std::move(target->m_buffers), target->m_endpoint, err);
	}
	delete target;
}

std::shared_ptr<logicmill::async::loop>
udp_transceiver_uv::loop()
{
	return reinterpret_cast<loop_data*>(reinterpret_cast<uv_handle_t*>(&m_udp_handle)->loop->data)->get_loop_ptr();
}

void
udp_transceiver_uv::init(uv_loop_t* lp, ptr const& self, std::error_code& err)
{
	auto stat = uv_udp_init(lp, get_udp_handle());
	uv_handle_set_data(get_handle(), get_handle_data());
	set_self_ptr(self);
	UV_ERROR_CHECK(stat, err, exit);
exit:
	return;
}

void
udp_transceiver_uv::on_receive(
		uv_udp_t*              udp_handle,
		ssize_t                nread,
		const uv_buf_t*        buf,
		const struct sockaddr* addr,
		unsigned               flags)
{
	ptr transceiver_ptr = logicmill::util::dynamic_pointer_cast<udp_transceiver_uv>(get_shared_ptr(udp_handle));
	assert(transceiver_ptr);
	if (nread < 0)
	{
		if (buf->base)
		{
			delete[] reinterpret_cast<byte_type*>(buf->base);
		}
		transceiver_ptr->m_receive_handler(
				transceiver_ptr,
				util::const_buffer{},
				async::ip::endpoint{*reinterpret_cast<const sockaddr_storage*>(addr)},
				map_uv_error(nread));
	}
	else if (nread == 0)
	{
		if (buf->base)
		{
			delete[] reinterpret_cast<byte_type*>(buf->base);
		}
		if (addr)
		{
			transceiver_ptr->m_receive_handler(
					transceiver_ptr,
					util::const_buffer{},
					async::ip::endpoint{*reinterpret_cast<const sockaddr_storage*>(addr)},
					std::error_code{});
		}
	}
	else if (nread > 0)
	{
		transceiver_ptr->m_receive_handler(
				transceiver_ptr,
				util::const_buffer{reinterpret_cast<byte_type*>(buf->base),
								   static_cast<size_type>(nread),
								   std::default_delete<byte_type[]>{}},
				async::ip::endpoint{*reinterpret_cast<const sockaddr_storage*>(addr)},
				std::error_code{});
	}
}

void
udp_transceiver_uv::on_allocate(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	// static buffer::memory_broker::ptr broker = buffer::default_broker::get();
	// buf->base = reinterpret_cast<char*>(std::allocator<byte_type>{}.allocate(suggested_size));
	buf->base = reinterpret_cast<char*>(new byte_type[suggested_size]);
	buf->len  = suggested_size;
}

bool
udp_transceiver_uv::really_close(transceiver::close_handler&& handler)
{
	bool result{true};
	if (!uv_is_closing(get_handle()))
	{
		m_close_handler = std::move(handler);
		uv_close(get_handle(), udp_transceiver_uv::on_close);
	}
	else
	{
		result = false;
	}
	return result;
}

bool
udp_transceiver_uv::really_close(transceiver::close_handler const& handler)
{
	bool result{true};
	if (!uv_is_closing(get_handle()))
	{
		m_close_handler = handler;
		uv_close(get_handle(), udp_transceiver_uv::on_close);
	}
	else
	{
		result = false;
	}
	return result;
}

bool
udp_transceiver_uv::is_closing()
{
	return uv_is_closing(get_handle());
}

void
udp_transceiver_uv::really_start_receive(std::error_code& err, transceiver::receive_handler&& handler)
{
	err.clear();
	auto stat = uv_udp_recv_start(get_udp_handle(), on_allocate, on_receive);
	if (stat < 0)
	{
		err = map_uv_error(stat);
	}
	else
	{
		m_receive_handler = std::move(handler);
	}
}

void
udp_transceiver_uv::really_start_receive(std::error_code& err, transceiver::receive_handler const& handler)
{
	err.clear();
	auto stat = uv_udp_recv_start(get_udp_handle(), on_allocate, on_receive);
	if (stat < 0)
	{
		err = map_uv_error(stat);
	}
	else
	{
		m_receive_handler = handler;
	}
}

void
udp_transceiver_uv::stop_receive()
{
	uv_udp_recv_stop(get_udp_handle());
}

void
udp_transceiver_uv::really_send(
		util::mutable_buffer&&             buf,
		endpoint const&                    dest,
		std::error_code&                   err,
		transceiver::send_buffer_handler&& handler)
{
	err.clear();
	auto request = new udp_send_buf_req_uv{std::move(buf), dest, std::move(handler)};
	auto status  = request->start(get_udp_handle());
	if (status < 0)
	{
		err = map_uv_error(status);
	}
}

void
udp_transceiver_uv::really_send(
		mutable_buffer&&                        buf,
		endpoint const&                         dest,
		std::error_code&                        err,
		transceiver::send_buffer_handler const& handler)
{
	err.clear();
	auto request = new udp_send_buf_req_uv{std::move(buf), dest, handler};
	auto status  = request->start(get_udp_handle());
	if (status < 0)
	{
		err = map_uv_error(status);
	}
}

void
udp_transceiver_uv::really_send(
		std::deque<mutable_buffer>&&        bufs,
		endpoint const&                     dest,
		std::error_code&                    err,
		transceiver::send_buffers_handler&& handler)
{
	err.clear();
	auto request = new udp_send_bufs_req_uv{std::move(bufs), dest, std::move(handler)};
	auto status  = request->start(get_udp_handle());
	if (status < 0)
	{
		err = map_uv_error(status);
	}
}

void
udp_transceiver_uv::really_send(
		std::deque<mutable_buffer>&&             bufs,
		endpoint const&                          dest,
		std::error_code&                         err,
		transceiver::send_buffers_handler const& handler)
{
	err.clear();
	auto request = new udp_send_bufs_req_uv{std::move(bufs), dest, handler};
	auto status  = request->start(get_udp_handle());
	if (status < 0)
	{
		err = map_uv_error(status);
	}
}
