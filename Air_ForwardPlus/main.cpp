#include <rttr/registration>
#include <iostream>
static void f() { std::cout << "Hello World" << std::endl; }
using namespace rttr;
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::method("f", &f);
}
int main()
{
    type::invoke("f", {});
}
// outputs: "Hello World"