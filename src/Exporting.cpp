// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Exporting.hpp"
#include <xlsxwriter.h>

void AddNewWorksheet(lxw_workbook* workbook, lxw_worksheet*& worksheet, lxw_format* defaultFormat,
                     const char* name)
{
    worksheet = workbook_add_worksheet(workbook, name);
    worksheet_write_string(worksheet, 0, 0, "Name", defaultFormat);
    worksheet_write_string(worksheet, 0, 1, "Score", defaultFormat);
    worksheet_write_string(worksheet, 0, 2, "Price", defaultFormat);
    worksheet_write_string(worksheet, 0, 3, "Value", defaultFormat);
    worksheet_set_column(worksheet, 0, 0, 60, defaultFormat);
    worksheet_set_column(worksheet, 1, 3, 20, defaultFormat);
}

void AddResultEntry(lxw_worksheet* worksheet, lxw_format* defaultFormat,
                    const std::pair<BenchmarkEntry, DealEntry>& result, const int& index)
{
    worksheet_write_string(worksheet, index, 0, result.first.name.c_str(), defaultFormat);
    worksheet_write_number(worksheet, index, 1, result.first.score, defaultFormat);
    worksheet_write_number(worksheet, index, 2, result.second.price, defaultFormat);
    std::string formula = "=B" + std::to_string(index + 1) + "/C" + std::to_string(index + 1);
    double formulaValue = result.first.score / result.second.price;
    worksheet_write_formula_num(worksheet, index, 3, formula.c_str(), defaultFormat, formulaValue);
}

void ExportAsXlsx(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks,
                  const std::unordered_map<std::string, DealEntry>& prices)
{
    if (benchmarks.size() == 0) return;
    lxw_workbook* workbook = workbook_new("output.xlsx");

    std::vector<std::pair<BenchmarkEntry, DealEntry>> cpuResults, gpuResults;
    for (auto& benchmark: benchmarks)
    {
        if (prices.find(benchmark.first) == prices.end()) continue;
        if (benchmark.second.type == DeviceType::CPU)
        {
            cpuResults.push_back(std::make_pair(benchmark.second, prices.at(benchmark.first)));
        }
        else
        {
            gpuResults.push_back(std::make_pair(benchmark.second, prices.at(benchmark.first)));
        }
    }

    lxw_worksheet* worksheet = nullptr;
    int index = 1;

    lxw_format* defaultFormat = workbook_add_format(workbook);
    format_set_font_size(defaultFormat, 18);

    index = 1;
    AddNewWorksheet(workbook, worksheet, defaultFormat, "CPU");
    for (size_t i = 0; i < cpuResults.size(); i++)
    {
        AddResultEntry(worksheet, defaultFormat, cpuResults[i], index++);
    }
    index = 1;
    AddNewWorksheet(workbook, worksheet, defaultFormat, "GPU");
    for (size_t i = 0; i < gpuResults.size(); i++)
    {
        AddResultEntry(worksheet, defaultFormat, gpuResults[i], index++);
    }

    workbook_close(workbook);
}
