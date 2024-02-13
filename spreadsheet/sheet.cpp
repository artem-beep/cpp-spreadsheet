#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <algorithm>

using namespace std::literals;

Sheet::~Sheet() {}

inline std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value)
{
    std::visit(
        [&](const auto &x)
        {
            output << x;
        },
        value);
    return output;
}

void Sheet::SetCell(Position pos, std::string text)
{

    if (!pos.IsValid())
    {
        throw InvalidPositionException("INVALID POSITION");
    }

    // Смена кол-ва строк
    // Обновление индекса максимальной строки
    if (pos.row > last_raw_ || all_cells_.empty())
    {
        all_cells_.resize(pos.row + 1);
        last_raw_ = pos.row;
        for (size_t i = static_cast<int>(last_raw_); i != all_cells_.size(); ++i)
        {
            all_cells_[i].resize(all_cells_.begin()->size() == 0 ? 1 : all_cells_.begin()->size());
        }
    }

    // Смена кол-ва столбцов
    // Обновление индекса максимального столбца
    if (pos.col > last_cal_)
    {
        last_cal_ = pos.col;
        for (size_t i = 0; i != all_cells_.size(); ++i)
        {
            all_cells_[i].resize(pos.col + 1);
        }
    }

    Cell *raw_cell_ptr = new Cell(*this);

    raw_cell_ptr->Set(text);

    std::unique_ptr<Cell> cell_ptr{raw_cell_ptr};

    auto prev_value = std::move(all_cells_[pos.row][pos.col]);
    all_cells_[pos.row][pos.col] = std::move(cell_ptr);

    std::set<Cell *> visited;
    std::set<Cell *> to_visit;

    if (all_cells_[pos.row][pos.col]->HasCellCircularReferences(visited, to_visit, all_cells_[pos.row][pos.col].get()))
    {
        all_cells_[pos.row][pos.col] = std::move(prev_value);
        throw CircularDependencyException("");
    }
}

const CellInterface *Sheet::GetCell(Position pos) const
{

    if (!pos.IsValid())
    {
        throw InvalidPositionException("INVALID POSITION");
    }

    if ((pos.row + 1 > static_cast<int>(all_cells_.size())) || (pos.col + 1 > static_cast<int>(all_cells_[pos.row].size())))
    {
        return nullptr;
    }

    return all_cells_[pos.row][pos.col].get();
}
CellInterface *Sheet::GetCell(Position pos)
{

    if (!pos.IsValid())
    {
        throw InvalidPositionException("INVALID POSITION");
    }
    if ((pos.row + 1 > static_cast<int>(all_cells_.size())) || (pos.col + 1 > static_cast<int>(all_cells_[pos.row].size())))
    {
        return nullptr;
    }
    return all_cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("INVALID POSITION");
    }

    int col = 1, row = 1;
    if (pos.col <= last_cal_ && pos.row <= last_raw_)
    {
        last_cal_ = 0;
        last_raw_ = 0;

        // Случай когда ячейка уже существует
        if (all_cells_.size() > static_cast<size_t>(pos.row))
        {
            if (all_cells_[pos.row].size() > static_cast<size_t>(pos.col))
            {
                all_cells_.at(pos.row).at(pos.col).release();
            }
        }

        // Обновление наибольших индексов столбцов и строк
        for (auto &line : all_cells_)
        {
            col = 1;
            for (auto &cell : line)
            {
                if (cell != nullptr)
                {
                    if (col > last_cal_)
                    {
                        last_cal_ = col;
                    }
                    if (row > last_raw_)
                    {
                        last_raw_ = row;
                    }
                }
                col++;
            }

            row++;
        }

        // Обновение таблицы по новым максимальным значениям
        all_cells_.resize(last_raw_);
        for (auto &line : all_cells_)
        {
            line.resize(last_cal_);
        }
    }
}

Size Sheet::GetPrintableSize() const
{
    if (all_cells_.empty())
    {
        return {0, 0};
    }
    return {static_cast<int>(all_cells_.size()),
            static_cast<int>(all_cells_[0].size())};
}

void Sheet::PrintValues(std::ostream &output) const
{

    for (const auto &line : all_cells_)
    {
        bool first = true;
        for (const auto &cell : line)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                output << '\t';
            }
            if (cell != nullptr)
            {
                const auto &value = cell.get()->GetValue();
                output << value;
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream &output) const
{

    for (const auto &line : all_cells_)
    {
        bool first = true;
        for (const auto &cell : line)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                output << '\t';
            }
            if (cell != nullptr)
            {
                const auto &value = cell.get()->GetText();
                output << value;
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}