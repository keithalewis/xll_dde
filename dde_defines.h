// dde_defines.hh
#pragma once
#include <stdexcept>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#include <ddeml.h>

#define DDE_I2S(i) if (dw == i) return #i; 

namespace DDE {

	// Codepage traits
	enum class CP : UINT {
		WINANSI = CP_WINANSI,
		WINUNICODE = CP_WINUNICODE
	};

	template<typename T> struct CP_ { static const CP codepage; };
	template<> struct CP_<CHAR> { static const CP codepage = CP::WINANSI; };
	template<> struct CP_<WCHAR> { static const CP codepage = CP::WINUNICODE; };

	// Clipboard format traits
	template<typename T> struct CF { static const UINT text; };
	template<> struct CF<CHAR> { static const UINT text = CF_TEXT; };
	template<> struct CF<WCHAR> { static const UINT text = CF_UNICODETEXT; };

	// Connections states (uState)
#undef NULL

#define DDE_XST(X) \
	X(NULL) \
	X(INCOMPLETE) \
	X(CONNECTED) \
	X(INIT1) \
	X(INIT2) \
	X(REQSENT) \
	X(DATARCVD) \
	X(POKESENT) \
	X(POKEACKRCVD) \
	X(EXECSENT) \
	X(EXECACKRCVD) \
	X(ADVSENT) \
	X(UNADVSENT) \
	X(ADVACKRCVD) \
	X(UNADVACKRCVD) \
	X(ADVDATASENT) \
	X(ADVDATAACKRCVD) \

#define X(i) i = XST_##i,
	enum class XST : UINT {
		DDE_XST(X)
	};
#undef X
	static_assert((UINT)XST::INCOMPLETE == XST_INCOMPLETE);

#define X(i) if (ui == XST_##i) return "XST_" #i; 
	// enum to string_view
	constexpr const std::string_view XST_(UINT ui) {
		DDE_XST(X) 
		throw std::out_of_range("DDE::XST_: not found");
	}
#undef X
	static_assert(XST_((UINT)XST::INCOMPLETE) == "XST_INCOMPLETE");

#define X(i) if (s == "XST_" #i) return XST::##i;
	// string_view to enum
	constexpr XST _XST(const std::string_view s) {
		DDE_XST(X)
			throw std::out_of_range("DDE::_XST: not fo");
	}
#undef X
	static_assert(_XST("XST_INCOMPLETE") == XST::INCOMPLETE);

#define NULL 0

	// Conversation status bits (fsStatus)
#define DDE_ST(X) \
	X(CONNECTED) \
	X(ADVISE) \
	X(ISLOCAL) \
	X(BLOCKED) \
	X(CLIENT) \
	X(TERMINATED) \
	X(INLIST) \
	X(BLOCKNEXT) \
	X(ISSELF) \

#define X(i) i = ST_##i,
	enum class ST : UINT {
		DDE_ST(X)
	};
#undef X
	static_assert((UINT)ST::CONNECTED == ST_CONNECTED);

#define X(i) if (ui == ST_##i) return "ST_" #i; 
	// enum to string_view
	constexpr const char* ST_(UINT ui) {
		DDE_ST(X) return "";
	}
#undef X
	static_assert(ST_((UINT)ST::CONNECTED) == "ST_CONNECTED");

#define X(i) if (s == "ST_" #i) return ST::##i;
	// string_view to enum
	constexpr ST _ST(const std::string_view s) {
		DDE_ST(X)
		throw std::out_of_range("DDE::_ST: not found");
	}
#undef X
	static_assert(_ST("ST_CONNECTED") == ST::CONNECTED);

	/* DDE constants for wStatus field */
#define DDE_DDE(X) \
	X(FACK) \
	X(FBUSY) \
	X(FDEFERUPD) \
	X(FACKREQ) \
	X(FRELEASE) \
	X(FREQUESTED) \
	X(FAPPSTATUS) \
	X(FNOTPROCESSED) \

#define X(i) i = DDE_##i,
	enum class Dde : UINT {
		DDE_DDE(X)
	};
#undef X
	static_assert((UINT)Dde::FACK == DDE_FACK);

#define X(i) if (ui == DDE_##i) return "DDE_" #i; 
	// enum to string_view
	constexpr const char* DDE_(UINT ui) {
		DDE_DDE(X) return "";
	}
#undef X
	static_assert(DDE_((UINT)Dde::FACK) == "DDE_FACK");

#define X(i) if (s == "DDE_" #i) return Dde::##i;
	// string_view to enum
	constexpr Dde _DDE(const std::string_view s) {
		DDE_DDE(X)
		throw std::out_of_range("DDE::_DDE: not found");
	}
#undef X
	static_assert(_DDE("DDE_FACK") == Dde::FACK);


#define DDE_XTYPF(X) \
	X(XTYPF_NOBLOCK) \
	X(XTYPF_NODATA) \
	X(XTYPF_ACKREQ) \

#define DDE_XCLASS(X) \
	X(XCLASS_MASK) \
	X(XCLASS_BOOL) \
	X(XCLASS_DATA) \
	X(XCLASS_FLAGS) \
	X(XCLASS_NOTIFICATION) \

