#pragma once
#include <windows.h>
#include <ddeml.h>
#include <string>
#include <functional>
#include <stdexcept>

class DDEServer {
public:
    using RequestHandler =
        std::function<std::string(const std::string& item)>;

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
        DDEServer* self = nullptr;

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
                self = reinterpret_cast<DDEServer*>(convInfo.hUser);
            }
        }

        if (!self)
            return (HDDEDATA)NULL;

        switch (uType) {
        case XTYP_REQUEST:
            if (uFmt == CF_TEXT && self->onRequest_) {
                char itemBuf[256];
                DdeQueryStringA(self->idInst_, hsz2,
                    itemBuf, sizeof(itemBuf), CP_WINANSI);

                std::string item(itemBuf);
                std::string value = self->onRequest_(item);

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
    DDEServer(const std::string& service,
        const std::string& topic,
        RequestHandler handler)
        : onRequest_(std::move(handler))
    {
        // Pass 'this' as the last parameter to DdeInitialize
        UINT res = DdeInitialize(
            &idInst_,
            (PFNCALLBACK)DdeCallback,
            APPCMD_FILTERINITS | CBF_SKIP_CONNECT_CONFIRMS,
            (DWORD)(ULONG_PTR)this  // Pass 'this' pointer here
        );

        if (res != DMLERR_NO_ERROR)
            throw std::runtime_error("DdeInitialize failed");

        hszService_ = DdeCreateStringHandleA(idInst_, service.c_str(), CP_WINANSI);
        hszTopic_ = DdeCreateStringHandleA(idInst_, topic.c_str(), CP_WINANSI);

        if (!DdeNameService(idInst_, hszService_, 0, DNS_REGISTER))
            throw std::runtime_error("DdeNameService failed");
    }

    ~DDEServer() {
        if (hszService_) DdeFreeStringHandle(idInst_, hszService_);
        if (hszTopic_)   DdeFreeStringHandle(idInst_, hszTopic_);
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