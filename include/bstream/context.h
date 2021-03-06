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

#ifndef BSTREAM_CONTEXT_H
#define BSTREAM_CONTEXT_H

#include <boost/endian/conversion.hpp>
#include <bstream/error.h>
#include <util/error.h>
#include <util/error_context.h>
#include <bstream/ibstream_traits.h>
#include <bstream/types.h>
#include <util/buffer.h>
#include <util/shared_ptr.h>
#include <typeindex>

namespace bstream
{
// TODO: fix configurability of allocator, default size, etc.
class ibstream;

class context_options
{
public:
	friend class context_base;

	context_options()
		: m_categories{&std::system_category(), &std::generic_category(), &bstream::error_category(), &util::error_category()},
		  m_dedup{true},
		  m_byte_order{byte_order::big_endian},
		  m_buf_size{65536UL}
	{}

	context_options&
	error_categories(std::initializer_list<const std::error_category*> categ_list)
	{
		m_categories.reserve(m_categories.size() + categ_list.size());
		m_categories.insert(m_categories.end(), categ_list);
		return *this;
	}

	context_options&
	dedup_shared_ptrs(bool flag)
	{
		m_dedup = flag;
		return *this;
	}

	context_options&
	buffer_size(util::size_type bsize)
	{
		m_buf_size = bsize;
		return *this;
	}

private:
	std::vector<const std::error_category*> m_categories;
	bool                                    m_dedup;
	enum byte_order                         m_byte_order;
	util::size_type                               m_buf_size;
};


class context_base : public util::error_context
{
public:
	// using ptr = util::shared_ptr<context_base>;

	static constexpr bool
	is_valid_tag(poly_tag_type tag)
	{
		return tag != invalid_tag;
	};

	context_base(context_options&& opts)
		: util::error_context{std::move(opts.m_categories)},
		  m_dedup_shared_ptrs{opts.m_dedup},
		  m_byte_order{opts.m_byte_order},
		  m_buffer_size{opts.m_buf_size}
	{}

	context_base(context_options const& opts)
		: util::error_context{opts.m_categories},
		  m_dedup_shared_ptrs{opts.m_dedup},
		  m_byte_order{opts.m_byte_order},
		  m_buffer_size{opts.m_buf_size}
	{}

	context_base(bool dedup_shared_ptrs, byte_order order, util::size_type buffer_size = 65536)
		: util::error_context{},
		  m_dedup_shared_ptrs{dedup_shared_ptrs},
		  m_byte_order{order},
		  m_buffer_size{buffer_size}
	{}

	virtual ~context_base() {}

	virtual poly_tag_type
	get_type_tag(std::type_index index) const = 0;

	template<class T>
	poly_tag_type
	get_type_tag() const
	{
		return get_type_tag(typeid(T));
	}

	virtual bool
	can_downcast_ptr(poly_tag_type from, poly_tag_type to) const = 0;

	virtual void*
	create_raw_from_tag(poly_tag_type tag, ibstream& is) const = 0;

	virtual std::shared_ptr<void>
	create_shared_from_tag(poly_tag_type tag, ibstream& is) const = 0;

	template<class T>
	T*
	create_raw(poly_tag_type tag, ibstream& is) const
	{
		if (can_downcast_ptr(tag, get_type_tag<T>()))
		{
			return static_cast<T*>(create_raw_from_tag(tag, is));
		}
		else
		{
			throw std::system_error{make_error_code(bstream::errc::invalid_ptr_downcast)};
		}
	}

	template<class T>
	std::shared_ptr<T>
	create_shared(poly_tag_type tag, ibstream& is) const
	{
		if (can_downcast_ptr(tag, get_type_tag<T>()))
		{
			return std::static_pointer_cast<T>(create_shared_from_tag(tag, is));
		}
		else
		{
			throw std::system_error{make_error_code(bstream::errc::invalid_ptr_downcast)};
		}
	}

	bool
	dedup_shared_ptrs() const
	{
		return m_dedup_shared_ptrs;
	}

	byte_order
	byte_order() const
	{
		return m_byte_order;
	}

