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

#ifndef LOGICMILL_LAPS_TYPES_H
#define LOGICMILL_LAPS_TYPES_H

#include <deque>
#include <logicmill/async/event_flow.h>
#include <logicmill/bstream/bufseq/source.h>
#include <util/buffer.h>
#include <memory>

namespace logicmill
{
namespace laps
{

namespace flow = ::logicmill::async::event_flow;

enum class event_type
{
	data,
	mutable_data,
	const_data,
	frame,
	mutable_frame,
	const_frame,
	shared_frame,
	control,
	error
};

enum class control_state
{
	stop,
	start
};

using mbuf_sequence = std::deque<util::mutable_buffer>;
using cbuf_sequence = std::deque<util::const_buffer>;
using sbuf_sequence = std::deque<util::shared_buffer>;

inline sbuf_sequence
make_sbuf_sequence(cbuf_sequence&& cseq)
{
	sbuf_sequence result{std::move_iterator<cbuf_sequence::iterator>{cseq.begin()},
						 std::move_iterator<cbuf_sequence::iterator>{cseq.end()}};
	cseq.clear();
	return result;
}

template<class Payload>
using data_event         = flow::event<event_type, event_type::data, Payload>;
using mutable_data_event = data_event<mbuf_sequence&&>;
using const_data_event   = data_event<cbuf_sequence&&>;
using shared_data_event  = data_event<sbuf_sequence&&>;

using mutable_buffer_event = data_event<util::mutable_buffer&&>;
using const_buffer_event   = data_event<util::const_buffer&&>;
using shared_buffer_event  = data_event<util::shared_buffer&&>;

class frame
{
public:
	using flags_type      = std::uint32_t;
	using frame_size_type = std::uint32_t;

	frame_size_type
	size() const
	{
		return m_size;
	}

	flags_type
	flags() const
	{
		return m_flags;
	}

protected:
	frame() : m_size{0}, m_flags{0} {}

	frame(frame const& rhs) : m_size{rhs.m_size}, m_flags{rhs.m_flags} {}

	frame(frame_size_type size, flags_type flags) : m_size{size}, m_flags{flags} {}

	frame_size_type m_size;
	flags_type      m_flags;
};

class mutable_frame : public frame
{
public:
	using buffer_type   = util::mutable_buffer;
	using sequence_type = std::deque<buffer_type>;

	mutable_frame(flags_type flags, sequence_type&& bufs) : frame{0, flags}, m_bufs{std::move(bufs)}
	{
		for (auto& buf : m_bufs)
		{
			m_size += buf.size();
		}
	}

	mutable_frame(mutable_frame&& rhs) : frame{rhs}, m_bufs{std::move(rhs.m_bufs)} {}

	sequence_type const&
	bufs() const
	{
		return m_bufs;
	}

	sequence_type
	release_bufs()
	{
		return std::move(m_bufs);
	}

	bstream::bufseq::source<buffer_type>
	make_source()
	{
		return bstream::bufseq::source<buffer_type>{std::move(m_bufs)};
	}

	util::mutable_buffer
	merge()
	{
		util::mutable_buffer result{m_size};
		util::byte_type*           p{result.data()};
		for (auto& buf : m_bufs)
		{
			::memcpy(p, buf.data(), buf.size());
			p += buf.size();
		}
		return result;
	}

private:
	sequence_type m_bufs;
};

class const_frame : public frame
{
public:
	using buffer_type   = util::const_buffer;
	using sequence_type = std::deque<buffer_type>;

	const_frame(flags_type flags, sequence_type&& bufs) : frame{0, flags}, m_bufs{std::move(bufs)}
	{
		for (auto& buf : m_bufs)
		{
			m_size += buf.size();
		}
	}

	const_frame(const_frame&& rhs) : frame{rhs}, m_bufs{std::move(rhs.m_bufs)} {}

	sequence_type const&
	bufs() const
	{
		return m_bufs;
	}

	sequence_type
	release_bufs()
	{
		return std::move(m_bufs);
	}

	bstream::bufseq::source<buffer_type>
	make_source()
	{
		return bstream::bufseq::source<buffer_type>{std::move(m_bufs)};
	}

	util::const_buffer
	merge()
	{
		util::mutable_buffer result{m_size};
		util::byte_type*           p{result.data()};
		for (auto& buf : m_bufs)
		{
			::memcpy(p, buf.data(), buf.size());
			p += buf.size();
		}
		return util::const_buffer{std::move(result)};
	}

private:
	sequence_type m_bufs;
};


class shared_frame : public frame
{
public:
	using buffer_type   = util::shared_buffer;
	using sequence_type = std::deque<buffer_type>;

	shared_frame(flags_type flags, sequence_type&& bufs) : frame{0, flags}, m_bufs{std::move(bufs)}
	{
		for (auto& buf : m_bufs)
		{
			m_size += buf.size();
		}
	}

	shared_frame(flags_type flags, std::deque<util::const_buffer>&& bufs) : frame{0, flags}, m_bufs{}
	{
		using iter_type = std::deque<util::const_buffer>::iterator;
		for (auto it = bufs.begin(); it != bufs.end(); ++it)
		{
			// std::move_iterator<iter_type> move_it{it};
			m_size += it->size();
			m_bufs.emplace_back(*std::move_iterator<iter_type>{it});
		}
		bufs.clear();
	}

