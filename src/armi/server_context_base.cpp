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

#include <logicmill/armi/error.h>
#include <logicmill/armi/fail_proxy.h>
#include <logicmill/armi/server_context_base.h>
#include <logicmill/bstream/imbstream.h>

using namespace logicmill;
using namespace armi;
/*
server_context_base::server_context_base(bstream::context_base::ptr const& stream_context)
	: m_stream_context{stream_context}
{}

server_context_base::~server_context_base() {}
*/
/*
void
server_context_base::really_bind(async::options const& opts, std::error_code& err)
{
	m_acceptor = m_loop->create_acceptor(
			opts, err, [=](async::acceptor::ptr const& sp, async::channel::ptr const& chan, std::error_code err) {
				if (err)
				{
					if (m_on_server_error)
					{
						m_on_server_error->invoke(err);
					}
					else
					{
						on_acceptor_error_default();
					}
				}
				else
				{
					m_open_channels.insert(chan);
					chan->start_read(
							err, [=](async::channel::ptr const& chan, util::const_buffer&& buf, std::error_code err) {
								if (err)
								{
									if (m_on_channel_error)
									{
										m_on_channel_error(chan, err);
									}
									else
									{
										on_channel_error_default(chan);
									}
								}
								else
								{
									bstream::imbstream is{std::move(buf), m_stream_context};
									handle_request(is, chan);
								}
							});
				}
			});
}
*/

// void
// server_context_base::send_reply(transport::server_connection::ptr const& chan, util::mutable_buffer&& buf)
// {
// 	if (chan)
// 	{
// 		chan->send_reply(std::move(buf));
// 	}
// }

/*
void
server_context_base::handle_request(bstream::ibstream& is, channel_id_type chan)
{
	auto        req_id  = is.read_as<request_id_type>();
	// std::size_t if_index = is.read_as<std::size_t>();
		auto& stub = get_type_erased_stub();
		stub.process(req_id, chan, is);
}
*/
// void
// server_context_base::cleanup()
// {
// 	if (m_on_close)
// 	{
// 		m_on_close();
// 		m_on_close = nullptr;
// 	}
// }

// bool
// server_context_base::really_close()
// {
// 	bool result{false};
// 	if (m_acceptor)
// 	{
// 		bool acceptor_did_close = m_acceptor->close([=](async::acceptor::ptr const& lp) {
// 			m_acceptor.reset();
// 			if (m_open_channels.empty())
// 			{
// 				cleanup();
// 			}
// 		});
// 		if (acceptor_did_close)
// 		{
// 			result = true;
// 		}
// 		else
// 		{
// 			m_acceptor.reset();
// 		}
// 	}
// 	auto it = m_open_channels.begin();
// 	while (it != m_open_channels.end())
// 	{
// 		bool channel_did_close = (*it)->close([=](async::channel::ptr const& c) {
// 			m_open_channels.erase(c);
// 			if (m_open_channels.empty() && !m_acceptor)
// 			{
// 				cleanup();
// 			}
// 		});
// 		if (channel_did_close)
// 		{
// 			result = true;
// 			++it;
// 		}
// 		else
// 		{
// 			it = m_open_channels.erase(it);
// 		}
// 	}
// 	if (!result)    // there was no deferred closing action; wipe everything
// 	{
// 		m_acceptor.reset();         // probably unnecessary, but whatever
// 		m_open_channels.clear();    // ditto
// 		m_on_close = nullptr;
// 	}
// 	return result;
// }
