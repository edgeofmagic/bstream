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

#ifndef BSTREAM_ERROR_CATEGORY_CONTEXT_H
#define BSTREAM_ERROR_CATEGORY_CONTEXT_H

#include <system_error>
#include <unordered_map>
#include <vector>

namespace bstream
{

class error_category_context
{
public:
	using index_type      = int;
	using category_vector = std::vector<const std::error_category*>;
	using category_map    = std::unordered_map<const std::error_category*, index_type>;

	error_category_context() {}

	error_category_context(category_vector&& categories);

	error_category_context(category_vector const& categories);

	std::error_category const&
	category_from_index(index_type index) const;

	index_type
	index_of_category(std::error_category const& category) const;

private:
	category_vector m_category_vector;
	category_map    m_category_map;
};

}    // namespace bstream

#endif    // BSTREAM_ERROR_CATEGORY_MAP_H