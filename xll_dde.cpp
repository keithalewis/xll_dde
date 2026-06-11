#include "xll_dde.h"
//#include <iostream>

using namespace DDE;

int main()
{
    DDE::Server server(
        TEXT("MyServer"),
        TEXT("MyTopic"),
        [](const tstring& item) {
            if (item == TEXT("Hello"))
                return tstring(TEXT("Hello from modern C++ DDE"));
            return tstring(TEXT("Unknown item"));
        }
    );

    server.runMessageLoop();
    return 0;
}
