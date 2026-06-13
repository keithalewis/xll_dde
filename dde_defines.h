// dde_defines.h
#pragma once
#include <stdexcept>
#include <string_view>
#include <ddeml.h>

#define DDE_I2S(i) if (dw == i) return #i; 

namespace DDE {

	// Connections states (uState)
#ifdef NULL
#undef NULL
#endif
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
	enum class XST : DWORD {
		DDE_XST(X)
	};
#undef X
	static_assert((DWORD)XST::INCOMPLETE == XST_INCOMPLETE);

#define X(i) if (static_cast<DWORD>(dw) == XST_##i) return "XST_" #i; 
	// enum to string_view
	constexpr const std::string_view XST_(enum XST dw) {
		DDE_XST(X) 
		throw std::out_of_range("DDE::XST_: not found");
	}
#undef X
	static_assert(XST_(XST::INCOMPLETE) == "XST_INCOMPLETE");

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
	enum class ST : DWORD {
		DDE_ST(X)
	};
#undef X
	static_assert((DWORD)ST::CONNECTED == ST_CONNECTED);

#define X(i) if (static_cast<DWORD>(dw) == ST_##i) return "ST_" #i; 
	// enum to string_view
	constexpr const std::string_view ST_(enum ST dw) {
		DDE_ST(X) return "";
	}
#undef X
	static_assert(ST_(ST::CONNECTED) == "ST_CONNECTED");

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
	enum class Dde : DWORD {
		DDE_DDE(X)
	};
#undef X
	static_assert((DWORD)Dde::FACK == DDE_FACK);

#define X(i) if (static_cast<DWORD>(dw) == DDE_##i) return "DDE_" #i; 
	// enum to string_view
	constexpr const std::string_view DDE_(enum Dde dw) {
		DDE_DDE(X) return "";
	}
#undef X
	static_assert(DDE_(Dde::FACK) == "DDE_FACK");

#define X(i) if (s == "DDE_" #i) return Dde::##i;
	// string_view to enum
	constexpr Dde _DDE(const std::string_view s) {
		DDE_DDE(X)
			throw std::out_of_range("DDE::_DDE: not found");
	}
#undef X
	static_assert(_DDE("DDE_FACK") == Dde::FACK);

#define DDE_CODEPAGES(X) \
	X(CP_WINANSI) \
	X(CP_WINUNICODE) \

	constexpr const char* CODEPAGE(unsigned long dw) {
		DDE_CODEPAGES(DDE_I2S) return "";
	}

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
#define DDE_XTYPS(X) \
	X(XTYP_ERROR) \
	X(XTYP_ADVDATA) \
	X(XTYP_ADVREQ) \
	X(XTYP_ADVSTART) \
	X(XTYP_ADVSTOP) \
	X(XTYP_EXECUTE) \
	X(XTYP_CONNECT) \
	X(XTYP_CONNECT_CONFIRM) \
	X(XTYP_XACT_COMPLETE) \
	X(XTYP_POKE) \
	X(XTYP_REGISTER) \
	X(XTYP_REQUEST) \
	X(XTYP_DISCONNECT) \
	X(XTYP_UNREGISTER) \
	X(XTYP_WILDCONNECT) \

	constexpr const char* XTYP(unsigned long dw) {
		DDE_XTYPS(DDE_I2S) return "";
	}

} // namespace DDE

#undef DDE_I2S