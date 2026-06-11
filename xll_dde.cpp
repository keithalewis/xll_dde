#include "xll_dde.h"
//#include <iostream>

using namespace DDE;

int main()
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
