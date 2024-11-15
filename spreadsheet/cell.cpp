#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <stack>

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw std::logic_error("cell is empty");
    }

    if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
    }
    else {
        return text_;
    }
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) cache_ = formula_->Evaluate(sheet_);
    
    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    }
    else {
        return std::get<FormulaError>(cache_.value());
    }
}

bool Cell::CheckCircularDependency(const Impl& new_impl) const {
    if (new_impl.GetReferencedCells().empty()) {
        return false;
    }

    std::unordered_set<const Cell*> referenced_cells;
    for (const auto& pos : new_impl.GetReferencedCells()) {
        referenced_cells.insert(sheet_.GetCell(pos));
    }

    std::unordered_set<const Cell*> checked_cells;
    std::stack<const Cell*> cells_to_check;
    cells_to_check.push(this);
    while (!cells_to_check.empty()) {
        const Cell* current = cells_to_check.top();
        cells_to_check.pop();
        checked_cells.insert(current);

        if (referenced_cells.find(current) != referenced_cells.end()) {
            return true;
        }

        for (const Cell* cell_ptr : current->depends_cells_) {
            if (checked_cells.find(cell_ptr) == checked_cells.end()) 
                cells_to_check.push(cell_ptr);
        }
    }

    return false;
}

void Cell::InvalidateCache(bool force) {
    if (impl_->IsCacheValid() || force) {
        impl_->InvalidateCache();
        for (Cell* cell_ptr : depends_cells_) {
            cell_ptr->InvalidateCache();
        }
    }
}

void Cell::Set(std::string text) {
    
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }
    
    std::unique_ptr<Impl> impl;
    if (text.size() != 0 && text[0] == FORMULA_SIGN) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else {
        impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (CheckCircularDependency(*impl)) {
        throw CircularDependencyException("");
    }
    
    impl_ = std::move(impl);

    for (Cell* outgoing : refer_cells_) {
        outgoing->depends_cells_.erase(this);
    }
    refer_cells_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* outgoing = sheet_.GetCell(pos);
        if (!outgoing) {
            sheet_.SetCell(pos, "");
            outgoing = sheet_.GetCell(pos);
        }
        refer_cells_.insert(outgoing);
        outgoing->depends_cells_.insert(this);
    }

    InvalidateCache(true);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}