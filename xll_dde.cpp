#include <cassert>
#include <iostream>
#include "xll_dde.h"

using namespace DDE;

int test_dde()
{
    /*
    DDE::Service server(
        TEXT("MyServer"),
        [](const Tstring& item) {
            if (item == TEXT("Hello"))
                return Tstring(TEXT("Hello from modern C++ DDE"));
            return Tstring(TEXT("Unknown item"));
        }
    );
    auto sh = server.StringHandle(TEXT("MyText"));
    auto qs = sh.QueryString();
    assert(qs == TEXT("MyText"));
   */
    return 0;
}

int main()
{
    try {
        test_dde();
        DDE::Service service(TEXT("MyService"));
        /*
            TEXT("MyTopic"),
            [](const Tstring& item) {
                if (item == TEXT("Hello"))
                    return Tstring(TEXT("Hello from modern C++ DDE"));
                return Tstring(TEXT("Unknown item"));
            }
        );
        */
        service.runMessageLoop();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
