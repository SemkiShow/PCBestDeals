#pragma once

#include <string>

extern std::string lastCAUpdate;
extern bool benchmarksAvailable;
extern bool pricesAvailable;

void Save();
void Load();
