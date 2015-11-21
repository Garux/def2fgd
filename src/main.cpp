//Copyright (c) 2015 Roman Chistokhodov

#include <cstring>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <string>

#include "defreader.h"
#include "entreader.h"
#include "translate.h"

#ifndef DEF2FGD_VERSION
#define DEF2FGD_VERSION "unknown (was built without version)"
#endif

static bool globMatch(const char *str, const char *wild) {
    const char *tail = 0;
    const char* afterStar = 0;

    while (*str != '\0' && *wild != '*') {
        if (*wild == *str || *wild == '?') {
            wild++;
            str++;
        } else {
            return false;
        }
    }

    while (*str != '\0') {
        if (*wild == '*') {
            ++wild;
            if (*wild == '\0') {
                return true;
            }
            afterStar = wild;
            tail = str+1;
        } else if (*wild == *str || *wild == '?') {
            wild++;
            str++;
        } else {
            wild = afterStar;
            str = tail++;
        }
    }

    while (*wild == '*') {
        wild++;
    }
    return *wild == '\0';
}

static bool startsWith(const std::string& name, const char* start)
{
    return name.compare(0, strlen(start), start) == 0;
}

static bool needsOffset(const std::string& name)
{
    return startsWith(name, "item_") || startsWith(name, "weapon_") || startsWith(name, "ammo_") || name == "info_player_deathmatch" || name == "info_player_start";
}

static bool needsBob(const std::string& name)
{
    return startsWith(name, "item_") || startsWith(name, "weapon_") || startsWith(name, "ammo_");
}

struct FGDWriteOptions
{
    FGDWriteOptions();
    int bobparms[3];
    bool bob;
    
    bool useDefaultBobPatterns;
    bool useDefaultOffsetPatterns;
    
    std::vector<const char*> offsetPatterns;
    std::vector<const char*> bobPatterns;
};

FGDWriteOptions::FGDWriteOptions() : bob(false), useDefaultBobPatterns(true), useDefaultOffsetPatterns(true)
{
    bobparms[0] = bobparms[1] = bobparms[2] = 0;
}

void writefgd(std::ostream& stream, const std::vector<Entity>& entities, const FGDWriteOptions& options)
{
    stream << "//generated by def2fgd " << DEF2FGD_VERSION << "\n\n";
    
    for (size_t i=0; i<entities.size(); ++i)
    {
        Entity entity = entities[i];
        if (entity.solid) {
            stream << "@SolidClass ";
        }
        else {
            stream << "@PointClass size(";
            stream << entity.box[0] << ' ';
            stream << entity.box[1] << ' ';
            stream << entity.box[2] << ", ";
            stream << entity.box[3] << ' ';
            stream << entity.box[4] << ' ';
            stream << entity.box[5] << ')';
            
            bool addOffset = options.useDefaultOffsetPatterns && needsOffset(entity.name);
            
            if (!addOffset) {
                for (size_t j=0; j<options.offsetPatterns.size() && !addOffset; ++j) {
                    addOffset = globMatch(entity.name.c_str(), options.offsetPatterns[j]);
                }
            }
            
            if (addOffset) {
                stream << " offset(0 0 " << abs(entity.box[2]) << ")";
            }
            
            if (entity.name == "light" || entity.name == "lightJunior") {
                stream << " iconsprite(\"sprites/light.spr\") flags(Light) ";
                
                if (!entity.hasKey("_color")) {
                    entity.keys.push_back(Key("_color", "weighted RGB value of light color"));
                }
                
            } else {
                stream << " color(" << entity.color[0] << ' ' << entity.color[1] << ' ' << entity.color[2] << ") ";
                
                if (options.bob) {
                    bool addBob = options.useDefaultBobPatterns && needsBob(entity.name);
                    
                    if (!addBob) {
                        for (size_t k=0; k<options.bobPatterns.size() && !addBob; ++k) {
                            addBob = globMatch(entity.name.c_str(), options.bobPatterns[k]);
                        }
                    }
                    
                    if (addBob) {
                        stream << "bobparms( " << options.bobparms[0] << " " << options.bobparms[1] << " " << options.bobparms[2] << " ) ";
                    }
                }
                
                for (std::vector<Key>::const_iterator it = entity.keys.begin(); it != entity.keys.end(); ++it)
                {
                    if (it->name == "angle" || it->name == "angles") {
                        stream << "flags(Angle) ";
                        break;
                    }
                }
                
                if (!entity.model.empty()) {
                    stream << "studio(\"" << entity.model << "\") ";
                } else {
                    for (size_t j=0; j<entity.keys.size(); ++j) {
                        if (entity.keys[j].name == "model" || entity.keys[j].name == "model2") {
                            stream << "studio() ";
                            break;
                        }
                    }
                }
            }
        }
        
        stream << "= " << entity.name;
        if (!entity.description.empty()) {
            const size_t tokenLimit = 2047;
            
            if (entity.description.size() > tokenLimit) {
                stream << " : \"" << entity.description.substr(0, tokenLimit) << "\"";
                
                size_t current = tokenLimit;
                while(current <= entity.description.size()) {
                    stream << " + \"" << entity.description.substr(current, tokenLimit) << "\"";
                    current += tokenLimit;
                }
                
            } else {
                stream << " : \"" << entity.description << "\"";
            }
        }
        stream << "\n";
        stream << "[\n";
        for (size_t j=0; j<entity.keys.size(); ++j)
        {
            const Key& key = entity.keys[j];
            stream << "\t" << key.name;
            if (key.name == "target") {
                stream << "(target_destination) : Target : : ";
            }
            else if (key.name == "targetname") {
                stream << "(target_source) : Name : : ";
            }
            else if (key.name == "_color") {
                stream << "(color1) : \"RGB color\" : \"1 1 0.5\" : ";
            } else if (key.name == "model" || key.name == "model2") {
                stream << "(studio) : Model : : ";
            } else {
                std::string name = key.name;
                if (!name.empty())
                    name[0] = toupper(name[0]);
                if (key.type.empty())
                    stream << "(string)";
                else
                    stream << "(" << key.type << ")";
                stream << " : " << name << " : : ";
            }
            
            stream << "\"" << key.description << "\"";
            stream << "\n";
        }
        
        bool hasspawnflags = false;
        for (size_t j=0; j<Entity::SpawnFlagNum && !hasspawnflags; ++j)
        {
            if (!entity.spawnflags[j].empty()) {
                hasspawnflags = true;
            }
        }
        
        if (hasspawnflags)
        {
            stream << "\tspawnflags(flags) = \n";
            stream << "\t[\n";
            for (size_t j=0; j<Entity::SpawnFlagNum; ++j)
            {
                std::string flagname = entity.spawnflags[j];
                for (size_t k=0; k<flagname.size(); ++k)
                {
                    flagname[k] = tolower(flagname[k]);
                }
                
                if (!flagname.empty()) {
                    stream << "\t\t" << (1 << j) << " : \"" << flagname << "\" : 0";
                    if (!entity.flagsdescriptions[j].empty()) {
                        stream << " : \"" << entity.flagsdescriptions[j] << "\"";
                    }
                    stream << '\n';
                }
            }
            stream << "\t]\n";
        }
        
        stream << "]\n";
    }
}

