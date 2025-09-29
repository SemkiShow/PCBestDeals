// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

extern std::string lastCAUpdate;
extern bool benchmarksAvailable;
extern bool pricesAvailable;

void Save();
void Load();
