#include "Utils.h"

#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#endif

bool SetClipboardText( std::string_view text ) {
	if( text.empty( ) ) {
		return false;
	}

#if defined(_WIN32)
	if( OpenClipboard( NULL ) == false || EmptyClipboard( ) == false ) {
		return false;
	}

	auto memoryHandle = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, text.size( ) + 1 );
	if( memoryHandle == nullptr ) {
		CloseClipboard( );
		return false;
	}

	auto textMem = reinterpret_cast<char*>( GlobalLock( memoryHandle ) );
	if( textMem == nullptr ) {
		GlobalFree( memoryHandle );
		CloseClipboard( );
		return false;
	}

	memcpy( textMem, text.data( ), text.size( ) );
	GlobalUnlock( memoryHandle );
	auto handle = SetClipboardData( CF_TEXT, memoryHandle );
	GlobalFree( memoryHandle );
	CloseClipboard( );

	if( handle == nullptr ) {
		return false;
	}

	return true;
#elif defined(__APPLE__)
	FILE* pipe = popen( "pbcopy", "w" );
	if( pipe == nullptr ) {
		return false;
	}

	const auto bytesWritten = fwrite( text.data( ), 1, text.size( ), pipe );
	const auto closeResult = pclose( pipe );
	return bytesWritten == text.size( ) && closeResult == 0;
#else
	return false;
#endif
}

bool GetRegexMatches( std::string string, std::regex regex, std::vector<std::string>& matches ) {
	std::sregex_iterator iter( string.begin( ), string.end( ), regex );
	std::sregex_iterator end;

	matches.clear( );

	while( iter != end ) {
		matches.push_back( iter->str( ) );
		++iter;
	}
	return !matches.empty( );
}