void printHelp(const char* programName)
{
    printf( translate("Usage: %s [options] [input-file] [output-file]\n"
            "\n"
            "  -format format       specify format of input: def or ent\n"
            "\n"
            "  -offset-glob pattern add offset to entities matching given pattern;\n"
            "                       this option can be passed multiple times\n"
            "  -noauto-offset-glob  don't use default patterns when setting offset\n"
            "\n"
            "  -bob                 same as -bobparms \"180 8 0\"\n"
            "  -bobparms \"x y z\"    set bob parameters\n"
            "  -bob-glob pattern    add bobparms to entities matching given pattern;\n"
            "                       this option can be passed multiple times\n"
            "  -noauto-bob-glob     don't use default patterns when setting bobparms\n"
            "\n"
            "  -help                display this help and exit\n"
            "  -version             show version information and exit\n"
            "  -- [arguments...]    treat the rest of arguments as positional arguments\n"
            "\n"
            "When input-file is omitted or -, read standard input.\n"
            "When output-file is omitted or -, write to standard output.\n").c_str()
    , programName);
}

void printVersion()
{
    printf("def2fgd %s\n", DEF2FGD_VERSION);
}

void printHint(const char* program)
{
    fprintf(stderr, translate("Use %s -help to get help.\n").c_str(), program);
}

void printBobparms(const FGDWriteOptions& options)
{
    fprintf(stderr, translate("Bob parameters already set to ( %d %d %d ). Remove redundant argument\n").c_str(), 
            options.bobparms[0], options.bobparms[1], options.bobparms[2]);
}

#ifndef LOCALEDIR
#define LOCALEDIR "locale"
#endif

