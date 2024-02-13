#include "cell.h"
#include "formula.h"
#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <set>
void Cell::Set(std::string text)
{

    auto &first_symbol = text[0];
    if ((first_symbol == '=') && (text.size() != 1))
    {
        type_of_content_ = CONTENT::FORMULA;
        try
        {
            raw_content_ = std::move(text.substr(1));
            formula_ = ParseFormula(raw_content_);
        }
        catch (const std::exception &exc)
        {
            throw FormulaException(exc.what());
        }
    }
    else if (text.empty())
    {
        type_of_content_ = CONTENT::EMPTY;
        raw_content_ = std::string{};
    }
    else
    {
        type_of_content_ = CONTENT::TEXT;

        raw_content_ = std::move(text);
    }

    std::set<Cell *> visited;
    ClearChildrenCash(visited, this);
    AddChildToParents();
}

void Cell::Clear()
{
    type_of_content_ = CONTENT::EMPTY;
    raw_content_ = std::string{};
}

Cell::Value Cell::GetValue() const
{
    if ((type_of_content_ == TEXT) || (type_of_content_ == EMPTY))
    {
        if (raw_content_[0] == '\'')
        {
            return raw_content_.substr(1);
        }
        else
        {
            return raw_content_;
        }
    }
    else /*if (type_of_content_ == FORMULA) */
    {
        auto result = formula_->Evaluate(sheet_);

        if (std::holds_alternative<double>(result))
        {
            cache_ = std::get<double>(result);
            return std::get<double>(result);
        }
        else
        {
            return std::get<FormulaError>(result);
        }
    }
}
std::string Cell::GetText() const
{
    if ((type_of_content_ == TEXT) || (type_of_content_ == EMPTY))
    {
        return raw_content_;
    }
    else
    {

        return '=' + formula_.get()->GetExpression();
    }
}

std::vector<Position> Cell::GetReferencedCells() const
{
    if ((type_of_content_ == TEXT) || (type_of_content_ == EMPTY))
    {
        std::vector<Position> empty_vect;
        return empty_vect;
    }
    return formula_.get()->GetReferencedCells();
}

void Cell::ClearCache()
{
    cache_.reset();
}

bool Cell::HasCellCircularReferences(std::set<Cell *> &visited, std::set<Cell *> &to_visit, Cell *cell)
{

    to_visit.insert(cell);

    if (!cell->GetReferencedCells().empty())
    {
        for (auto pos : cell->GetReferencedCells())
        {
            if (sheet_.GetCell(pos) == nullptr)
            {
                sheet_.SetCell(pos, "");
            }
            Cell *cell_ptr = static_cast<Cell *>(sheet_.GetCell(pos));
            if (to_visit.count(cell_ptr))
            {
                return true;
            }
            if (visited.count(cell_ptr))
            {

                continue;
            }
            else
            {
                return HasCellCircularReferences(visited, to_visit, cell_ptr);
            }
        }
    }
    visited.insert(cell);
    to_visit.erase(cell);
    return false;
}

void Cell::ClearChildrenCash(std::set<Cell *> &visited, Cell *child)
{
    child->ClearCache();
    visited.insert(child);

    for (auto child_ : children_)
    {
        if (visited.count(child_))
        {
            continue;
        }
        else
        {
            ClearChildrenCash(visited, child_);
        }
    }
}

void Cell::AddChildToParents()
{
    if (!GetReferencedCells().empty())
    {

        for (auto &parent : GetReferencedCells())
        {

            if (sheet_.GetCell(parent) == nullptr)
            {
                sheet_.SetCell(parent, "");
            }

            Cell *cell_ptr = static_cast<Cell *>(sheet_.GetCell(parent));
            cell_ptr->children_.push_back(this);
        }
    }
}