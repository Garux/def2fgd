
#pragma once

#include <fstream>
#include <vector>
#include "entity.h"

void writeEnt(std::ostream& stream, const std::vector<Entity>& entities, const char *appPath);
