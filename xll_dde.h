#pragma once
#include "error.h"
#include <ddeml.h>
#include <compare>
#include <span>
#include <string>
#include <functional>
#include <stdexcept>

namespace DDE {

	using Tstring = std::basic_string<TCHAR>;
	using Data = std::span<BYTE>;

	// Clipboard format traits
	template<typename T> struct CF { UINT value; };
	// TODO: X-macro
	template<> struct CF<CHAR> { UINT value = CF_TEXT; };
	template<> struct CF<WCHAR> { UINT value = CF_UNICODETEXT; };

	// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddeaccessdata
	inline Data AccessData(HDDEDATA hData)
	{
		DWORD size;

		LPBYTE data = DdeAccessData(hData, &size);

		return Data(data, size);
	}

	class DataHandle {
		DWORD id;
		HDDEDATA hData;
	public:
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatedatahandle
		template<typename T = TCHAR>
		DataHandle(DWORD id, Data data, HSZ item = NULL, 
			UINT fmt = CF<T>::value, UINT cmd = HDATA_APPOWNED)
			: id(id)
		{
			hData = DdeCreateDataHandle(id, data.data(), (DWORD)data.size_bytes(),
				0, item, fmt, cmd);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreedatahandle
		~DataHandle()
		{
			DdeFreeDataHandle(hData);
		}
	};
	class StringHandle {
		DWORD id;
		HSZ hsz;
	public:
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatestringhandlea?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeCreateStringHandle%29%3Bk%28DdeCreateStringHandle%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		StringHandle(DWORD id, LPCTSTR string, int cp = CP_WINNEUTRAL)
			: id(id)
		{
			hsz = DdeCreateStringHandle(id, string, cp);
		}
		StringHandle(const StringHandle&) = delete;
		StringHandle& operator=(const StringHandle&) = delete;
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreestringhandle?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeFreeStringHandle);k(DdeFreeStringHandle);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		~StringHandle()
		{
			DdeFreeStringHandle(id, hsz);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecmpstringhandles?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeCmpStringHandles);k(DdeCmpStringHandles);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		auto operator<=>(HSZ hsz_) const
		{
			return (hsz, hsz_) <=> 0;
		}

		operator const HSZ() const
		{
			return hsz;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddequerystringa?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeQueryString%29%3Bk%28DdeQueryString%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		Tstring QueryString() const
		{
			Tstring sz;

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
		using RequestHandler = std::function<Tstring(const Tstring& item)>;
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

					Tstring item(itemBuf);
					Tstring value = self->onRequest_(item);

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