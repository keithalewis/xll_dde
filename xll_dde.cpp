#include "xll_dde.h"
#include <iostream>

using namespace DDE;

int test_dde()
{
    DDE::Server server(
        TEXT("MyServer"),
        TEXT("MyTopic"),
        [](const Tstring& item) {
            if (item == TEXT("Hello"))
                return Tstring(TEXT("Hello from modern C++ DDE"));
            return Tstring(TEXT("Unknown item"));
        }
    );
    server.runMessageLoop();
   
    return 0;
}

int main()
{
    try {
        test_dde();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
