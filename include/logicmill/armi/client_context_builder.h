/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client_context_builder.h
 * Author: David Curtis
 *
 * Created on January 4, 2018, 8:56 PM
 */

#ifndef LOGICMILL_ARMI_CLIENT_CONTEXT_BUILDER_H
#define LOGICMILL_ARMI_CLIENT_CONTEXT_BUILDER_H

#include <memory>
#include <vector>
#include <logicmill/armi/client_context_base.h>
#include <logicmill/armi/interface_proxy.h>

namespace logicmill
{
namespace armi
{
	template <class... Args>
	class client_context_builder : public client_context_base
	{
	public:
		client_context_builder(async::loop::ptr const& lp, bstream::context_base const& stream_context)
		: client_context_base{lp, stream_context}
		{
			m_proxies.reserve(sizeof...(Args));
			append_proxies<Args...>();
		}

		template<class T>
		void
		append_proxies()
		{
			append_proxy<T>();
		}

		template<class First_, class... Args_>
		typename std::enable_if<(sizeof...(Args_) > 0)>::type
		append_proxies()
		{
			append_proxy<First_>();
			append_proxies<Args_...>();
		}

		template<class T>
		void
		append_proxy()
		{
			std::size_t index = m_proxies.size();
			m_proxies.push_back(std::make_shared<T>(*this, index));
		}

	protected:
		std::vector<std::shared_ptr<armi::interface_proxy>> m_proxies;
	};

} // namespace armi
} // namespace logicmill

#endif /* LOGICMILL_ARMI_CLIENT_CONTEXT_BUILDER_H */

