#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>
#include <set>
class Sheet;

enum CONTENT
{
    EMPTY,
    TEXT,
    FORMULA,
};

class Cell : public CellInterface
{
public:
    ~Cell() = default;
    Cell(SheetInterface &sheet)
        : sheet_(sheet)
    {
    }

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void ClearCache();
    bool HasCellCircularReferences(std::set<Cell *> &visited, std::set<Cell *> &to_visit, Cell *cell);
    void ClearChildrenCash(std::set<Cell *> &visited, Cell *child);
    void AddChildToParents();

private:
    CONTENT type_of_content_;
    std::string raw_content_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<double> cache_;
    SheetInterface &sheet_;
    std::vector<Cell *> children_;
};