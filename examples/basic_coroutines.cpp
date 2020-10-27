// Copyright(C) 2020 Henry Bullingham
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#define CPPCOSL_IMPLEMENTATION

#include "cppcosl/cppcosl.h"

#include <iostream>

co_declare(count_to_ten)
{
    cppcosl::co_local<int> i;

    co_begin();

    std::cout << "Count to Ten" << std::endl;
    
    for (i = 1; i <= 10; i++)
    {
        std::cout << "i = " << i << std::endl;
        yield_return(cppcosl::wait_for_seconds(1.0f));
    }

    yield_break();
}

co_declare_args(count_to_n, int n)
{
    cppcosl::co_local<int> i;

    co_begin();

    std::cout << "Count to N" << std::endl;

    for (i = 1; i <= n; i++)
    {
        std::cout << "i = " << i << std::endl;
        yield_return(cppcosl::wait_for_seconds(1.0f));
    }

    yield_break();
}

co_declare_args(count_to_c, char c)
{
    cppcosl::co_local<char> current;

    co_begin();

    std::cout << "Count to C" << std::endl;

    yield_return(cppcosl::wait_for_seconds(0.5f));

    for (current = 'a'; current <= c; current++)
    {
        std::cout << "c = " << current << std::endl;
        yield_return(cppcosl::wait_for_seconds(1.0f));
    }

    yield_break();
}

static bool s_done;

int main(int argc, char ** argv)
{
    auto thread = cppcosl::cppcosl_start_thread([&] {return s_done; });

    cppcosl::co_handle handle = cppcosl::co_start(count_to_ten);

    cppcosl::co_wait(handle);

    cppcosl::co_handle handle1 = cppcosl::co_start(co_bind(count_to_n, 13));
    cppcosl::co_handle handle2 = cppcosl::co_start(co_bind(count_to_c, 'a' + 13));

    cppcosl::co_wait(handle);
    cppcosl::co_wait(handle2);

    s_done = true;

    thread.join();

    return 0;
}