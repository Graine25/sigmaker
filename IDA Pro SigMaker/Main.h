#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Version.h"
#include "Plugin.h"

// Signature types and structures
enum class SignatureType : uint32_t {
	IDA = 0,
	x64Dbg,
	Signature_Mask,
	SignatureByteArray_Bitmask
};

typedef struct {
	uint8_t value;
	bool isWildcard;
} SignatureByte;

using Signature = std::vector<SignatureByte>;

struct SignatureResult {
	bool ok = false;
	Signature signature;
	std::string errorMessage;

	SignatureResult( ) = default;
	SignatureResult( Signature value ) : ok( true ), signature( std::move( value ) ) {
	}
	SignatureResult( std::string error ) : ok( false ), errorMessage( std::move( error ) ) {
	}

	bool has_value( ) const {
		return ok;
	}

	const Signature& value( ) const {
		return signature;
	}

	Signature& value( ) {
		return signature;
	}

	const std::string& error( ) const {
		return errorMessage;
	}
};
