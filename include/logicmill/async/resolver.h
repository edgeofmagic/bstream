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

#ifndef LOGICMILL_ASYNC_RESOLVE_REQUEST_H
#define LOGICMILL_ASYNC_RESOLVE_REQUEST_H

#include <memory>
#include <functional>
#include <system_error>
#include <logicmill/async/address.h>
#include <deque>

namespace logicmill
{
namespace async
{

class loop;

class resolver
{
public:
	using ptr = std::shared_ptr< resolver >;
	using handler = std::function< void( resolver::ptr req, std::deque< ip::address >&& addresses, std::error_code const& err ) >;

	virtual ~resolver() {}

	virtual std::shared_ptr< loop >
	owner() = 0;

	virtual void
	cancel() = 0;

	virtual std::string const&
	hostname() const = 0;
};

} // namespace async
} // namespace logicmill

#endif // LOGICMILL_ASYNC_TIMER_H