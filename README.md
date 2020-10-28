# Coroutines for C++ using setjmp/longjmp

Coroutines are co-operatively scheduled user space threads. Normally, they require operating system support to implement, but this library shows how to implement them using the functionality of C's setjmp & longjmp functions
cppcosl is a header-only library.

## Usage:

``` C++

//Define the implementation in one file
#define CPPCOSL_IMPLEMENTATION
#include "cppcosl/cppcosl.hpp"
#include <iostream>

//Declaring a coroutine
co_declare(my_coroutine)
{
    //Because of how setjmp & longjmp work, we need special local variable containers
    cppcosl::co_local<int> i;

    //Import coroutine functionality here
    co_begin();

    for (i = 0; i < 10; i++)
    {
        std::cout << i << std::endl;

        //Yield the coroutine and wait 0.5 seconds to be scheduled again
        yield_return(cppcosl::wait_for_seconds(0.5f));
    }

    //Return from the coroutine
    yield_break();
}


void main()
{
    //We need to initialize the library
    bool done = false;
    auto thread = cppcosl::cppcosl_start_thread([&]{return done});

    auto handle = cppcosl::co_start(my_coroutine);

    cppcosl::co_wait(my_coroutine);

    //Let the library know we are done
    done = true;

    thread.join();
}

```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)