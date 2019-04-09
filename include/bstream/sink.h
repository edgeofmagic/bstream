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

#ifndef BSTREAM_SINK_H
#define BSTREAM_SINK_H

#include <boost/endian/conversion.hpp>
#include <bstream/types.h>
#include <util/buffer.h>
#include <system_error>

namespace bend = boost::endian;


namespace bstream
{
namespace detail
{
class sink_test_probe;
}

/** Base buffer class for binary output streams
 * 
 * This class is designed to be the base class for all binary output stream buffer classes.
 * A binary output stream buffer is associated with a binary output stream object. The stream object
 * delegates write operations to the stream buffer. The stream buffer acts as an intermediary between
 * the stream object and the controlled output sequence.
 * 
 * The controlled output sequence is an abstract sequence of bytes of arbitrary length. The
 * stream buffer maintains an internal byte sequence in memory. The internal byte sequence
 * may hold the output sequence in its entirety, or it may hold a sub-sequence. The buffer
 * maintains four values that internal byte sequence and its relationship to 
 * the output sequence. There are two basic types of values to be considered: positions
 * and pointers. Positions are integral values that indicate an offset relative to the beginning
 * of the output sequence. Pointers are addresses of bytes in the internal byte sequence. 
 * 
 * pbase
 * pbase is a pointer the the first byte in the buffer's internal sequence.
 * 
 * pnext
 * pnext is a pointer the the next available (unwritten) byte in the internal sequence.
 * 
 * pend
 * pend is a pointer to the end of the internal sequence; specifically, it points the 
 * byte following the last useable byte in the internal sequence.
 * 
 * pbase_offset
 * pbase_offset is the position (that is, the offset from the beginning of the output sequence)
 * of the first byte in the internal sequence (at pbase).
 * 
 * The following invariants hold true:
 * pnext >= pbase
 * pend >= pnext
 * pbase_offset >= 0
 * 
 * Also, any pointer value p in the range (pbase <= p < pend) has a corresponding position, computed as 
 * pbase_offset + ( p - pbase ). Certain of these positions are explicitly represented in the stream
 * buffer interface:
 * 
 * ppos
 * ppos is the position corresponding to the pnext pointer, sometimes referred to as the current position
 * of the stream buffer (i.e., the position at which the next write will take place).
 * 
 * Other important operations:
 * 
 * flush
 * The flush operation synchronizes the internal sequence with the output sequence. Derived implementations
 * must provide an implementation of the virtual member function really_flush() that
 * effects synchronization for the specific output sequence type. It is important to note
 * that flush may (but does not necessarily) change the internal sequence pointers or positions.
 * 
 * dirty
 * dirty is a boolean value associated with the stream buffer. dirty is true if the buffer's internal
 * sequence contains values that have not been synchronized (flushed) with the output sequence.
 * After a flush operation, dirty is always false.
 * 
 * dirty_start
 * if dirty is true, dirty_start is a pointer indicating the first byte in the internal sequence
 * that is not synchronized with the output sequence. Unsynchronized byte are always contiguous, 
 * including bytes from dirty_start to pnext - 1. 
 * 
 * Stream buffers support the ability to set the current position explicitly, with a seek operation,
 * A derived implementation may choose not to support seeking, in which seek operations will result
 * in an error condition. The ability to seek forces additional complexity on the stream buffer's
 * interface and behavior.
 * 
 * high_water_mark
 * high_water_mark is a positional value, indicating the largest position in the output 
 * sequence that contains a valid value (essentially, the size of the output sequence).
 * If the current position (ppos) is equal to the high_water_mark, a write operation
 * will cause the high_water_mark value to increase.
 * 
 * touch
 * Some derived implementations (specifically, output file buffers) may permit the 
 * existence of "holes"--regions in the output sequence to which values have not been explicitly
 * written. When a seek operation moves the current position past the high_water_mark position, 
 * and a write occurs, the region of the output sequnce between the previous high_water_mark 
 * and the position of the seek/write operations potentially constitutes such a hole. The 
 * In general, the abstract behavior of any output sequence type should be to fill holes with
 * zero-valued bytes. However, a particular implementation may choose to either write zero-valued
 * bytes to the region, or mark the region as a hole, in which case any future attempt to
 * obtain values from the region of the output sequence will return zero-valued bytes.
 * 
 * To permit implementations freedom of choice in the matter, stream buffers support a touch
 * operation. When a seek operation sets the current position beyond the high_water_mark, the
 * state of the buffer (except for the current position) does not change until a write occurs
 * at the new position. Immediately prior to the actual write, the stream buffer invokes touch
 * (which, in turn, invokes the virtual member function really_touch), allowing the derived
 * implementation to create and/or fill the resulting hole. In essence, the touch operation 
 * announces to the derived implementation that a write operation at the current position
 * is imminent. touch forces a synchronization with the output sequence.
 * 
 * In order to prevent unnecessary invocations of touch, the stream buffer maintains a positional
 * value last_touched. Whenever a synchronization operation (touch or flush) occurs, last_touched 
 * is set to the current position after the synchronization. 
 */

class sink
{
public:
	friend class detail::sink_test_probe;

	sink(util::byte_type* data, util::size_type size, byte_order order = byte_order::big_endian);

	sink(sink const&) = delete;
	sink&
	operator=(sink&&)
			= delete;
	sink&
	operator=(sink const&)
			= delete;

	virtual ~sink() {}

protected:
	sink(byte_order order = byte_order::big_endian);

	sink(sink&& rhs);

public:
	void
	flush(std::error_code& err);