int main(int argc, char** argv)
{
    const char* format = "";
    const char* inputFileName = 0;
    const char* outputFileName = 0;
    FGDWriteOptions options;
    
    const char* package = "def2fgd";
    const char* localeDir = LOCALEDIR;
    
    generateLocale(package, localeDir);
    
    std::vector<const char*> positionalArgs;
    const char* programName = argv[0];
    for(int i=1; i<argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-format") == 0) {
            if (format[0]) {
                fputs(translate("-format option repetition\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
            
            i++;
            if (i < argc) {
                format = argv[i];
            } else {
                fputs(translate("-format requires argument\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
        } else if (strcmp(arg, "-bob") == 0) {
            if (options.bob) {
                printBobparms(options);
                return EXIT_FAILURE;
            }
            
            options.bobparms[0] = 180;
            options.bobparms[1] = 8;
            options.bobparms[2] = 0;
            options.bob = true;
        } else if (strcmp(arg, "-bobparms") == 0) {
            if (options.bob) {
                printBobparms(options);
                return EXIT_FAILURE;
            }
            
            i++;
            if (i < argc) {
                char* current = argv[i];
                options.bobparms[0] = static_cast<int>(strtol(current, &current, 10));
                options.bobparms[1] = static_cast<int>(strtol(current, &current, 10));
                options.bobparms[2] = static_cast<int>(strtol(current, NULL, 10));
                options.bob = true;
            } else {
                fputs(translate("-bobparms requires argument\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
        } else if (strcmp(arg, "-offset-glob") == 0) {
            i++;
            if (i < argc) {
                options.offsetPatterns.push_back(argv[i]);
            } else {
                fputs(translate("-offset-glob requires argument\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
        } else if (strcmp(arg, "-bob-glob") == 0) {
            i++;
            if (i < argc) {
                options.bobPatterns.push_back(argv[i]);
            } else {
                fputs(translate("-bob-glob requires argument\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
            
        } else if (strcmp(arg, "-noauto-bob-glob") == 0) {
            options.useDefaultBobPatterns = false;
        } else if (strcmp(arg, "-noauto-offset-glob") == 0) {
            options.useDefaultOffsetPatterns = false;
        } else if (strcmp(arg, "-help") == 0) {
            printHelp(programName);
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-version") == 0) {
            printVersion();
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "--") == 0) {
            for (; ++i<argc;) {
                positionalArgs.push_back(argv[i]);
            }
        } else if (arg[0] == '-' && arg[1] != '\0') {
            fprintf(stderr, translate("Unknown parameter: %s\n").c_str(), arg);
            printHint(argv[0]);
            return EXIT_FAILURE;
        } else {
            positionalArgs.push_back(arg);
        }
    }
    
    if (positionalArgs.size() > 0) {
        inputFileName = positionalArgs[0];
        if (positionalArgs.size() > 1) {
            outputFileName = positionalArgs[1];
        }
    }
    
    if (inputFileName && strcmp(inputFileName, "-") == 0) {
        inputFileName = 0;
    }
    
    if (outputFileName && strcmp(outputFileName, "-") == 0) {
        outputFileName = 0;
    }
    
    if (format[0] == '\0') {
        if (inputFileName) {
            const char* extension = strrchr(inputFileName, '.');
            if (extension && strcmp(extension, ".ent") == 0){
                format = "ent";
            } else if (extension && strcmp(extension, ".def") == 0) {
                format = "def";
            } else {
                fputs(translate("Could not detect input format. Use -format option to explicitly set it.\n").c_str(), stderr);
                return EXIT_FAILURE;
            }
        } else {
            fputs(translate("No input file name nor format given.\n").c_str(), stderr);
            printHint(argv[0]);
            return EXIT_FAILURE;
        }
    }
    
    std::istream* inStream = &std::cin;
    std::ostream* outStream = &std::cout;
    
    std::ifstream inFile;
    std::ofstream outFile;
    
    if (inputFileName != 0) {
        inFile.open(inputFileName);
        if (!inFile) {
            fprintf(stderr, translate("Could not open file %s for reading: %s\n").c_str(), inputFileName, strerror(errno));
            return EXIT_FAILURE;
        }
        inStream = &inFile;
    }
    
    try
    {
        std::vector<Entity> entities;
        if (strcmp(format, "def") == 0) {
            entities = readDefFile(*inStream);
        } else if (strcmp(format, "ent") == 0) {
            entities = readEntFile(*inStream);
        } else {
            fprintf(stderr, translate("Unknown format '%s'\n").c_str(), format);
            return EXIT_FAILURE;
        }
        
        if (outputFileName) {
            outFile.open(outputFileName);
            if (!outFile) {
                fprintf(stderr, translate("Could not open file %s for writing: %s\n").c_str(), outputFileName, strerror(errno));
                return EXIT_FAILURE;
            }
            outStream = &outFile;
        }
        
        writefgd(*outStream, entities, options);
        outStream->flush();
    }
    catch(DefReadError& e)
    {
        if (inputFileName) {
            fprintf(stderr, translate("%s:%d:%d: error: %s\n").c_str(), inputFileName, e.line(), e.column(), e.what());
        } else {
            fprintf(stderr, translate("%d:%d: error: %s\n").c_str(), e.line(), e.column(), e.what());
        }
        
        return EXIT_FAILURE;
    }
    catch(std::exception& e)
    {
        fprintf(stderr, translate("Error: %s\n").c_str(), e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
