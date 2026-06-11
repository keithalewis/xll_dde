#pragma once
#include <windows.h>
#include <ddeml.h>
#include <compare>
#include <string>
#include <functional>
#include <stdexcept>

namespace DDE {

    using tstring = std::basic_string<TCHAR>;

    class StringHandle {
        DWORD id;
        HSZ hsz;
    public:
        StringHandle(DWORD id, LPCTSTR string, int cp = CP_WINNEUTRAL)
            : id(id)
        {
            hsz = DdeCreateStringHandle(id, string, cp);
        }
        StringHandle(const StringHandle&) = delete;
        StringHandle& operator=(const StringHandle&) = delete;
        ~StringHandle()
        {
            DdeFreeStringHandle(id, hsz);
        }

        auto operator<=>(HSZ hsz_) const
        {
            return DdeCmpStringHandles(hsz, hsz_) <=> 0;
        }

        operator const HSZ() const
        {
            return hsz;
        }

        std::basic_string<TCHAR> QueryString() const
        {
            std::basic_string<TCHAR> sz;
            
            DWORD nb = DdeQueryString(id, hsz, 0, 0, CP_WINNEUTRAL);
            if (nb != 0) {
                sz.resize(nb);
                nb = DdeQueryString(id, hsz, sz.data(), nb, CP_WINNEUTRAL);
            }

            return sz;
        }
    };
  
    class Server {
    public:
        using RequestHandler =
            std::function<tstring(const tstring& item)>;
        HSZ StringHandle(LPCTSTR string, int cp = CP_WINNEUTRAL)
        {
            return DDE::StringHandle(idInst_, string, cp);
        }
    private:
        DWORD idInst_ = 0;
        HSZ hszService_ = 0;
        HSZ hszTopic_ = 0;
        RequestHandler onRequest_;

        static HDDEDATA CALLBACK DdeCallback(
            UINT uType, UINT uFmt, HCONV hConv,
            HSZ hsz1, HSZ hsz2, HDDEDATA hData,
            ULONG_PTR dwData1, ULONG_PTR dwData2)
        {
            Server* self = nullptr;

            // For XTYP_CONNECT, we don't have user data yet
            if (uType == XTYP_CONNECT) {
                // Just allow the connection
                return (HDDEDATA)TRUE;
            }

            // For XTYP_CONNECT_CONFIRM, store the this pointer
            if (uType == XTYP_CONNECT_CONFIRM) {
                // dwData1 contains user-defined data from DdeInitialize (last param)
                // We'll use a global or static map, or better: use CONVINFO
                return 0;
            }

            // For other callbacks, retrieve the this pointer from CONVINFO
            if (hConv) {
                CONVINFO convInfo = { 0 };
                convInfo.cb = sizeof(CONVINFO);

                if (DdeQueryConvInfo(hConv, QID_SYNC, &convInfo)) {
                    self = reinterpret_cast<Server*>(convInfo.hUser);
                }
            }

            if (!self)
                return (HDDEDATA)NULL;

            switch (uType) {
            case XTYP_REQUEST:
                if (uFmt == CF_TEXT && self->onRequest_) {
                    TCHAR itemBuf[256];
                    DdeQueryString(self->idInst_, hsz2,
                        itemBuf, sizeof(itemBuf), CP_WINANSI);

                    tstring item(itemBuf);
                    tstring value = self->onRequest_(item);

                    return DdeCreateDataHandle(
                        self->idInst_,
                        (LPBYTE)value.c_str(),
                        static_cast<DWORD>(value.size() + 1),
                        0,
                        hsz2,
                        CF_TEXT,
                        0
                    );
                }
                break;
            }

            return (HDDEDATA)NULL;
        }

    public:
        Server(const std::basic_string<TCHAR>& service,
            const std::basic_string<TCHAR>& topic,
            RequestHandler handler,
            DWORD cmd = APPCLASS_STANDARD)
            : onRequest_(std::move(handler))
        {
            UINT res = DdeInitialize(
                &idInst_,
                (PFNCALLBACK)DdeCallback,
                APPCMD_FILTERINITS | CBF_SKIP_CONNECT_CONFIRMS,
                0
            );

            if (res != DMLERR_NO_ERROR)
                throw std::runtime_error("DdeInitialize failed");

            hszService_ = StringHandle(service.c_str(), CP_WINANSI);
            hszTopic_ = StringHandle(topic.c_str(), CP_WINANSI);

            if (!DdeNameService(idInst_, hszService_, 0, DNS_REGISTER))
                throw std::runtime_error("DdeNameService failed");
        }

        ~Server() {
            if (idInst_)     DdeUninitialize(idInst_);
        }

        void runMessageLoop() {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    };
} // namespace DDE