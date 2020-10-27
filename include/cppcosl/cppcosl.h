// Copyright(C) 2020 Henry Bullingham
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.


/*

C++ Coroutine Library Using Setjmp and Longjmp

This is a header only-library. 

use #define CPPCOSL_IMPLEMENTATION in one source file where the compiled library code should be located

USAGE:

Firstly, some things to keep in mind:
1) you cannot have normal local variables or temporaries in the coroutines, nor will function parameters persist changes 
    (This is because setjmp & longjmp do not preserve the values of locals/temporaries/parameters that change between calls. 
     Furthermore, to ensure that setjmp & longjmp work for coroutines, the function is called multiple times, and within itself jumps to the correct position)


The structure of a coroutine is as follows:

co_declare(my_coroutine_name) OR
co_declare_args(my_coroutine_name, _type _name, _type2 _name2 ...) OR
{
    //local declarations

    co_begin(); 

    //Coroutine body

    yield_break();
}

    for local declarations, they can be done ass follows, and will call the appropriate constructor:
    //Local declarations (as many as you want)
    co_local<the_type> the_name; OR
    co_local<the_type> the_name = value; OR
    co_local<the_type> the_name(value1, value2);

    for the coroutine body, you can have a mix of the normal c++ expressions, and if you want to yield, you can use:

    yield_return(_______)

    yield_call(another_coroutine) OR
    yield_call(another_coroutine, arg1, arg2, ...)

    yield_break()

    for yield return, call it with the result of a function that returns a co_result. IE

    yield_return(wait_for_seconds(0.5)); OR
    yield_return(yield())

    yield call is similar.



*/
#pragma once

#if !defined(CPPCOSL_INCLUDE_CPPCOSL_H)
#define CPPCOSL_INCLUDE_CPPCOSL_H

#include <csetjmp>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <type_traits>
#include <utility>

namespace cppcosl
{
    namespace detail
    {
        struct co_block;
    }

    /// <summary>
    /// A handle to a coroutine (for joining)
    /// </summary>
    using co_handle = std::shared_ptr<detail::co_block>;

    /// <summary>
    /// The type returned from coroutine functions
    /// </summary>
    struct co_result;

    /// <summary>
    /// Yield the coroutine and waits 'seconds' seconds before scheduling it
    /// </summary>
    /// <param name="seconds"></param>
    /// <returns></returns>
    co_result wait_for_seconds(float seconds);

    /// <summary>
    /// Yield the coroutine and waits 'frames' frames before scheduling it.
    /// A frame is a single processing of all running coroutines
    /// </summary>
    /// <param name="frames"></param>
    /// <returns></returns>
    co_result wait_for_frames(int frames);

    /// <summary>
    /// Yield the coroutine and wait until the given coroutine terminates
    /// </summary>
    /// <param name="routine"></param>
    /// <returns></returns>
    co_result wait_for_coroutine(co_handle routine);

    /// <summary>
    /// Yields the coroutine to allow others to run. Will run again as soon as it can be scheduled
    /// </summary>
    /// <returns></returns>
    co_result yield();

    /// <summary>
    /// A type for creating local variables in coroutines
    /// Normal local variables are invalidated by setjmp / longjmp,
    /// So local variables need to be allocated on a separate context
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    class co_local;

    namespace detail
    {
        class co_context;
    }

    using coroutine = std::function<co_result(detail::co_context&, int)>;

    /// <summary>
    /// Binds arguments to a coroutine function with additional parameters
    /// </summary>
    /// <typeparam name="Coroutine"></typeparam>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="func"></param>
    /// <param name="...args"></param>
    /// <returns></returns>
    template<typename Coroutine, typename ... Args>
    coroutine co_bind(Coroutine func, Args&& ... args)
    {
        return std::bind(func, std::placeholders::_1, std::placeholders::_2, std::forward<Args>(args)...);
    }

    /// <summary>
    /// Starts a coroutine and returns a handle to it
    /// </summary>
    /// <param name="co"></param>
    /// <returns></returns>
    co_handle co_start(coroutine co);

    /// <summary>
    /// Waits for a coroutine to finish (spin locks). DO NOT USE THIS INSIDE A COROUTINE. IT WILL BLOCK ALL COROUTINES FROM EXECUTING
    /// </summary>
    /// <param name="co"></param>
    void co_wait(co_handle co);

    /// <summary>
    /// Call this if you prefer not to use std::thread
    /// </summary>
    /// <param name="should_terminate"></param>
    void cppcosl_thread_func(std::function<bool()> should_terminate);

