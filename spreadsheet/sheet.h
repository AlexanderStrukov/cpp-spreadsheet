#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class CellHasher {
public:
    size_t operator()(const Position p) const {
        return std::hash<std::string>()(p.ToString());
    }
};

class Sheet : public SheetInterface {
public:

    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const Cell* GetCell(Position pos) const override;
    Cell* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
	std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher> cells_;
};