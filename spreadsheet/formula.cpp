#include "formula.h"

#include "FormulaAST.h"
#include "sheet.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe)
{
    return output << "#ARITHM!";
}

namespace
{
    class Formula : public FormulaInterface
    {
    public:
        // Реализуйте следующие методы:
        explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
        Value Evaluate(const SheetInterface &sheet) const override
        {
            try
            {
                return ast_.Execute(sheet);
            }
            catch (const FormulaError &exc)
            {
                return exc;
            }
        }
        std::string GetExpression() const override
        {
            std::stringstream ss;
            ast_.PrintFormula(ss);
            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const
        {
            std::vector<Position> result;
            for (auto &memb : ast_.GetCells())
            {
                result.push_back(memb);
            }
            // удаление повторяющихся
            result.erase(std::unique(result.begin(), result.end()), result.end());

            return result;
        }

    private:
        FormulaAST ast_;
    };
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    try
    {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (const std::exception &exc)
    {
        throw FormulaException(exc.what());
    }
}