    /// <summary>
    /// Starts a thread to run coroutines on. Cppcosl only supports one thread
    /// </summary>
    /// <param name="should_terminate"></param>
    /// <returns></returns>
    std::thread cppcosl_start_thread(std::function<bool()> should_terminate);

    /*Coroutine definition Macros*/

#define cppcosl_stat cppcosl_arg_status_hidden
#define cppcosl_ctx  cppcosl_arg_context_hidden
#define cppcosl_buff cppcosl_ctx.get_jmp_buf()
#define cppcosl_ret  cppcosl_local_return_hidden
#define cppcosl_co_ret cppcosl_local_co_result_hidden

/// <summary>
/// Declaration of coroutine function name that takes no additional arguments
/// </summary>
#define co_declare(_funcname) \
cppcosl::co_result _funcname(cppcosl::detail::co_context& cppcosl_ctx, int cppcosl_stat)

/// <summary>
/// Declaration of coroutine function name that takes additional arguments
/// </summary>
#define co_declare_args(_funcname, ...)\
cppcosl::co_result _funcname(cppcosl::detail::co_context& cppcosl_ctx, int cppcosl_stat, __VA_ARGS__)

/// <summary>
/// Declares the start of the coroutine. Must appear exactly once in every coroutine!
// Must be placed after all co_local declarations and before any other code is run!
/// </summary>
#define co_begin() \
    int cppcosl_ret = 0;\
    if (cppcosl_stat != 0)\
    {\
        longjmp(cppcosl_buff, cppcosl_stat);\
    }

/// <summary>
/// Yields control of the coroutine and delays execution based on the returned result
/// </summary>
#define yield_return(_co_result) \
    cppcosl_ret = setjmp(cppcosl_buff);\
    if (cppcosl_ret == 0) \
    { \
        return (_co_result);\
    }

/// <summary>
/// Yields control of the coroutine to call another coroutine
/// </summary>
#define yield_call(_coroutine)\
{ \
    cppcosl::detail::make_context_current(&cppcosl_ctx.get_or_make_nested()); \
    cppcosl::co_result cppcosl_co_ret = _coroutine(cppcosl_ctx.get_or_make_nested(), 0); \
    while(cppcosl_co_ret.type != cppcosl::detail::co_result_type::Done) \
    {\
        yield_return(cppcosl_co_ret);\
        cppcosl::detail::make_context_current(&cppcosl_ctx.get_or_make_nested()); \
        cppcosl_co_ret = _coroutine(cppcosl_ctx.get_or_make_nested(), 1);\
    }\
    cppcosl_ctx.clear_nested();\
}

    /// <summary>
/// Yields control of the coroutine to call another coroutine with ARGS
/// </summary>
#define yield_call_args(_coroutine, ...)\
{ \
    cppcosl::detail::make_context_current(&cppcosl_ctx.get_or_make_nested()); \
    cppcosl::co_result cppcosl_co_ret = _coroutine(cppcosl_ctx.get_or_make_nested(), 0, __VA_ARGS__); \
    while(cppcosl_co_ret.type != cppcosl::detail::co_result_type::Done) \
    {\
        yield_return(cppcosl_co_ret);\
        cppcosl::detail::make_context_current(&cppcosl_ctx.get_or_make_nested()); \
        cppcosl_co_ret = _coroutine(cppcosl_ctx.get_or_make_nested(), 1, __VA_ARGS__);\
    }\
    cppcosl_ctx.clear_nested();\
}

/// <summary>
/// Notifies that the coroutine is done. Must appear at least once in every coroutine, as the final statement
/// </summary>
#define yield_break()\
    return cppcosl::detail::done();

    //Library of useful coroutines

    namespace lib
    {
        /// <summary>
        /// Applies the given function to the elements in the sequence. Yields the specified result after each iteration
        /// </summary>
        /// <typeparam name="ForwardIterator"></typeparam>
        /// <typeparam name="Func"></typeparam>
        /// <param name=""></param>
        /// <param name="begin"></param>
        /// <param name="end"></param>
        /// <param name="func"></param>
        /// <returns></returns>
        template<typename ForwardIterator, typename Func>
        coroutine co_foreach(ForwardIterator begin, ForwardIterator end, Func&& func, co_result yield_result = yield());

        /// <summary>
        /// Calls the given function every 'period' seconds, for the given number of repetitions.
        /// If repetitions is 0, this continues indefinitely.
        /// </summary>
        /// <typeparam name="Func"></typeparam>
        /// <param name=""></param>
        /// <param name="operation"></param>
        /// <param name="period"></param>
        /// <param name="repetitions"></param>
        /// <returns></returns>
        coroutine co_timer(std::function<void()> func, float period, int repetitions = 0);

    }