	void
	flush();

	void
	put(util::byte_type byte, std::error_code& err);

	void
	put(util::byte_type byte);

	void
	putn(const util::byte_type* src, util::size_type n, std::error_code& err);

	void
	putn(const util::byte_type* src, util::size_type n);

	void
	putn(util::buffer const& buf)
	{
		putn(buf.data(), buf.size());
	}

	void
	putn(util::buffer const& buf, std::error_code& err)
	{
		putn(buf.data(), buf.size(), err);
	}

	template<class U>
	typename std::enable_if<std::is_arithmetic<U>::value && sizeof(U) == 1>::type
	put_num(U value)
	{
		return put(static_cast<std::uint8_t>(value));
	}

	template<class U>
	typename std::enable_if<std::is_arithmetic<U>::value && sizeof(U) == 1>::type
	put_num(U value, std::error_code& err)
	{
		return put(static_cast<std::uint8_t>(value), err);
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 2)>
	put_num(U value)
	{
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			std::error_code err;
			really_jump(err);
			if (err)
				throw std::system_error{err};
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize);
		}
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 2)>
	put_num(U value, std::error_code& err)
	{
		err.clear();
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			really_jump(err);
			if (err)
				goto exit;
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize, err);
		}

	exit:
		return;
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 4)>
	put_num(U value)
	{
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			std::error_code err;
			really_jump(err);
			if (err)
				throw std::system_error{err};
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize);
		}
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 4)>
	put_num(U value, std::error_code& err)
	{
		err.clear();
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			really_jump(err);
			if (err)
				goto exit;
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize, err);
		}
	exit:
		return;
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 8)>
	put_num(U value)
	{
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			std::error_code err;
			really_jump(err);
			if (err)
				throw std::system_error{err};
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[7];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[6];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[5];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[4];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[4];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[5];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[6];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[7];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize);
		}
	}

	template<class U>
	typename std::enable_if_t<std::is_arithmetic<U>::value && (sizeof(U) == 8)>
	put_num(U value, std::error_code& err)
	{
		err.clear();
		constexpr std::size_t usize = sizeof(U);
		using ctype                 = typename detail::canonical_type<usize>::type;

		using byte_vec = util::byte_type[usize];

		if (m_did_jump)
		{
			really_jump(err);
			if (err)
				goto exit;
		}

		if ((m_end - m_next) >= usize)
		{
			if (!m_dirty)
			{
				m_dirty_start = m_next;
				m_dirty       = true;
			}

			if (m_reverse)
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[7];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[6];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[5];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[4];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
			}
			else
			{
				*m_next++ = reinterpret_cast<byte_vec&>(value)[0];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[1];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[2];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[3];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[4];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[5];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[6];
				*m_next++ = reinterpret_cast<byte_vec&>(value)[7];
			}
		}
		else
		{
			ctype cval = m_reverse ? bend::endian_reverse(reinterpret_cast<ctype&>(value))
								   : reinterpret_cast<ctype&>(value);
			putn(reinterpret_cast<util::byte_type*>(&cval), usize, err);
		}
	exit:
		return;
	}

	void
	filln(const util::byte_type fill_byte, util::size_type n, std::error_code& err);

	void
	filln(const util::byte_type fill_byte, util::size_type n);

	util::position_type
	position(util::position_type pos, std::error_code& err)
	{
		return position(pos, bstream::seek_anchor::begin, err);
	}

	util::position_type
	position(util::position_type pos)
	{
		return position(pos, bstream::seek_anchor::begin);
	}

	util::position_type
	position(util::offset_type offset, seek_anchor where, std::error_code& err);

	util::position_type
	position(util::offset_type offset, seek_anchor where);

	util::position_type
	position() const;

	util::size_type
	size() const
	{
		return really_get_size();
	}

protected:
	void
	set_ptrs(util::byte_type* base, util::byte_type* next, util::byte_type* end)
	{
		m_base = base;
		m_next = next;
		m_end  = end;
	}

	void
	overflow(util::size_type requested, std::error_code& err);

	void
	overflow(util::size_type requested);

	util::position_type
	get_high_watermark() const noexcept
	{
		return m_high_watermark;
	}

	util::position_type
	set_high_watermark()
	{
		if (m_dirty && (ppos() > m_high_watermark))
		{
			m_high_watermark = ppos();
		}
		return m_high_watermark;
	}

	void
	force_high_watermark(util::position_type hwm)
	{
		m_high_watermark = hwm;
	}

	void
	reset_high_water_mark()
	{
		m_high_watermark = 0;
	}

	util::position_type
	ppos() const noexcept
	{
		return m_base_offset + (m_next - m_base);
	}

	void
	really_fill(util::byte_type fill_byte, util::size_type n);

	util::position_type
	new_position(util::offset_type offset, seek_anchor where) const;

protected:    // to be overridden by derived classes
	virtual void
	really_jump(std::error_code& err);

	virtual bool
	is_valid_position(util::position_type pos) const;

	virtual void
	really_overflow(util::size_type, std::error_code& err);

	virtual void
	really_flush(std::error_code& err);

	virtual util::size_type
	really_get_size() const;

protected:
	util::position_type m_base_offset;
	util::position_type m_high_watermark;
	util::position_type m_jump_to;
	util::byte_type*    m_base;
	util::byte_type*    m_next;
	util::byte_type*    m_end;
	util::byte_type*    m_dirty_start;
	bool          m_dirty;
	bool          m_did_jump;
	bool          m_reverse;
};

}    // namespace bstream

#endif    // BSTREAM_SINK_H