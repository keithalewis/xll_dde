#include <cassert>
#include <iostream>
#include "xll_dde.h"

using namespace DDE;

int test_dde()
{
    {
        DDE::Service svc(TEXT("MyServer"));

        Hsz sh = svc.StringHandle(TEXT("MyText"));
        Tstring qs = sh.QueryString();
        Tstring qqs(TEXT("MyText"));
        bool b = (qs == TEXT("MyText"));
        b = (qs == qqs);
        assert(qs == TEXT("MyText"));

        BYTE buf[3] = { 1,2,3 };
        DataHandle dh = svc.DataHandle(Data(buf, sizeof(buf)), sh);
        Data da = dh.Access();
        assert(std::equal(da.begin(), da.end(), buf, buf + sizeof(buf)));

    }
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
