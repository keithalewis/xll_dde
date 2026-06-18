#pragma once
#include "error.h"
#include "dde_defines.h"
#include <cassert>
#include <map>
#include <span>
#include <string>
#include <functional>
#include <stdexcept>
#include <concepts>

namespace DDE {

	template<typename T>
	concept is_win_char = std::same_as<T, CHAR> or std::same_as<T, WCHAR>;

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
	inline bool operator==(const Data& a, const Data& b)
	{
		return std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	class DataHandle {
		HDDEDATA hData;
	public:
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatedatahandle
		DataHandle(DWORD id, Data data, HSZ item = NULL, 
			UINT fmt = CF_<TCHAR>::value, UINT cmd = HDATA_APPOWNED)
			: hData{ DdeCreateDataHandle(id, data.data(), (DWORD)data.size_bytes(),
				0, item, fmt, cmd) }
		{ }
		// Create from null terminated value.
		DataHandle(DWORD id, LPCSTR data, HSZ item = NULL, UINT cmd = HDATA_APPOWNED)
			: DataHandle(id, Data(LPBYTE(data), strlen(data) + 1), item,  CF_TEXT, cmd)
		{ }
		// Create from null terminated UNICODE value.
		DataHandle(DWORD id, LPCTSTR data, HSZ item = NULL, UINT cmd = HDATA_APPOWNED)
			: DataHandle(id, Data(LPBYTE(data), wcslen(data) + 1), item, CF_UNICODETEXT, cmd)
		{ }
		DataHandle(const DataHandle&) = delete;
		DataHandle& operator=(const DataHandle&) = delete;
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreedatahandle
		~DataHandle()
		{
			// Called by Excel when returned from callback.
			// DdeFreeDataHandle(hData);
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
		// Will resize if data.size() + off > current size 
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
		BOOL Free()
		{
			return DdeFreeDataHandle(hData);
		}
	};

	template<is_win_char T>
	class StringHandle {
		DWORD id;
		HSZ hsz;
	public:
		StringHandle()
			: id(0), hsz(0)
		{ }
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecreatestringhandlea?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeCreateStringHandle%29%3Bk%28DdeCreateStringHandle%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		StringHandle(DWORD id, const std::basic_string<T>& string)
			: id(id)
		{
			hsz = DdeCreateStringHandle(id, string.data(), (UINT)CP_<T>::value);
		}
		StringHandle(DWORD id, HSZ hsz)
			: id(id), hsz(hsz)
		{ }
		// TODO: reference counting??? DdeKeepStringHandle
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
			sh.hsz = 0;

			return *this;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddefreestringhandle?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeFreeStringHandle);k(DdeFreeStringHandle);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		~StringHandle()
		{
			DdeFreeStringHandle(id, hsz);
		}
	
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddekeepstringhandle?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(WIN_DDE%2FDDE%3A%3AStringHandle%3A%3ADdeKeepStringHandle);k(DDE%3A%3AStringHandle%3A%3ADdeKeepStringHandle);k(DdeKeepStringHandle);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		// Increments the usage count associated with the specified handle. 
		// This function enables an application to save a string handle passed
		// to the application's Dynamic Data Exchange (DDE) callback function.
		// Otherwise, a string handle passed to the callback function is deleted 
		// when the callback function returns. 
		BOOL Keep()
		{
			return DdeKeepStringHandle(id, hsz);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddecmpstringhandles?devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk(DDEML%2FDdeCmpStringHandles);k(DdeCmpStringHandles);k(DevLang-C%2B%2B);k(TargetOS-Windows)%26rd%3Dtrue
		auto operator<=>(HSZ hsz_) const
		{
			return DdeCmpStringHandles(hsz, hsz_) <=> 0;
		}

		operator HSZ() const
		{
			return hsz;
		}
		// https://learn.microsoft.com/en-us/windows/win32/api/ddeml/nf-ddeml-ddequerystringa?redirectedfrom=MSDN&devlangs=cpp&f1url=%3FappId%3DDev18IDEF1%26l%3DEN-US%26k%3Dk%28DDEML%2FDdeQueryString%29%3Bk%28DdeQueryString%29%3Bk%28DevLang-C%2B%2B%29%3Bk%28TargetOS-Windows%29%26rd%3Dtrue
		std::basic_string<T> QueryString() const
		{
			std::basic_string<T> sz;

			// TODO: Keep???
			DWORD nb = DdeQueryString(id, hsz, 0, 0, (UINT)CP_<TCHAR>::value);
			if (nb != 0) {
				sz.resize(nb);
				// TODO: Keep???
				nb = DdeQueryString(id, hsz, sz.data(), nb + 1, (UINT)CP_<TCHAR>::value);
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

	struct TopicHandlers {
		// handleConnect // TRUE/FALSE
		// handleConnectConfirm // 0 
		// handleDisconnect // 0
		// HDDEDATA/NULL
		std::function<HDDEDATA(UINT uFmt, HCONV hConv, HSZ item, HSZ topic)> handleRequest;
		// handlePoke // DDE_FACK or DDE_FNOTPROCESSED
		// handleAdvExecute // DDE_FACK or DDE_FNOTPROCESSED
		// handleAdvStart // TRUE/FALSE
		// handleAdvStop // 0
		// handleAdvData // DDE_FACK or DDE_FNOTPROCESSED
		// handleAdvReq
	};
	
	class Service;
	inline Service* g_pService;

	class Service {
		DWORD id_ = 0;
		Tstring service_;
		// Global map from service name to id and Service.
		static inline std::map<Tstring, std::pair<DWORD, Service*>> g_idService;
		// Map topic to handlers
		std::map<Tstring, TopicHandlers> topic_; //
	public:
		// Create service id and register service name.
		Service(const Tstring& service, DWORD cmd = APPCLASS_STANDARD)
			: service_(service)
		{
			// TODO: cmd |= APPCMD_FILTERINITS | CBF_SKIP_CONNECT_CONFIRMS, 0);
			if (DMLERR_NO_ERROR != DdeInitialize(&id_, (PFNCALLBACK)DdeCallback, cmd, 0)) {
				throw std::runtime_error(DDE::DMLERR_(DdeGetLastError(id_)));
			}

			HSZ _service = Hsz(id_, service_);
			if (0 == DdeNameService(id_, _service, 0, DNS_REGISTER)) {
				throw std::runtime_error(DDE::DMLERR_(DdeGetLastError(id_)));
			}	

			// TODO: ???check if service exists???
			g_pService = this;
			g_idService[service_] = std::pair(id_, this);
		}
		Service(const Service&) = delete;
		Service& operator=(const Service&) = delete;
		~Service() 
		{
			if (id_) {
				DdeNameService(id_, Hsz(id_, service_), 0, DNS_UNREGISTER);
				DdeUninitialize(id_);
				g_idService.erase(service_);
			}
		}
		DWORD id() const
		{
			return id_;
		}
		Tstring service() const
		{
			return service_;
		}

		Service& setTopic(const Tstring& topic, const TopicHandlers& th)
		{
			topic_[topic] = th;

			return *this;
		}
		const TopicHandlers* getTopic(const Tstring& topic) const
		{
			const auto tp = topic_.find(topic);

			return tp != topic_.end() ? &(tp->second) : nullptr;
		}
		
		// Supply service id for DDEML functions
		DDE::DataHandle DataHandle(Data data, HSZ item, UINT fmt = CF_<TCHAR>::value, UINT cmd = HDATA_APPOWNED)
		{
			return DDE::DataHandle(id_, data, item, fmt, cmd);
		}

		DDE::StringHandle<TCHAR> StringHandle(const Tstring& string)
		{
			return DDE::StringHandle(id_, string);
		}

		// ---- Instance callback ----
		HDDEDATA Callback(UINT uType, UINT uFmt, HCONV hConv,
			HSZ topic, HSZ service, HDDEDATA hData)
		{
			const TopicHandlers* ptopic = getTopic(*Hsz(id_, topic));
			// Lookup topic handlers
			CONVINFO hInfo = { 0 };
			DdeQueryConvInfo(hConv, QID_SYNC, &hInfo);
			//std::string s;
			//Tstring ts;
			const char* s = XTYP_(uType);
			/*
			ts = Hsz(id_, hInfo.hszSvcPartner).QueryString();
			ts = Hsz(id_, hInfo.hszServiceReq).QueryString();
			ts = Hsz(id_, hInfo.hszTopic).QueryString();
			ts = Hsz(id_, hInfo.hszItem).QueryString();
			s = XTYP_(hInfo.wType);
			ts = *Hsz(id_, topic);
			ts = *Hsz(id_, service);
			*/
			// May throw std::bad_function_call.
			switch (uType) {
			case XTYP_CONNECT: {
				BOOL b;
				b = DdeKeepStringHandle(id_, service);
				auto svc = *Hsz(id_, service);
				svc = *Hsz(id_, service);
				//auto cnd = Service::g_idService[*Hsz(id_, service)];
				return (HDDEDATA)(service_ == svc && Service::g_idService.count(*Hsz(id_, topic)));
			}
			case XTYP_REQUEST:
				return ptopic->handleRequest(uFmt, hConv, hInfo.hszItem, topic);

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
	private:
		static HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hConv,
			HSZ topic, HSZ service, HDDEDATA hData, ULONG_PTR dw1, ULONG_PTR dw2)
		{
			return g_pService->Callback(uType, uFmt, hConv, topic, service, hData);
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