	// Transaction types
#ifdef ERROR
#define _ERROR ERROR
#undef ERROR
#endif

#define DDE_XTYP(X) \
	X(ERROR) \
	X(ADVDATA) \
	X(ADVREQ) \
	X(ADVSTART) \
	X(ADVSTOP) \
	X(EXECUTE) \
	X(CONNECT) \
	X(CONNECT_CONFIRM) \
	X(XACT_COMPLETE) \
	X(POKE) \
	X(REGISTER) \
	X(REQUEST) \
	X(DISCONNECT) \
	X(UNREGISTER) \
	X(WILDCONNECT) \

#define X(i) i = XTYP_##i,
	enum class XTYP : UINT {
		DDE_XTYP(X)
	};
#undef X
	static_assert((UINT)XTYP::ERROR == XTYP_ERROR);

#define X(i) if (ui == XTYP_##i) return "XTYP_" #i; 
	// enum to string_view
	constexpr const char* XTYP_(UINT ui) {
		DDE_XTYP(X) return "";
	}
#undef X
	static_assert(XTYP_((UINT)XTYP::ERROR) == "XTYP_ERROR");

#define X(i) if (s == "XTYP_" #i) return XTYP::##i;
	// string_view to enum
	constexpr XTYP _XTYP(const std::string_view s) {
		DDE_XTYP(X)
		throw std::out_of_range("DDE::_XTYP: not found");
	}
#undef X
	static_assert(_XTYP("XTYP_ERROR") == XTYP::ERROR);

#ifdef _ERROR
#define ERROR _ERROR
#undef _ERROR
#endif

#ifdef NO_ERROR
#define _NO_ERROR NO_ERROR
#undef NO_ERROR
#endif

#define DDE_DMLERR(X) \
	X(NO_ERROR) \
	X(ADVACKTIMEOUT) \
	X(BUSY) \
	X(DATAACKTIMEOUT) \
	X(DLL_NOT_INITIALIZED) \
	X(DLL_USAGE) \
	X(EXECACKTIMEOUT) \
	X(INVALIDPARAMETER) \
	X(LOW_MEMORY) \
	X(MEMORY_ERROR) \
	X(NOTPROCESSED) \
	X(NO_CONV_ESTABLISHED) \
	X(POKEACKTIMEOUT) \
	X(POSTMSG_FAILED) \
	X(REENTRANCY) \
	X(SERVER_DIED) \
	X(SYS_ERROR) \
	X(UNADVACKTIMEOUT) \
	X(UNFOUND_QUEUE_ID) \

#define X(i) i = DMLERR_##i,
	enum class DMLERR : UINT {
		DDE_DMLERR(X)
	};
#undef X
	static_assert((UINT)DMLERR::BUSY == DMLERR_BUSY);

#define X(i) if (ui == DMLERR_##i) return "DMLERR_" #i; 
	// enum to string_view
	constexpr const char* DMLERR_(UINT ui) {
		DDE_DMLERR(X) return "";
	}
#undef X
	static_assert(DMLERR_((UINT)DMLERR::BUSY) == "DMLERR_BUSY");

#define X(i) if (s == "DMLERR_" #i) return DMLERR::##i;
	// string_view to enum
	constexpr DMLERR _DMLERR(const std::string_view s) {
		DDE_DMLERR(X)
		throw std::out_of_range("DDE::_DMLERR: not found");
	}
#undef X
	static_assert(_DMLERR("DMLERR_BUSY") == DMLERR::BUSY);

#ifdef _NO_ERROR
#define NO_ERROR _NO_ERROR
#undef _NO_ERROR
#endif

} // namespace DDE

#undef DDE_I2S