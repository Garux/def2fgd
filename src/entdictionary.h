
#pragma once

#include <fstream>
#include <cstring>
#include <map>
#include "string_nocase.h"
#include "translate.h"

struct StdStringLessNoCase
{
    using is_transparent = void;

	bool operator()( const std::string& x, const std::string& y ) const {
		return string_compare_nocase( x.c_str(), y.c_str() );
	}
	bool operator()( const char* x, const std::string& y ) const {
		return string_compare_nocase( x, y.c_str() );
	}
	bool operator()( const std::string& x, const char* y ) const {
		return string_compare_nocase( x.c_str(), y );
	}
};

using KeyTypes = std::map<std::string, std::string, StdStringLessNoCase>;

inline KeyTypes readEntDictionary(){
    std::ifstream file( "ent.dic" );
    KeyTypes keyTypes;
    if( file ){
        std::string string;
        std::getline( file, string, char( 0 ) );
        {
            char *str = const_cast<char*>( string.c_str() );
            const char *delim = " \n\r\t";
            const char *type = "";
            const char *token;
            while( ( token = strtok_r( str, delim, &str ) ) ){
                if( strcmp( token, "{" ) == 0 ){
                    while( ( token = strtok_r( str, delim, &str ) ) ){
                        if( strcmp( token, "}" ) == 0 ){
                            break;
                        }
                        else{
                            if( !keyTypes.emplace( token, type ).second ){
                                fprintf(stderr, translate("Duplicate key in ent.dic: \"%s\"\n").c_str(), token );
                            }
                        }
                    }
                }
                else{
                    type = token;
                }
            }
        }
    }
    else{
        fprintf(stderr, translate("Could not open file %s for reading: %s\n").c_str(), "ent.dic", strerror(errno));
    }
    return keyTypes;
}

