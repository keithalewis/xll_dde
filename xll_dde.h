#pragma once
#include "error.h"
#include "dde_defines.h"
#include <cassert>
#include <map>
#include <span>
#include <string>
#include <functional>
#include <stdexcept>

namespace DDE {

	using Tstring = std::basic_string<TCHAR>;
	using Data = std::span<BYTE>;

	// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddeaccessdata
	// Return data as a span.
	inline Data AccessData(HDDEDATA hData)
	{
		DWORD size;

		LPBYTE data = DdeAccessData(hData, &size);

		return Data(data, size);
	}

	class DataHandle {
		HDDEDATA hData;
	public:
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatedatahandle
		DataHandle(DWORD id, Data data, HSZ item = NULL, 
			UINT fmt = CF<TCHAR>::text, UINT cmd = HDATA_APPOWNED)
			: hData{ DdeCreateDataHandle(id, data.data(), (DWORD)data.size_bytes(),
				0, item, fmt, cmd) }
		{ }
		// Create from null terminated text.
		DataHandle(DWORD id, LPCSTR data, HSZ item = NULL, UINT cmd = HDATA_APPOWNED)
		{ 
			hData = DdeCreateDataHandle(id, (LPBYTE)data, DWORD(strlen(data) + 1),
				0, item, CF_TEXT, cmd);
		}
		// Create from null terminated UNICODE text.
		DataHandle(DWORD id, LPCTSTR data, HSZ item = NULL, UINT cmd = HDATA_APPOWNED)
		{
			hData = DdeCreateDataHandle(id, (LPBYTE)data, DWORD((wcslen(data) + 1) * sizeof(WCHAR)),
				0, item, CF_UNICODETEXT, cmd);
		}
		DataHandle(const DataHandle&) = delete;
		DataHandle& operator=(const DataHandle&) = delete;
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreedatahandle
		~DataHandle()
		{
			DdeFreeDataHandle(hData);
		}

		// After a data handle has been used as a parameter in another 
		// Dynamic Data Exchange Management Library function or 
		// has been returned by a DDE callback function, 
		// the handle can be used only for read access 
		// to the DDE object identified by the handle.
		operator const HDDEDATA() const
		{
			return hData;
		}

		Data Access() const
		{
			return AccessData(hData);
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddeadddata
		// 
		DataHandle& Add(Data data, DWORD off = 0)
		{
			hData = DdeAddData(hData, data.data(), (DWORD)data.size(), off);

			return *this;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddeunaccessdata?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeUnaccessData);k(DdeUnaccessData);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		// An application must call this function after it has finished accessing the object.
		BOOL Unaccess()
		{
			return DdeUnaccessData(hData);
		}
	};

	struct HandleRequest {
		//request;
		//poke;
		//advise;
	};

	template<typename T>
	// require T = CHAR or TCHART
	class StringHandle {
		DWORD id;
		HSZ hsz;
	public:
		StringHandle()
			: id(0), hsz(0)
		{ }
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatestringhandlea?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeCreateStringHandle%29%3Bk%28DdeCreateStringHandle%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		StringHandle(DWORD id, const std::basic_string_view<T> string)
			: id(id)
		{
			hsz = DdeCreateStringHandle(id, string.data(), (UINT)CP_<T>::codepage);
		}
		StringHandle(DWORD id, HSZ hsz)
			: id(id), hsz(hsz)
		{ }
		// TODO: reference counting???
		StringHandle(const StringHandle&) = delete;
		StringHandle(StringHandle&& sh) noexcept
			: id(sh.id), hsz(sh.hsz)
		{ }
		StringHandle& operator=(const StringHandle&) = delete;
		StringHandle& operator=(StringHandle&& sh) noexcept
		{
			id = sh.id;
			hsz = sh.hsz;
			sh.id = 0;
			hsz = 0;

			return *this;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreestringhandle?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeFreeStringHandle);k(DdeFreeStringHandle);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		~StringHandle()
		{
			DdeFreeStringHandle(id, hsz);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecmpstringhandles?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeCmpStringHandles);k(DdeCmpStringHandles);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		auto operator<=>(HSZ hsz_) const
		{
			return DdeCmpStringHandles(hsz, hsz_) <=> 0;
		}

		operator const HSZ() const
		{
			return hsz;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddequerystringa?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeQueryString%29%3Bk%28DdeQueryString%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		std::basic_string<T> QueryString() const
		{
			std::basic_string<T> sz;

			DWORD nb = DdeQueryString(id, hsz, 0, 0, (UINT)CP_<TCHAR>::codepage);
			if (nb != 0) {
				sz.resize(nb + 1);
				nb = DdeQueryString(id, hsz, sz.data(), nb + 1, (UINT)CP_<TCHAR>::codepage);
			}

			return sz;
		}
		std::basic_string<T> operator*() const
		{
			return QueryString();
		}
	};