	shared_frame(flags_type flags, std::deque<util::mutable_buffer>&& bufs) : frame{0, flags}, m_bufs{}
	{
		using iter_type = std::deque<util::mutable_buffer>::iterator;
		for (auto it = bufs.begin(); it != bufs.end(); ++it)
		{
			// std::move_iterator<iter_type> move_it{it};
			m_size += it->size();
			m_bufs.emplace_back(*std::move_iterator<iter_type>{it});
		}
		bufs.clear();
	}

	shared_frame(shared_frame&& rhs) : frame{rhs}, m_bufs{std::move(rhs.m_bufs)} {}

	sequence_type const&
	bufs() const
	{
		return m_bufs;
	}

	sequence_type
	release_bufs()
	{
		return std::move(m_bufs);
	}

	bstream::bufseq::source<buffer_type>
	make_source() const
	{
		return bstream::bufseq::source<buffer_type>{std::move(m_bufs)};
	}

	util::shared_buffer
	merge()
	{
		util::mutable_buffer result{m_size};
		util::byte_type*           p{result.data()};
		for (auto& buf : m_bufs)
		{
			::memcpy(p, buf.data(), buf.size());
			p += buf.size();
		}
		return util::shared_buffer{std::move(result)};
	}

private:
	sequence_type m_bufs;
};


template<class Payload>
using frame_event         = flow::event<event_type, event_type::frame, Payload>;
using mutable_frame_event = frame_event<mutable_frame&&>;
using shared_frame_event  = frame_event<shared_frame&&>;

using control_event = flow::event<event_type, event_type::control, control_state>;
using error_event   = flow::event<event_type, event_type::error, std::error_code>;

using mutable_buffer_in_connector
		= flow::connector<flow::sink<mutable_buffer_event>, flow::source<control_event>, flow::source<error_event>>;
using mutable_buffer_out_connector = flow::complement<mutable_buffer_in_connector>::type;

using const_buffer_in_connector
		= flow::connector<flow::sink<const_buffer_event>, flow::source<control_event>, flow::source<error_event>>;
using const_buffer_out_connector = flow::complement<const_buffer_in_connector>::type;

using shared_buffer_in_connector
		= flow::connector<flow::sink<shared_buffer_event>, flow::source<control_event>, flow::source<error_event>>;
using shared_buffer_out_connector = flow::complement<shared_buffer_in_connector>::type;

using mutable_data_in_connector = flow::connector<
		flow::sink<mutable_data_event>,
		flow::sink<mutable_buffer_event>,
		flow::source<control_event>,
		flow::source<error_event>>;
using mutable_data_out_connector = flow::complement<mutable_data_in_connector>::type;

using const_data_in_connector
		= flow::connector<flow::sink<const_data_event>, flow::source<control_event>, flow::source<error_event>>;
using const_data_out_connector = flow::complement<const_data_in_connector>::type;

using shared_data_in_connector
		= flow::connector<flow::sink<shared_data_event>, flow::source<control_event>, flow::sink<error_event>>;
using shared_data_out_connector = flow::complement<shared_data_in_connector>::type;

using mutable_frame_in_connector
		= flow::connector<flow::sink<mutable_frame_event>, flow::source<control_event>, flow::source<error_event>>;
using mutable_frame_out_connector = flow::complement<mutable_frame_in_connector>::type;

using shared_frame_in_connector
		= flow::connector<flow::sink<shared_frame_event>, flow::source<control_event>, flow::source<error_event>>;
using shared_frame_out_connector = flow::complement<shared_frame_in_connector>::type;

using buffer_duplex_top    = flow::surface<mutable_buffer_in_connector, const_buffer_out_connector>;
using buffer_duplex_bottom = flow::complement<buffer_duplex_top>::type;

using stream_duplex_top    = flow::surface<mutable_data_in_connector, const_buffer_out_connector>;
using stream_duplex_bottom = flow::complement<stream_duplex_top>::type;
using frame_duplex_top     = flow::surface<mutable_frame_in_connector, shared_frame_out_connector>;
using frame_duplex_bottom  = flow::complement<frame_duplex_top>::type;

template<class T, class Derived>
class face
{
public:
	face(T* owner) : m_owner{owner}, m_is_reading{false}, m_is_writable{false} {}

	void
	propagate_start()
	{
		if (!m_is_reading)
		{
			m_is_reading = true;
			static_cast<Derived*>(this)->template emit<control_event>(control_state::start);
		}
	}

	void
	propagate_stop()
	{
		if (m_is_reading)
		{
			m_is_reading = false;
			static_cast<Derived*>(this)->template emit<control_event>(control_state::stop);
		}
	}

	bool
	is_reading() const
	{
		return m_is_reading;
	}

	void
	set_reading(bool value)
	{
		m_is_reading = value;
	}

	bool
	is_writable() const
	{
		return m_is_writable;
	}

	void
	set_writable(bool value)
	{
		m_is_writable = value;
	}

	T*
	owner()
	{
		return m_owner;
	}

protected:
	bool m_is_writable;
	bool m_is_reading;
	T*   m_owner;
};

enum class errc
{
	ok = 0,
	not_writable,
	already_reading,
	not_reading,
	cannot_resume_read
};

std::error_category const&
error_category() noexcept;

std::error_condition
make_error_condition(errc e);

std::error_code
make_error_code(errc e);

}    // namespace laps
}    // namespace logicmill


namespace std
{

template<>
struct is_error_condition_enum<logicmill::laps::errc> : public true_type
{};

}    // namespace std

#endif    // LOGICMILL_LAPS_TYPES_H