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

#ifndef LOGICMILL_ARMI_METHOD_STUB_H
#define LOGICMILL_ARMI_METHOD_STUB_H

#include <logicmill/armi/member_func_stub_base.h>
#include <logicmill/armi/server_stub_base.h>
#include <logicmill/util/promise.h>

namespace logicmill
{
namespace armi
{
template<class ServerStubBase, class MemberFuncPtr, class StrippedMemberFuncPtr, class Enable = void>
class member_func_stub;

/*
 * specialization for non-void promise return value
 */

template<
		template<class...> class ServerStubBaseTemplate,
		class SerializationTraits,
		class AsyncIOTraits,
		class MemberFuncPtr,
		class Target,
		class PromiseType,
		class... Args>
class member_func_stub<
		ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>,
		MemberFuncPtr,
		util::promise<PromiseType> (Target::*)(Args...),
		std::enable_if_t<!std::is_void<PromiseType>::value>>
	: public member_func_stub_base<Target, ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>>
{
public:
	using member_func_ptr_type = MemberFuncPtr;
	using target_ptr_type      = std::shared_ptr<Target>;
	using base = member_func_stub_base<Target, ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>>;
	using base::request_failed;
	using base::server;
	using server_stub_base_type = ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>;

	using serialization_traits    = SerializationTraits;
	using async_io_traits        = AsyncIOTraits;
	using bridge_type             = adapters::bridge<serialization_traits, async_io_traits>;
	using deserializer_param_type = typename bridge_type::deserializer_param_type;

	inline member_func_stub(
			server_stub_base_type* server,
			member_func_ptr_type      member_func_ptr,
			std::size_t               member_func_id)
		: base{server}, m_member_func_id{member_func_id}, m_member_func_ptr{member_func_ptr}
	{}

	virtual void
	dispatch(
			request_id_type         request_id,
			channel_id_type         channel,
			deserializer_param_type request,
			target_ptr_type const&  target) const override
	{
		std::error_code err;
		auto            item_count = serialization_traits::read_sequence_prefix(request);

		if (!expected_count<sizeof...(Args)>(item_count))
		{
			err = make_error_code(armi::errc::invalid_argument_count);
			goto exit;
		}

		if (!target)
		{
			err = make_error_code(armi::errc::no_target_provided);
			goto exit;
		}

		try
		{
			((*target).*m_member_func_ptr)(
					serialization_traits::template read<
							typename std::remove_cv_t<typename std::remove_reference_t<Args>>>(request)...)
					.then(
							[&](PromiseType value) mutable {
								bridge_type::new_serializer([&](typename bridge_type::serializer_param_type reply) {
									serialization_traits::write(reply, request_id);
									serialization_traits::write(reply, reply_kind::normal);
									serialization_traits::write_sequence_prefix(reply, 1);
									serialization_traits::write(reply, value);
									server()->send_reply(channel, reply);
								});
							},
							[&](std::error_code err) { request_failed(request_id, channel, err); });
		}
		catch (std::system_error const& e)
		{
			err = e.code();
		}
		catch (std::exception const& e)
		{
			err = make_error_code(armi::errc::exception_thrown_by_member_func_stub);
		}

	exit:
		if (err)
			request_failed(request_id, channel, err);
	}

private:
	std::size_t          m_member_func_id;
	member_func_ptr_type m_member_func_ptr;
};

/*
 * specialization for void promise return value
 */

template<
		template<class...> class ServerStubBaseTemplate,
		class SerializationTraits,
		class AsyncIOTraits,
		class MemberFuncPtr,
		class Target,
		class PromiseType,
		class... Args>
class member_func_stub<
		ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>,
		MemberFuncPtr,
		util::promise<PromiseType> (Target::*)(Args...),
		std::enable_if_t<std::is_void<PromiseType>::value>>
	: public member_func_stub_base<Target, ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>>
{
public:
	using member_func_ptr_type = MemberFuncPtr;
	using target_ptr_type      = std::shared_ptr<Target>;
	using base = member_func_stub_base<Target, ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>>;
	using base::request_failed;
	using base::server;
	using server_stub_base_type = ServerStubBaseTemplate<SerializationTraits, AsyncIOTraits>;

	using serialization_traits    = SerializationTraits;
	using async_io_traits        = AsyncIOTraits;
	using bridge_type             = adapters::bridge<serialization_traits, async_io_traits>;
	using deserializer_param_type = typename bridge_type::deserializer_param_type;

	inline member_func_stub(
			server_stub_base_type* server,
			member_func_ptr_type      member_func_ptr,
			std::size_t               member_func_id)
		: base{server}, m_member_func_id{member_func_id}, m_member_func_ptr{member_func_ptr}
	{}

	virtual void
	dispatch(
			request_id_type         request_id,
			channel_id_type         channel,
			deserializer_param_type request,
			target_ptr_type const&  target) const override
	{
		std::error_code err;
		auto            item_count = serialization_traits::read_sequence_prefix(request);

		if (!expected_count<sizeof...(Args)>(item_count))
		{
			err = make_error_code(armi::errc::invalid_argument_count);
			goto exit;
		}

		if (!target)
		{
			err = make_error_code(armi::errc::no_target_provided);
			goto exit;
		}

		try
		{
			((*target).*m_member_func_ptr)(
					serialization_traits::template read<
							typename std::remove_cv_t<typename std::remove_reference_t<Args>>>(request)...)
					.then(
							[&]() {
								bridge_type::new_serializer([&](typename bridge_type::serializer_param_type reply) {
									serialization_traits::write(reply, request_id);
									serialization_traits::write(reply, reply_kind::normal);
									serialization_traits::write_sequence_prefix(reply, 0);
									server()->send_reply(channel, reply);
								});
							},
							[=](std::error_code err) { request_failed(request_id, channel, err); });
		}
		catch (std::system_error const& e)
		{
			err = e.code();
		}
		catch (std::exception const& e)
		{
			err = make_error_code(armi::errc::exception_thrown_by_member_func_stub);
		}
	exit:
		if (err)
			request_failed(request_id, channel, err);
	}

private:
	std::size_t          m_member_func_id;
	member_func_ptr_type m_member_func_ptr;
};

}    // namespace armi
}    // namespace logicmill

#endif    // LOGICMILL_ARMI_METHOD_STUB_H
