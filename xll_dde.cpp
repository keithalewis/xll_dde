#include <cassert>
#include <iostream>
#include "xll_dde.h"

using namespace DDE;

int test_dde()
{
    DDE::Service svc(TEXT("MyServer"));
    {
        Hsz sh = svc.StringHandle(TEXT("MyText"));
        Tstring qs = *sh;// sh.QueryString();
        Tstring qqs(TEXT("MyText"));
        bool b = (qs == TEXT("MyText"));
        b = (qs == qqs);
        assert(qs == TEXT("MyText"));

        BYTE buf[3] = { 1,2,3 };
        DataHandle dh = svc.DataHandle(Data(buf, sizeof(buf)), sh);
        Data da = dh.Access();
        assert(std::equal(da.begin(), da.end(), buf, buf + sizeof(buf)));
        DataHandle dh2 = svc.DataHandle(Data(buf, sizeof(buf)), sh);
        assert(dh != dh2);
        assert(dh.Access() == dh2.Access());

    }
    return 0;
}

int main()
{
    try {
        //test_dde();
        DDE::Service service(TEXT("MyService"));
        TopicHandlers th;
        th.handleRequest = [&service](UINT uFmt, HCONV hConv, HSZ item, HSZ topic) {
            return DataHandle(service.Id(), TEXT("Hi"), item, CF<TCHAR>::text);
            };
        service.setTopic(TEXT("MyTopic"), th);

        service.runMessageLoop();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
