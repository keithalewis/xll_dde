#include "xll_dde.h"
//#include <iostream>

int main()
{
    DDE::Server server(
        "MyServer",
        "MyTopic",
        [](const std::string& item) {
            if (item == "Hello")
                return std::string("Hello from modern C++ DDE");
            return std::string("Unknown item");
        }
    );

    server.runMessageLoop();
    return 0;
}