    /*
    
        Implementation Details
    
    */

    namespace detail
    {
        /// <summary>
        /// The types of results that can be returned from coroutines
        /// </summary>
        enum class co_result_type : int
        {
            Uninit,
            Done,
            WaitForSeconds,
            WaitForFrames,
            WaitForCoroutine,
            Yield
        };

        /// <summary>
        /// A union type for holding multiple arguments
        /// </summary>
        union co_result_param
        {
            int64_t i_val;
            float f_val;
            void* ptr_val;
            co_result_param() {}
            co_result_param(int i) : i_val(i) {}
            co_result_param(float f) : f_val(f) {}
            co_result_param(void* ptr) : ptr_val(ptr) {}
        };
    }

    struct co_result
    {
        detail::co_result_type type;
        detail::co_result_param arg0;
    };

    namespace detail
    {
        /// <summary>
        /// Special return to denote coroutine has not started
        /// </summary>
        /// <returns></returns>
        inline co_result uninit()
        {
            return { detail::co_result_type::Uninit };
        }

        /// <summary>
        /// Special return to denote coroutine is finished
        /// </summary>
        /// <returns></returns>
        inline co_result done()
        {
            return { detail::co_result_type::Done };
        }
    }


    inline co_result wait_for_seconds(float seconds)
    {
        return { detail::co_result_type::WaitForSeconds, seconds };
    }

    inline co_result wait_for_frames(int frames)
    {
        return { detail::co_result_type::WaitForSeconds, frames };
    }

    inline co_result wait_for_coroutine(co_handle routine)
    {
        return { detail::co_result_type::WaitForCoroutine, (void*)routine.get() };
    }

    inline co_result yield()
    {
        return { detail::co_result_type::Yield };
    }

    namespace detail
    {
        /*
        
        Co-context class
        
        */
        class co_context
        {
        private:
            //The jump buffer fot the context
            jmp_buf m_buf;
            //The allocations for 'local' variables on the context
            std::vector<std::shared_ptr<void>> m_allocations;
            //Counter for getting the correct allocations
            int m_count = 0;
            //A nested context for calling nested coroutines
            std::unique_ptr<co_context> m_nested;

        public:
            /// <summary>
            /// Allocates a variable of the given type on the context (calls default constructor),
            /// Or gets it if it is already allocated
            /// And returns a reference to it
            /// </summary>
            /// <typeparam name="T"></typeparam>
            /// <returns></returns>
            template<typename T>
            T& allocate_or_get();

            /// <summary>
            /// Allocates a variable of the given type on the context (calls constructor with forwarded args),
            /// Or gets it if it is already allocated
            /// And returns a reference to it
            /// </summary>
            /// <typeparam name="T"></typeparam>
            /// <returns></returns>
            template<typename T, typename ... Args>
            T& allocate_or_get(Args&& ... args);

            /// <summary>
            /// Resets the internal count of the context so that the coroutine can be called
            /// And get the correct local args
            /// </summary>
            void reset();

            /// <summary>
            /// Clears all allocations in the context
            /// </summary>
            void clear();

            /// <summary>
            /// Gets or makes a nested context for calling nested coroutines
            /// </summary>
            /// <returns></returns>
            co_context& get_or_make_nested();

            /// <summary>
            /// Clears the nested context
            /// </summary>
            void clear_nested();

            /// <summary>
            /// Gets the jmp_buf stored in the context
            /// </summary>
            /// <returns></returns>
            jmp_buf& get_jmp_buf();
        };

        template<typename T>
        inline T& co_context::allocate_or_get()
        {
            if (m_count < m_allocations.size())
            {
                return *(T*)(m_allocations[m_count++].get());
            }

            m_count++;
            std::shared_ptr<T> ptr = std::make_shared<T>();
            m_allocations.push_back(ptr);
            return *ptr;
        }

        template<typename T, typename ... Args>
        inline T& co_context::allocate_or_get(Args&& ... args)
        {
            if (m_count < m_allocations.size())
            {
                return *(T*)(m_allocations[m_count++].get());
            }

            m_count++;
            std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<Args>(args)...);
            m_allocations.push_back(ptr);
            return *ptr;
        }

        inline void co_context::reset()
        {
            m_count = 0;
            if (m_nested != nullptr)
            {
                m_nested->reset();
            }
        }

        inline void co_context::clear()
        {
            m_allocations.clear();
        }

        inline co_context& co_context::get_or_make_nested()
        {
            if (m_nested == nullptr)
            {
                m_nested = std::make_unique<co_context>();
            }
            return *m_nested;
        }

        inline void co_context::clear_nested()
        {
            m_nested->clear();
        }

