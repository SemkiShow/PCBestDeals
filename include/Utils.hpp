// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <vector>

std::vector<std::string> Split(const std::string& input, const char delimiter = ',');
std::string TrimJunk(const std::string& input);
int DownloadFile(const std::string& url, const std::string& outputPath);
