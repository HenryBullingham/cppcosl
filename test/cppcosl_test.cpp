// Copyright(C) 2020 Henry Bullingham
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#include "cppcosl/cppcosl.h"
#include "catch_amalgamated.hpp"

//Add 2 numbers, store the result in an output param
template<typename Number>
co_declare_args(add, Number a, Number b, Number* c)
{
    co_begin();
    *c = a + b;
    yield_break();
}

TEST_CASE("run a coroutine")
{
    bool done = false;
    auto thread = cppcosl::cppcosl_start_thread([&] {return done; });

    int x;
    auto handle = cppcosl::co_start(cppcosl::co_bind(add<int>, 1, 2, &x));
    REQUIRE(handle != nullptr);

    cppcosl::co_wait(handle);

    REQUIRE(x == 3);

    done = true;
    thread.join();
}

//Iterate n times, add to output each time
co_declare_args(sum_1_to_n, int n, int* out_sum)
{
    cppcosl::co_local<int> i;

    co_begin();

    *out_sum = 0;
    for (i = 1; i <= n; i++)
    {
        (*out_sum) += i;
        yield_return(cppcosl::yield());
    }

    yield_break();
}

TEST_CASE("run a coroutine that yields")
{
    bool done = false;
    auto thread = cppcosl::cppcosl_start_thread([&] {return done; });

    int x;
    auto handle = cppcosl::co_start(cppcosl::co_bind(sum_1_to_n, 10, &x));
    REQUIRE(handle != nullptr);

    cppcosl::co_wait(handle);

    REQUIRE(x == 55);

    done = true;
    thread.join();
}

//Test time scheduling
co_declare_args(wait_some_seconds, float seconds)
{
    co_begin();

    yield_return(cppcosl::wait_for_seconds(seconds));

    yield_break();
}

TEST_CASE("test waiting for time")
{
    bool done = false;
    auto thread = cppcosl::cppcosl_start_thread([&] {return done; });

    for (float t = 0.1f; t < 10; t *= 2)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto handle = cppcosl::co_start(cppcosl::co_bind(wait_some_seconds, t));
        REQUIRE(handle != nullptr);
        cppcosl::co_wait(handle);

        auto end = std::chrono::high_resolution_clock::now();
        float dt = (end - start).count() / 1000000000.0f;

        //Tolerate less than 5 percent error
        float error = std::abs((dt - t) / t);
        REQUIRE(error < 0.05);
    }

    done = true;
    thread.join();
}