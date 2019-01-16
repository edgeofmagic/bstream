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

#include <doctest.h>
#include <iostream>
#include <logicmill/async/loop.h>

using namespace logicmill;

class stopwatch
{
public:
	stopwatch() : m_start{std::chrono::system_clock::now()} {}

	void
	start()
	{
		m_start = std::chrono::system_clock::now();
	}

	template<class T>
	T
	elapsed() const
	{
		auto current_time = std::chrono::system_clock::now();
		T    et           = std::chrono::duration_cast<T>(current_time - m_start);
		return et;
	}

private:
	std::chrono::time_point<std::chrono::system_clock> m_start;
};

#if 1

TEST_CASE("logicmill::async::loop [ smoke ] { dispatch }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;

	bool first_dispatch_called  = false;
	bool second_dispatch_called = false;

	lp->dispatch(err, [&](async::loop::ptr const& loop_ptr) {
		first_dispatch_called = true;
		loop_ptr->dispatch(err, [&](async::loop::ptr const& loop_ptr) {
			second_dispatch_called = true;
			auto loop_exit_timer   = loop_ptr->create_timer(err, [](async::timer::ptr tp) {
                std::error_code err;
                tp->loop()->stop(err);
                CHECK(!err);
            });
			CHECK(!err);
			loop_exit_timer->start(std::chrono::milliseconds{500}, err);
			CHECK(!err);
		});
		CHECK(!err);
	});

	lp->run(err);
	CHECK(first_dispatch_called);
	CHECK(second_dispatch_called);
	CHECK(!err);
}

TEST_CASE("logicmill::async::loop [ smoke ] { nullptr handler }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;
	lp->dispatch(err, nullptr);
	CHECK(err);
	CHECK(err == std::errc::invalid_argument);
}


TEST_CASE("logicmill::async::loop [ smoke ] { basic }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;

	lp->dispatch(err, [](async::loop::ptr const& loop_ptr) {
		std::error_code err;
		loop_ptr->stop(err);
		CHECK(!err);
	});

	lp->run(err);
	CHECK(!err);
	lp->close(err);
	CHECK(!err);
}

TEST_CASE("logicmill::async::loop [ smoke ] { default loop }")
{
	async::loop::ptr lp = async::loop::get_default();
	std::error_code  err;

	bool first_visited{false};
	bool second_visited{false};

	lp->dispatch(err, [&](async::loop::ptr const& loop_ptr) {
		first_visited = true;
		std::error_code err;
		loop_ptr->stop(err);
		CHECK(!err);
	});

	lp->run(err);
	CHECK(!err);

	lp->dispatch(err, [&](async::loop::ptr const& lp) {
		second_visited = true;
		std::error_code err;
		lp->stop(err);
		CHECK(!err);
	});

	lp->run(err);
	CHECK(!err);

	CHECK(first_visited);
	CHECK(second_visited);
}

TEST_CASE("logicmill::async::loop::timer [ smoke ] { basic }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;

	{
		auto tp = lp->create_timer(err, [&](async::timer::ptr timer_ptr) {
			CHECK(!timer_ptr->is_pending());
			std::cout << "timer expired" << std::endl;
			timer_ptr->loop()->stop(err);
		});

		CHECK(!err);
		tp->start(std::chrono::milliseconds{1000}, err);
		CHECK(!err);
		CHECK(tp->is_pending());
	}

	lp->run(err);
	CHECK(!err);
	lp->close(err);
	CHECK(!err);
}

TEST_CASE("logicmill::async::loop::timer [ smoke ] { close before expire }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;
	auto tp0 = lp->create_timer(err, [](async::timer::ptr timer_ptr) { std::cout << "timer 0 expired" << std::endl; });
	CHECK(!err);

	auto tp1 = lp->create_timer(err, [](async::timer::ptr timer_ptr) {
		std::error_code ec;
		std::cout << "timer 1 expired" << std::endl;
		auto my_loop = timer_ptr->loop();
		my_loop->stop(ec);
		CHECK(!ec);
	});

	CHECK(!err);
	tp0->start(std::chrono::milliseconds{2000}, err);
	CHECK(!err);
	tp1->start(std::chrono::milliseconds{1000}, err);
	CHECK(!err);
	lp->run(err);
	CHECK(!err);
	std::cout << "loop run completed" << std::endl;
}