	// HSZ value type
	using Hsz = StringHandle<TCHAR>;

	class Service;
	// Global map from service name to Id and Service.
	inline thread_local std::unordered_map<HSZ, std::pair<DWORD,Service*>> g_idService;

	// Forward declaration.
	HDDEDATA CALLBACK DdeCallback(
		UINT uType, UINT uFmt, HCONV hConv,
		HSZ topic, HSZ service, HDDEDATA hData,
		ULONG_PTR dw1, ULONG_PTR dw2);

	class Service {
		DWORD id_ = 0;
		Hsz service_;
		std::map<HSZ, int> topic_; // TODO: all topics?
	public:
		Service(const std::basic_string_view<TCHAR>& service, DWORD cmd = APPCLASS_STANDARD)
		{
			UINT ret;

			// TODO: cmd |= APPCMD_FILTERINITS | CBF_SKIP_CONNECT_CONFIRMS, 0);
			ret = DdeInitialize(&id_, (PFNCALLBACK)DdeCallback, cmd, 0);
			if (DMLERR_NO_ERROR != ret) {
				THROW_LAST_ERROR(DdeGetLastError(id_));
			}

			if (!DdeNameService(id_, service_, 0, DNS_REGISTER)) {
				THROW_LAST_ERROR(DdeGetLastError(id_));
			}

			// TODO: ???check if service exists???
			service_ = std::move(Hsz(id_, service));
			g_idService[service_] = std::pair(id_, this);
		}
		Service(const Service&) = delete;
		Service& operator=(const Service&) = delete;
		~Service() 
		{
			if (id_) {
				if (g_idService.count(service_)) {
					g_idService.erase(service_);
				}
				DdeNameService(id_, service_, 0, DNS_UNREGISTER);
				DdeUninitialize(id_);
			}
		}
		DWORD Id() const
		{
			return id_;
		}
		auto serviceName() const
		{
			return service_.QueryString();
		}
		
		// Supply service id for DDEML functions
		DDE::DataHandle DataHandle(Data data, HSZ item, UINT fmt, UINT cmd = HDATA_APPOWNED)
		{
			return DDE::DataHandle(id_, data, item, fmt, cmd);
		}

		DDE::StringHandle<TCHAR> StringHandle(const std::basic_string_view<TCHAR> string)
		{
			return DDE::StringHandle(id_, string);
		}

		// ---- Instance callback ----
		HDDEDATA Callback(UINT uType, UINT uFmt, HCONV hConv,
			HSZ topic, HSZ service, HDDEDATA hData)
		{
			CONVINFO hInfo = { 0 };
			DdeQueryConvInfo(hConv, QID_SYNC, &hInfo);

			switch (uType) {

			case XTYP_CONNECT:
				return (HDDEDATA)(service == service_ && g_idService.count(topic));

			case XTYP_REQUEST:
				return 0;// handleRequest(topic, service, uFmt);

			case XTYP_POKE:
				return 0;// handlePoke(topic, service, uFmt, hData);

			case XTYP_ADVREQ:
				return 0;// handleAdvise(topic, service, uFmt);

			case XTYP_DISCONNECT:
				return nullptr;

			default:
				return nullptr;
			}
		}
	protected:
		// Function that can be overridden in derived class
		virtual HDDEDATA handleRequest(HSZ service, HSZ topic, UINT fmt)
		{
			return 0; 
		}
	private:
		static HDDEDATA CALLBACK DdeCallback(
			UINT uType, UINT uFmt, HCONV hConv,
			HSZ topic, HSZ service, HDDEDATA hData,
			ULONG_PTR dw1, ULONG_PTR dw2)
		{
			DWORD id = 0;
			Service* pservice = reinterpret_cast<Service*>(dw2);

			// Hook up instance data.
			if (!pservice) {
				assert(g_idService.count(service));
				std::tie(id, pservice) = g_idService[service];
				// Subsequent calls will have pservice in dw2.
				DdeSetUserHandle(hConv, QID_SYNC, (ULONG_PTR)pservice);
			}
			else {
				id = pservice->Id();
			}

			return pservice->Callback(uType, uFmt, hConv, topic, service, hData);
		}
	public:
		void runMessageLoop() {
			MSG msg;
			while (GetMessage(&msg, nullptr, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	};


} // namespace DDE