	util::size_type
	buffer_size() const
	{
		return m_buffer_size;
	}

private:
	bool            m_dedup_shared_ptrs;
	enum byte_order m_byte_order;
	util::size_type       m_buffer_size;
};

using poly_raw_factory_func = std::function<void*(ibstream&)>;

using poly_shared_factory_func = std::function<std::shared_ptr<void>(ibstream&)>;

template<class T1, class T2, class Enable = void>
struct can_downcast_ptr : public std::false_type
{};

template<class T1, class T2>
struct can_downcast_ptr<T1, T2, std::enable_if_t<std::is_base_of<T2, T1>::value>> : public std::true_type
{};

template<class T, class... Args>
struct can_downcast_ptr_row
{
	static const std::vector<bool> row_values;
};

template<class T, class... Args>
const std::vector<bool> can_downcast_ptr_row<T, Args...>::row_values = {can_downcast_ptr<T, Args>::value...};

template<class... Args>
struct can_downcast_ptr_table
{
	static const std::vector<std::vector<bool>> values;
};

template<class... Args>
const std::vector<std::vector<bool>> can_downcast_ptr_table<Args...>::values
		= {{can_downcast_ptr_row<Args, Args...>::row_values}...};

template<class... Args>
class context : public context_base
{
public:
	context(context_options&& opts) : context_base{std::move(opts)} {}

	context(context_options const& opts) : context_base{opts} {}

	context(bool dedup_shared_ptrs = true, enum byte_order order = byte_order::big_endian, util::size_type buf_size = 65536UL)
		: context_base{dedup_shared_ptrs, order, buf_size}
	{}

	virtual poly_tag_type
	get_type_tag(std::type_index index) const override
	{
		auto it = m_type_tag_map.find(index);
		if (it != m_type_tag_map.end())
		{
			return it->second;
		}
		else
		{
			return invalid_tag;
		}
	}

	virtual bool
	can_downcast_ptr(poly_tag_type from, poly_tag_type to) const override
	{
		return m_downcast_ptr_table.values[from][to];
	}

	virtual void*
	create_raw_from_tag(poly_tag_type tag, ibstream& is) const override
	{
		return m_factories[tag](is);
	}

	virtual std::shared_ptr<void>
	create_shared_from_tag(poly_tag_type tag, ibstream& is) const override
	{
		return m_shared_factories[tag](is);
	}

protected:
	static const std::unordered_map<std::type_index, poly_tag_type> m_type_tag_map;
	static const can_downcast_ptr_table<Args...>                    m_downcast_ptr_table;
	static const std::vector<poly_raw_factory_func>                 m_factories;
	static const std::vector<poly_shared_factory_func>              m_shared_factories;
};

template<class... Args>
const std::unordered_map<std::type_index, poly_tag_type> context<Args...>::m_type_tag_map
		= {{typeid(Args), util::traits::index<Args, Args...>::value}...};

template<class... Args>
const can_downcast_ptr_table<Args...> context<Args...>::m_downcast_ptr_table;

template<class T, class Enable = void>
struct poly_factory;

template<class T>
struct poly_factory<T, std::enable_if_t<std::is_abstract<T>::value || !has_ptr_deserializer<T>::value>>
{
	void*
	operator()(ibstream& is) const
	{
		return get(is);
	}

	static void*
	get(ibstream&)
	{
		return nullptr;
	}
};

template<class T>
struct poly_factory<T, std::enable_if_t<!std::is_abstract<T>::value && has_ptr_deserializer<T>::value>>
{
	void*
	operator()(ibstream& is) const
	{
		return get(is);
	}

	static void*
	get(ibstream& is)
	{
		return ptr_deserializer<T>::get(is);
	}
};

template<class... Args>
const std::vector<poly_raw_factory_func> context<Args...>::m_factories
		= {[](ibstream& is) { return poly_factory<Args>::get(is); }...};

template<class T, class Enable = void>
struct poly_shared_factory;

template<class T>
struct poly_shared_factory<T, std::enable_if_t<std::is_abstract<T>::value || !has_shared_ptr_deserializer<T>::value>>
{
	std::shared_ptr<void>
	operator()(ibstream& is) const
	{
		return get(is);
	}

	static std::shared_ptr<void>
	get(ibstream&)
	{
		return nullptr;
	}
};

template<class T>
struct poly_shared_factory<T, std::enable_if_t<!std::is_abstract<T>::value && has_shared_ptr_deserializer<T>::value>>
{
	std::shared_ptr<void>
	operator()(ibstream& is) const
	{
		return get(is);
	}

	static std::shared_ptr<void>
	get(ibstream& is)
	{
		return shared_ptr_deserializer<T>::get(is);
	}
};

template<class... Args>
const std::vector<poly_shared_factory_func> context<Args...>::m_shared_factories
		= {[](ibstream& is) { return poly_shared_factory<Args>::get(is); }...};

class default_context_factory
{
public:
	using context_type = bstream::context<>;

	static context_options
	options()
	{
		return context_options{};
	}

	static bstream::context_base const&
	get()
	{
		static const context_type instance{options()};
		return instance;
	}
};

inline context_base const&
get_default_context()
{
	return default_context_factory::get();
}

}    // namespace bstream

#endif    // BSTREAM_CONTEXT_H