TEST_CASE("logicmill::async::loop::timer [ smoke ] { stop before expire }")
{
	async::loop::ptr lp = async::loop::create();
	std::error_code  err;
	auto tp0 = lp->create_timer(err, [](async::timer::ptr timer_ptr) { std::cout << "timer 0 expired" << std::endl; });
	CHECK(!err);

	auto tp1 = lp->create_timer(err, [=](async::timer::ptr timer_ptr) {
		std::error_code ec;
		std::cout << "timer 1 expired" << std::endl;
		tp0->stop(ec);
		CHECK(!ec);
	});

	auto loop_exit = lp->create_timer(err, [&](async::timer::ptr timer_ptr) { timer_ptr->loop()->stop(err); });
	CHECK(!err);
	tp0->start(std::chrono::milliseconds{2000}, err);
	CHECK(!err);
	tp1->start(std::chrono::milliseconds{1000}, err);
	CHECK(!err);
	loop_exit->start(std::chrono::milliseconds{2500}, err);
	CHECK(!err);
	stopwatch sw;
	lp->run(err);
	CHECK(!err);
	auto ms = sw.elapsed<std::chrono::milliseconds>();
	std::cout << "loop run completed: " << ms.count() << " ms" << std::endl;
	CHECK(true);
}
#endif


TEST_CASE("logicmill::async::resolver [ smoke ] { basic }")
{
	std::error_code err;

	async::loop::ptr lp = async::loop::create();
	lp->resolve(
			"google.com",
			err,
			[&](std::string const& hostname, std::deque<async::ip::address>&& addresses, std::error_code err) {
				std::cout << "resolver handler called for hostname " << hostname << std::endl;
				std::cout << "status is " << err.message() << std::endl;
				if (!err)
				{
					for (auto& addr : addresses)
					{
						std::cout << addr.to_string() << std::endl;
					}
				}
				std::cout.flush();
				lp->stop(err);
				CHECK(!err);
			});

	if (err)
	{
		std::cout << "create_resolver failed, err: " << err.message() << std::endl;
	}
	else
	{
		lp->run(err);
		CHECK(!err);
		std::cout << "loop run completed" << std::endl;
	}

	lp->close(err);
	CHECK(!err);
}

TEST_CASE("logicmill::async::resolver [ smoke ] { not found failure }")
{
	std::error_code err;

	async::loop::ptr lp = async::loop::create();
	lp->resolve(
			"no_such_host.com",
			err,
			[&](std::string const& hostname, std::deque<async::ip::address>&& addresses, std::error_code err) {
				std::cout << "resolver handler called for hostname " << hostname << std::endl;
				std::cout << "status is " << err.message() << std::endl;
				if (!err)
				{
					for (auto& addr : addresses)
					{
						std::cout << addr.to_string() << std::endl;
					}
				}
				std::cout.flush();
				lp->stop(err);
				CHECK(!err);
			});

	if (err)
	{
		std::cout << "create_resolver failed, err: " << err.message() << std::endl;
	}
	else
	{
		lp->run(err);
		CHECK(!err);
		std::cout << "loop run completed" << std::endl;
	}

	// lp->close();
}

TEST_CASE("logicmill::async::resolver [ smoke ] { cancellation loop close }")
{
	{

		std::cout << "starting cancellation_loop_close test" << std::endl;
		auto start = std::chrono::system_clock::now();

		std::error_code err;

		async::loop::ptr lp = async::loop::create();
		lp->resolve(
				"gorblesnapper.org",
				err,
				[&](std::string const& hostname, std::deque<async::ip::address>&& addresses, std::error_code err) {
					auto                      current = std::chrono::system_clock::now();
					std::chrono::microseconds elapsed
							= std::chrono::duration_cast<std::chrono::microseconds>(current - start);
					std::cout << "resolver handler called for hostname " << hostname << std::endl;
					std::cout << "elapsed time is " << elapsed.count() << " usec" << std::endl;
					std::cout << "status is " << err.message() << std::endl;
					if (!err)
					{
						for (auto& addr : addresses)
						{
							std::cout << addr.to_string() << std::endl;
						}
					}
					std::cout.flush();
				});

		async::timer::ptr tp = lp->create_timer(
				err,
				[&](async::timer::ptr timer_ptr)    // mutable
				{
					auto                      current = std::chrono::system_clock::now();
					std::chrono::microseconds elapsed
							= std::chrono::duration_cast<std::chrono::microseconds>(current - start);
					std::cout << "stopping loop" << std::endl;
					std::cout << "elapsed time is " << elapsed.count() << " usec" << std::endl;
					std::cout.flush();
					std::error_code ec;
					timer_ptr->loop()->stop(ec);
					CHECK(!ec);
				});
		REQUIRE(!err);
		tp->start(std::chrono::milliseconds{1}, err);
		REQUIRE(!err);
		lp->run(err);
		CHECK(!err);
		lp->close(err);
		std::cout << "error on loop close: " << err.message() << std::endl; 
		CHECK(!err);
		std::cout << "loop run completed" << std::endl;
		std::cout << "outstanding loop refcount after run: " << lp.use_count() << std::endl;
	}
}
