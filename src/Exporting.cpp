#include "Exporting.hpp"
#include <xlsxwriter.h>

void ExportAsXlsx(const std::vector<BenchmarkEntry>& benchmarks,
                  const std::vector<DealEntry>& prices)
{
    if (benchmarks.size() == 0) return;
    lxw_workbook* workbook = workbook_new("output.xlsx");

    DeviceType lastType =
        (benchmarks[0].type == DeviceType::CPU ? DeviceType::GPU : DeviceType::CPU);
    lxw_worksheet* worksheet = nullptr;

    lxw_format* defaultFormat = workbook_add_format(workbook);
    format_set_font_size(defaultFormat, 18);

    int index = 1;
    for (size_t i = 0; i < std::min(benchmarks.size(), prices.size()); i++)
    {
        if (benchmarks[i].type != lastType)
        {
            worksheet = workbook_add_worksheet(
                workbook, (benchmarks[i].type == DeviceType::CPU ? "CPU" : "GPU"));
            worksheet_write_string(worksheet, 0, 0, "Name", defaultFormat);
            worksheet_write_string(worksheet, 0, 1, "Score", defaultFormat);
            worksheet_write_string(worksheet, 0, 2, "Price", defaultFormat);
            worksheet_write_string(worksheet, 0, 3, "Value", defaultFormat);
            worksheet_set_column(worksheet, 0, 0, 60, defaultFormat);
            worksheet_set_column(worksheet, 1, 3, 20, defaultFormat);
            index = 1;
        }
        worksheet_write_string(worksheet, index, 0, benchmarks[i].name.c_str(), defaultFormat);
        worksheet_write_number(worksheet, index, 1, benchmarks[i].score, defaultFormat);
        worksheet_write_number(worksheet, index, 2, prices[i].price, defaultFormat);
        std::string formula = "=B" + std::to_string(index + 1) + "/C" + std::to_string(index + 1);
        double formulaValue = benchmarks[i].score / prices[i].price;
        worksheet_write_formula_num(worksheet, index, 3, formula.c_str(), defaultFormat,
                                    formulaValue);
        lastType = benchmarks[i].type;
        index++;
    }

    workbook_close(workbook);
}
