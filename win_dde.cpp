#include <cassert>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "win_dde.h"

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

        // Shared data that will be updated
        static int counter = 0;
        static Tstring currentData = TEXT("Initial value");

        TopicHandlers th;

        // Handle one-time data requests
        th.handleRequest = [&service](UINT uFmt, HCONV hConv, HSZ item, HSZ topic) {
            Tstring itemName = *Hsz(service.id(), item);
            std::wcout << L"Request for item: " << itemName << std::endl;
            return DataHandle(service.id(), currentData.c_str(), item);
        };
        /*
        // Handle advise loop start requests
        th.handleAdvStart = [&service](UINT uFmt, HCONV hConv, HSZ item, HSZ topic) -> BOOL {
            Tstring itemName = *Hsz(service.id(), item);
            std::wcout << L"AdvStart requested for item: " << itemName 
                      << L" (Format: " << uFmt << L")" << std::endl;

            // You can be selective about which items/formats to accept
            // For example, only accept CF_TEXT or CF_UNICODETEXT
            if (uFmt == CF_TEXT || uFmt == CF_UNICODETEXT) {
                return TRUE; // Accept the advise loop
            }
            return FALSE; // Reject other formats
        };

        // Handle advise loop data requests (triggered by DdePostAdvise)
        th.handleAdvReq = [&service](UINT uFmt, HCONV hConv, HSZ item, HSZ topic) {
            Tstring itemName = *Hsz(service.id(), item);
            std::wcout << L"AdvReq for item: " << itemName 
                      << L" - Sending: " << currentData << std::endl;

            // Return current data for the advise loop
            return DataHandle(service.id(), currentData.c_str(), item, uFmt);
        };
        */
        service.setTopic(TEXT("MyTopic"), th);
        /*
        std::wcout << L"DDE Server started: MyService|MyTopic" << std::endl;
        std::wcout << L"Updates will be posted every 3 seconds..." << std::endl;
        std::wcout << L"Try connecting with Excel: =MyService|MyTopic!MyItem" << std::endl;

        // Set up a timer to periodically update data and notify clients
        const UINT_PTR TIMER_ID = 1;
        SetTimer(NULL, TIMER_ID, 3000, [](HWND hwnd, UINT msg, UINT_PTR id, DWORD time) {
            counter++;

            // Update the data
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_s(&tm, &time_t);

            std::wostringstream oss;
            oss << L"Update #" << counter << L" at " 
                << std::put_time(&tm, L"%H:%M:%S");
            currentData = oss.str();

            std::wcout << L"\n[Timer] Data updated: " << currentData << std::endl;

            // Notify all clients with active advise loops
            if (g_pService) {
                bool posted = g_pService->PostAdvise(TEXT("MyTopic"), TEXT("MyItem"));
                if (posted) {
                    std::wcout << L"[Timer] Posted advise notification" << std::endl;
                } else {
                    std::wcout << L"[Timer] No active advise loops" << std::endl;
                }
            }
        });
        */
        service.runMessageLoop();

        //KillTimer(NULL, TIMER_ID);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
