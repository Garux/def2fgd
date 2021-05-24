//Copyright (c) 2015 Roman Chistokhodov

#include <cmath>

#include <algorithm>

#include "entity.h"

unsigned colorFromFloat(float f)
{
    return std::min( static_cast<unsigned>(ceil(f*255)), 255u );
}

std::string withoutQuotes(std::string str)
{
    return std::string(str.begin(), std::remove(str.begin(), str.end(), '"'));
}

Key::Key(const NcString& keyname, const std::string& keydescription) : name(keyname), description(keydescription)
{

}

Key::Key(const NcString& keyname, const std::string& keydescription, const std::string& keytype) : name(keyname), description(keydescription), type(keytype)
{

}

bool Entity::hasKey(const NcString& name) const
{
    for (size_t i=0; i<keys.size(); ++i)
    {
        if (keys[i].name == name) {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& out, const Entity& entity)
{
    out << entity.name << (entity.solid ? " solid" : " point");
    for (size_t i=0;i<6;++i)
        out << " " << entity.box[i];
    out << std::endl;
    for (size_t i=0; i<entity.keys.size(); ++i)
    {
        out << entity.keys[i].name.c_str() << " ";
    }
    if (!entity.model.empty())
        out << "model=" <<entity.model;
    out << std::endl;
    return out;
}