        jmp_buf& co_context::get_jmp_buf()
        {
            return m_buf;
        }


        /*
        
        Context utilities
        
        */

        void make_context_current(co_context* context);
        co_context* get_current_context();

#if defined(CPPCOSL_IMPLEMENTATION)

        static thread_local co_context* current_context;

        void make_context_current(co_context* context)
        {
            current_context = context;
        }
        co_context* get_current_context()
        {
            return current_context;
        }

#endif
    }


    template<typename T>
    class co_local
    {
    private:
        T* m_ptr;

    public:
        co_local();

        co_local(const T& obj);

        template<typename E>
        co_local(std::initializer_list<E>);

        template<typename ... Args>
        co_local(Args&& ... args);

        co_local(const co_local<T>&);
        co_local(co_local<T>&&);
        co_local<T>& operator=(const co_local<T>&);
        co_local<T>& operator=(co_local<T>&&);

        operator T& ();

        T& operator=(const T&);
        T& operator=(T&& obj);

        T* operator->();
        T& operator*();

        auto& operator[](int n);


    };

    template<typename T>
    inline co_local<T>::co_local()
    {
        m_ptr = &detail::get_current_context()->allocate_or_get<T>();
    }

    template<typename T>
    inline co_local<T>::co_local(const T& obj)
    {
        m_ptr = &detail::get_current_context()->allocate_or_get<T>(obj);
    }

    template<typename T>
    template<typename E>
    inline co_local<T>::co_local(std::initializer_list<E> lst)
    {
        m_ptr = &detail::get_current_context()->allocate_or_get<T>(lst);
    }

    template<typename T>
    template<typename ... Args>
    inline co_local<T>::co_local(Args&& ... args)
    {
        m_ptr = &detail::get_current_context()->allocate_or_get<T>(std::forward<Args>(args)...);
    }
    template<typename T>
    co_local<T>::co_local(const co_local<T>& other)
    {
        *m_ptr = *other.m_ptr;
    }

    template<typename T>
    co_local<T>::co_local(co_local<T>&&)
    {
        *m_ptr = std::move(*other.m_ptr);
    }

    template<typename T>
    co_local<T>& co_local<T>::operator=(const co_local<T>& other)
    {
        *m_ptr = *other.m_ptr;
        return *this;
    }

    template<typename T>
    co_local<T>& co_local<T>::operator=(co_local<T>&& other)
    {
        *m_ptr = std::move(*other.m_ptr);
        return *this;
    }

    template<typename T>
    inline co_local<T>::operator T& ()
    {
        return *m_ptr;
    }

    template<typename T>
    inline T& co_local<T>::operator=(const T& obj)
    {
        *m_ptr = obj;
        return *m_ptr;
    }

    template<typename T>
    inline T& co_local<T>::operator=(T&& obj)
    {
        *m_ptr = std::move(obj);
        return *m_ptr;
    }

    template<typename T>
    inline T* co_local<T>::operator->()
    {
        return m_ptr;
    }

    template<typename T>
    inline T& co_local<T>::operator*()
    {
        return *m_ptr;
    }

    template<typename T>
    inline auto& co_local<T>::operator[](int n)
    {
        return (*m_ptr)[n];
    }

    /*
    
        Coroutine Control Block
    
    */

    namespace detail
    {
        struct co_block
        {
            coroutine function;
            co_context context;
            int status = 0;
            bool done = false;

            co_result last_result = uninit();
            int64_t next_frame = 0;
            float next_time = 0;
        };
    }


    /*
    Coroutine function implementations
    */

