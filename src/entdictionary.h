
#pragma once

#include <fstream>
#include <cstring>
#include <map>
#include "string_nocase.h"
#include "translate.h"

struct StdStringLessNoCase
{
    using is_transparent = void;

	bool operator()( const NcString& x, const NcString& y ) const {
		return x < y;
	}
	bool operator()( const char* x, const NcString& y ) const {
		return x < y;
	}
	bool operator()( const NcString& x, const char* y ) const {
		return x < y;
	}
};

using KeyTypes = std::map<NcString, NcString, StdStringLessNoCase>;

inline KeyTypes readEntDictionary( const char *appPath ){
    std::ifstream file;
    KeyTypes keyTypes;
    {
        const char *slash = std::max( strrchr( appPath, '/' ), strrchr( appPath, '\\' ) );
        slash = std::max( slash, appPath - 1 ) + 1;
        file.open( std::string( appPath, slash - appPath ) + "ent.dic" );
    }
    if( file ){
        std::string string;
        std::getline( file, string, char( 0 ) );
        {
            const char *str = string.c_str();
            const char *end = str + string.size();
            int depth = 0;
            NcString type;
            NcString key;
            const auto isSpace = [&](){ return *str == ' ' || *str == '\t'; };
            const auto isEOL = [&](){ return *str == '\n' || *str == '\r'; };
            const auto isDelimiter = [&](){ return isSpace() || isEOL(); };
            const auto skipComment = [&](){
                const bool isLineComment = strncmp( str, "//", 2 ) == 0;
                if( isLineComment ){
                    while( str < end && !isEOL() ){ // skip it
                        ++str;
                    }
                }
                return isLineComment;
            };
            const auto tryInsertKey = [&](){
                if( depth == 1 && !key.empty() ){
                    if( !keyTypes.emplace( key, type ).second ){
                        fprintf(stderr, translate("Duplicate key in ent.dic: \"%s\"\n").c_str(), key.c_str() );
                    }
                    key.clear();
                }
            };

            while( str < end ){
                if( isDelimiter() ){
                    tryInsertKey();
                }
                else if( skipComment() ){
                    --str; // provide delimiter to the next iteration
                }
                else if( *str == '{' ){ // enter key level
                    depth = 1;
                    key.clear();
                }
                else if( *str == '}' ){ // quit to type level
                    tryInsertKey();
                    depth = 0;
                    type.clear();
                }
                else if( depth == 0 ){ // type level
                    type += *str;
                }
                else if( depth == 1 ){ // key level
                    key += *str;
                }
                ++str;
            }
        }
    }
    else{
        fprintf(stderr, translate("Could not open file %s for reading: %s\n").c_str(), "ent.dic", strerror(errno));
    }
    return keyTypes;
}

