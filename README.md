# Dependency Injection for C++ (cppdj)

Dependency Injection is a useful testing / decoupling mechanism in other languages / platforms.
cppdi is a small header-only dependency injection module

## Usage:

``` C++

#include "cppdj/cppdj.hpp"

struct foo
{
    void do_something();
}

struct bar
{
    cppdj::dep<foo> m_foo; //Automatically injected on construction

    void do_other_thing()
    {
		    m_foo->do_something();
    }
}

void main()
{
    //Register foo and bar (in order)
    //Note: doesn't currently support constructor arg forwarding
    cppdj::register_dep<foo>();
    cppdj::register_dep<bar>();

    //Get a bar
    cppdj::dep<bar> the_bar;
    the_bar->do_other_thing(); //Will call the foo do_something()
}

```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)