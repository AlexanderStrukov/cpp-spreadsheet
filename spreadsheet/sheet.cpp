#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Cell is out of range");
    }
    
    const auto& cell = cells_.find(pos);
    if (cell == cells_.end()) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    
    cells_.at(pos)->Set(std::move(text));
}

const Cell* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet&>(*this).GetCell(pos);
}

Cell* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("cell is out of range");
    }

    const auto iter = cells_.find(pos);
    if (iter == cells_.end()) {
        return nullptr;
    }

    return cells_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Cell is out of range");
    }

    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr) {
        cell->second->Clear();
        if (!cell->second->IsReferenced()) {
            cell->second.reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size result{ 0, 0 };
    
    for (auto it = cells_.begin(); it != cells_.end(); ++it) {
        if (it->second != nullptr) {
            result.rows = std::max(result.rows, it->first.row + 1);
            result.cols = std::max(result.cols, it->first.col + 1);
        }
    }

    return { result.rows, result.cols };
}

void Sheet::PrintValues(std::ostream& output) const {
    
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {

        for (int col = 0; col < size.cols; ++col) {

            if (col > 0) output << "\t";

            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                std::visit([&](const auto value) { output << value; }, it->second->GetValue());
            }
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {

        for (int col = 0; col < size.cols; ++col) {

            if (col > 0) output << "\t";

            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                output << it->second->GetText();
            }
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}