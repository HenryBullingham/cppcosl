// Copyright(C) 2020 Henry Bullingham
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#define CPPCOSL_IMPLEMENTATION

#include "cppcosl/cppcosl.h"


#include <algorithm>
#include <iostream>
#include <memory>

/// <summary>
/// Prints out numbers from 0 to n
/// </summary>
/// <param name=""></param>
/// <param name="n"></param>
/// <returns></returns>
co_declare_args(count_to_n, int n)
{
    cppcosl::co_local<int> i;

    co_begin();

    for (i = 0; i <= n; i++)
    {
        std::cout << "i = " << i << std::endl;
        yield_return(cppcosl::wait_for_seconds(1.0f));
    }
    yield_break();
}

/// <summary>
/// Prints out numbers from n down to 0
/// </summary>
/// <param name=""></param>
/// <param name="n"></param>
/// <returns></returns>
co_declare_args(count_down_n, int n)
{
    cppcosl::co_local<int> j;

    co_begin();

    for(j = n; j >= 0; j--)
    {
        std::cout << "j = " << j << std::endl;
        yield_return(cppcosl::wait_for_seconds(1.0f));
    }
    yield_break();
}

/// <summary>
/// Calls count_to_n and count_down_n concurrently
/// </summary>
/// <param name=""></param>
/// <returns></returns>
co_declare(work_in_parallel)
{
    cppcosl::co_local<cppcosl::co_handle> h1, h2;

    co_begin();

    std::cout << "Begin Parallel Work!" << std::endl;

    h1 = cppcosl::co_start(cppcosl::co_bind(count_to_n, 10));

    yield_return(cppcosl::wait_for_seconds(0.5f));

    h2 = cppcosl::co_start(cppcosl::co_bind(count_down_n, 10));

    yield_return(cppcosl::wait_for_coroutine(h1));
    yield_return(cppcosl::wait_for_coroutine(h2));

    std::cout << "Parallel Work Done!" << std::endl;

    yield_break();
}

/// <summary>
/// Calls cppcosl::lib::foreach in parallel on a vector of pair<int, char> and prints the elements separately
/// </summary>
/// <param name=""></param>
/// <returns></returns>
co_declare(parallel_foreach)
{
    cppcosl::co_local<std::vector<std::pair<int, char>>> vec({ {7, 'a'}, {9, 'd'}, {23, 'l'}, {-5, 'o'}, {42, 'O'}, {69, 'f'}, {420, 'R'} });
    cppcosl::co_local<cppcosl::co_handle> h1, h2;

    co_begin();

    std::cout << "Parallel Foreach" << std::endl;

    h1 = cppcosl::co_start(cppcosl::lib::co_foreach(vec->begin(), vec->end(), [](const std::pair<int, char>& p) { std::cout << p.first << std::endl; }, cppcosl::wait_for_seconds(0.25f)));
    h2 = cppcosl::co_start(cppcosl::lib::co_foreach(vec->begin(), vec->end(), [](const std::pair<int, char>& p) { std::cout << p.second << std::endl; }, cppcosl::wait_for_seconds(0.35f)));

    yield_return(cppcosl::wait_for_coroutine(h1));
    yield_return(cppcosl::wait_for_coroutine(h2));

    yield_break();
}

/// <summary>
/// Merge sorts a list using coroutines
/// </summary>
/// <param name=""></param>
/// <param name="list"></param>
/// <param name="size"></param>
/// <returns></returns>
co_declare_args(co_merge_sort, int* list, int size)
{
    cppcosl::co_local<cppcosl::co_handle> h1, h2;
    cppcosl::co_local<std::unique_ptr<int[]>> buffer;
    cppcosl::co_local<int> i, j, k, sizeA;

    co_begin();

    yield_return(cppcosl::wait_for_seconds(1));

    std::cout << "Merge Sort: " << size << " elements. " << std::endl;

    if (size > 1)
    {
        h1 = cppcosl::co_start(cppcosl::co_bind(co_merge_sort, list, size / 2));
        h2 = cppcosl::co_start(cppcosl::co_bind(co_merge_sort, list + (size / 2), size - (size / 2)));

        yield_return(cppcosl::wait_for_coroutine(h1));
        yield_return(cppcosl::wait_for_coroutine(h2));

        buffer = std::make_unique<int[]>(size);
        sizeA = size / 2;
        i = 0;
        j = sizeA;
        k = 0;

        while (i < sizeA && j < size)
        {
            if (list[i] <= list[j])
            {
                buffer[k++] = list[i++];
            }
            else
            {
                buffer[k++] = list[j++];
            }
        }

        while (i < sizeA)
        {
            buffer[k++] = list[i++];
        }

        while (j < size)
        {
            buffer[k++] = list[j++];
        }

        for (i = 0; i < size; i++)
        {
            list[i] = buffer[i];
        }

        buffer->reset();
    }

    std::cout << "List = ";
    for (i = 0; i < size; i++)
    {
        std::cout << list[i] << " ";
    }
    std::cout << std::endl;

    yield_return(cppcosl::wait_for_seconds(1));

    yield_break();
}

static bool s_done;

int main(int argc, char** argv)
{
    auto thread = cppcosl::cppcosl_start_thread([&] {return s_done; });

    auto handle = cppcosl::co_start(work_in_parallel);

    cppcosl::co_wait(handle);

    handle = cppcosl::co_start(parallel_foreach);

    cppcosl::co_wait(handle);

    std::vector<int> numbers = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
    std::random_shuffle(numbers.begin(), numbers.end());

    handle = cppcosl::co_start(cppcosl::co_bind(co_merge_sort, &numbers[0], (int)numbers.size()));

    cppcosl::co_wait(handle);

    s_done = true;

    thread.join();

    return 0;
}