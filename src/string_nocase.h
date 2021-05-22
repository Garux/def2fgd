//Copyright (c) 2015 Roman Chistokhodov

#pragma once

#include <cstring>

/// \brief Returns <0 if \p string is lexicographically less than \p other after converting both to lower-case.
/// Returns >0 if \p string is lexicographically greater than \p other after converting both to lower-case.
/// Returns 0 if \p string is lexicographically equal to \p other after converting both to lower-case.
/// O(n)
inline int string_compare_nocase( const char* string, const char* other ){
#ifdef WIN32
	return _stricmp( string, other );
#else
	return strcasecmp( string, other );
#endif
}

/// \brief Returns <0 if [\p string, \p string + \p n) is lexicographically less than [\p other, \p other + \p n).
/// Returns >0 if [\p string, \p string + \p n) is lexicographically greater than [\p other, \p other + \p n).
/// Returns 0 if [\p string, \p string + \p n) is lexicographically equal to [\p other, \p other + \p n).
/// Treats all ascii characters as lower-case during comparisons.
/// O(n)
inline int string_compare_nocase_n( const char* string, const char* other, std::size_t n ){
#ifdef WIN32
	return _strnicmp( string, other, n );
#else
	return strncasecmp( string, other, n );
#endif
}

/// \brief Returns true if \p string is lexicographically equal to \p other.
/// Treats all ascii characters as lower-case during comparisons.
/// O(n)
inline bool string_equal_nocase( const char* string, const char* other ){
	return string_compare_nocase( string, other ) == 0;
}

/// \brief Returns true if [\p string, \p string + \p n) is lexicographically equal to [\p other, \p other + \p n).
/// Treats all ascii characters as lower-case during comparisons.
/// O(n)
inline bool string_equal_nocase_n( const char* string, const char* other, std::size_t n ){
	return string_compare_nocase_n( string, other, n ) == 0;
}

#include <string>

struct char_traits_nocase : public std::char_traits<char> {
    static char to_lower( char ch ) {
        return std::tolower( static_cast<unsigned char>( ch ) );
    }
    static bool eq( char c1, char c2 ) {
         return to_lower( c1 ) == to_lower( c2 );
     }
    static bool lt( char c1, char c2 ) {
         return to_lower( c1 ) < to_lower( c2 );
    }
    static int compare( const char* s1, const char* s2, std::size_t n ) {
		return string_compare_nocase_n( s1, s2, n );
    }
    static const char* find( const char* s, std::size_t n, char a ) {
        auto const la ( to_lower( a ) );
        while ( n-- != 0 )
        {
            if ( to_lower( *s ) == la )
                return s;
            s++;
        }
        return nullptr;
    }
};

using NcString = std::basic_string<char, char_traits_nocase>;
