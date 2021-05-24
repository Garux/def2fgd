//Copyright (c) 2015 Roman Chistokhodov

#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <string>
#include <ostream>
#include <array>
#include "string_nocase.h"

unsigned colorFromFloat(float f);

std::string withoutQuotes(std::string str);

struct Key
{
    Key(const NcString& keyname, const std::string& keydescription);
    Key(const NcString& keyname, const std::string& keydescription, const std::string& keytype);
    NcString name;
    std::string description;
    std::string type;
};

struct Entity
{
    std::string name;
    std::string description;
    std::vector<Key> keys;

    struct SpawnFlag{
        NcString name;
        std::string description;
        bool operator==( const NcString& string ) const {
            return name == string;
        }
    };
    using SpawnFlags = std::array<SpawnFlag, 32>;
    SpawnFlags spawnflags;

    float color[3] = { .784f, .0f, .784f };
    int box[6] =     { 0 };
    bool solid =     false;
    std::string model;

    bool hasKey(const NcString& name) const;
};

std::ostream& operator<<(std::ostream& out, const Entity& entity);

#endif