#if defined (CPPCOSL_IMPLEMENTATION)

    namespace detail
    {
        // Need two lists because coroutines might be started while iterating
        static std::vector<co_handle> s_running_coroutines;

        static std::mutex s_scheduled_mutex;
        static std::vector<co_handle> s_scheduled_coroutines;
    }

    static co_handle co_start(coroutine co)
    {
        std::shared_ptr<detail::co_block> block = std::make_shared<detail::co_block>();
        block->function = co;
        {
            std::lock_guard<std::mutex> lock(detail::s_scheduled_mutex);
            detail::s_scheduled_coroutines.push_back(block);
        }
        return block;
    }

    namespace detail
    {
        /// <summary>
        /// gets the current time in seconds, for scheduling purposes
        /// </summary>
        /// <returns></returns>
        static float get_current_time()
        {
            static unsigned long long start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            unsigned long long current_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            return (float)((current_time - start_time) / 1000000000.0);
        }

        /// <summary>
        /// Checks if the given coroutine can run, given its scheduling
        /// </summary>
        /// <param name="block"></param>
        /// <param name="frame"></param>
        /// <returns></returns>
        bool can_run(const co_block& block, int64_t current_frame)
        {
            switch (block.last_result.type)
            {
            case co_result_type::WaitForFrames:
                if (block.next_frame <= current_frame)
                {
                    return true;
                }
                break;
            case co_result_type::WaitForSeconds:
                if (block.next_time <= get_current_time())
                {
                    return true;
                }
                break;
            case co_result_type::WaitForCoroutine:
            {
                co_block* awaiting = (co_block*)block.last_result.arg0.ptr_val;
                if (awaiting->done)
                {
                    return true;
                }
            }
            break;
            case co_result_type::Yield:
            case co_result_type::Uninit:
                return true;
            case co_result_type::Done:
                return false;
            }

            return false;
        }

        /// <summary>
        /// Processes all running coroutines and all scheduled coroutines once.
        /// </summary>
        void process_coroutines()
        {
            static int64_t frame = 0;

            //Copy over the temp list
            {
                std::lock_guard<std::mutex> lock(s_scheduled_mutex);
                s_running_coroutines.insert(s_running_coroutines.end(), s_scheduled_coroutines.begin(), s_scheduled_coroutines.end());
                s_scheduled_coroutines.resize(0);
            }
            //Run the coroutines
            for (auto block : s_running_coroutines)
            {
                if (!can_run(*block, frame))
                {
                    continue;
                }

                block->context.reset();
                make_context_current(&block->context);
                //This line is the most important line.
                //It calls the coroutine, and must only be called in one spot,
                //And the EXACT SAME SPOT in the call hierarchy EVERY TIME
                block->last_result = block->function(block->context, block->status);
                block->status = 1;
                switch (block->last_result.type)
                {
                case co_result_type::Done:
                    block->done = true;
                    break;
                case co_result_type::WaitForFrames:
                    block->next_frame = frame + block->last_result.arg0.i_val;
                    break;
                case co_result_type::WaitForSeconds:
                    block->next_time = get_current_time() + block->last_result.arg0.f_val;
                    break;
                case co_result_type::WaitForCoroutine: // Do nothing
                case co_result_type::Yield: // Do nothing
                    break;
                }
            }
            s_running_coroutines.erase(
                std::remove_if(s_running_coroutines.begin(), s_running_coroutines.end(), [](std::shared_ptr<co_block> block) { return block->done; }), 
                s_running_coroutines.end()
            );

            frame++;
        }
    }

    void cppcosl_thread_func(std::function<bool()> should_terminate)
    {
        while (!should_terminate())
        {
            detail::process_coroutines();
        }
    }

    std::thread cppcosl_start_thread(std::function<bool()> should_terminate)
    {
        std::thread cppcosl_thread(cppcosl_thread_func, should_terminate);
        return cppcosl_thread;
    }

    void co_wait(co_handle handle)
    {
        while (!handle->done);
    }



#endif //CPPCOSL_IMPLEMENTATION

    /*

        Useful coroutine library

    */


    namespace lib
    {

        template<typename ForwardIterator, typename Element>
        co_declare_args(co_foreach_impl, ForwardIterator begin, ForwardIterator end, std::function<void(const Element&)> func, co_result yield_result)
        {
            co_local<ForwardIterator> it;

            co_begin();

            for (it = begin; *it != end; (*it)++)
            {
                func(**it);
                yield_return(yield_result);
            }

            yield_break();
        }

        template<typename ForwardIterator, typename Func>
        coroutine co_foreach(ForwardIterator begin, ForwardIterator end, Func&& func, co_result yield_result)
        {
            using obj_type = decltype(*begin);
            using func_type = std::function<void(const obj_type&)>;
            return co_bind(co_foreach_impl<ForwardIterator, obj_type>, begin, end, func_type(func), yield_result);
        }

#if defined (CPPCOSL_IMPLEMENTATION)
        co_declare_args(co_timer_impl, std::function<void()> func, float period, int repetitions)
        {
            co_local<int> i;

            co_begin();

            if (repetitions == 0)
            {
                while (true)
                {
                    func();
                    yield_return(wait_for_seconds(period));
                }
            }
            else
            {
                for(i = 0; i < repetitions; i++)
                {
                    func();
                    yield_return(wait_for_seconds(period));
                }
            }

            yield_break();
        }

        coroutine co_timer(std::function<void()> func, float period, int repetitions)
        {
            return co_bind(co_timer_impl, func, period, repetitions);
        }

#endif //CPPCOSL_IIMPLEMENTATION
    }
}

#endif //CPPCOSL_INCLUDE_CPPCOSL_H