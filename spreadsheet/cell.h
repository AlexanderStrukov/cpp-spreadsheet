#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet)
        : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}
    
    ~Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override {
        return impl_->GetValue();
    }
    
    std::string GetText() const override {
        return impl_->GetText();
    }
    
    std::vector<Position> GetReferencedCells() const override {
        return impl_->GetReferencedCells();
    }

    bool IsReferenced() const {
        return !depends_cells_.empty();
    }
    
private:
    class Impl {
    public:
        virtual ~Impl() = default;
        
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        
        virtual std::vector<Position> GetReferencedCells() const {
            return {};
        }
        
        virtual bool IsCacheValid() const {
            return true;
        }
        
        virtual void InvalidateCache() {}
    };
    
    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override { return ""; }
        std::string GetText() const override { return ""; }
    };
    
    class TextImpl : public Impl {
    public:
        TextImpl(std::string text) 
            : text_(std::move(text)) {}
    
        Value GetValue() const override;
    
        std::string GetText() const override {
            return text_;
        }
    
    private:
        std::string text_;
    };
    
    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string expression, const SheetInterface& sheet)
            : formula_(ParseFormula(expression.substr(1))), sheet_(sheet) {}
    
        Value GetValue() const override;
    
        std::string GetText() const override {
            return FORMULA_SIGN + formula_->GetExpression();
        }
    
        bool IsCacheValid() const override {
            return cache_.has_value();
        }
    
        void InvalidateCache() override {
            cache_.reset();
        }
    
        std::vector<Position> GetReferencedCells() const {
            return formula_->GetReferencedCells();
        }
    
    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
    
    bool CheckCircularDependency(const Impl& new_impl) const;
    
    void InvalidateCache(bool force = false);

    std::unique_ptr<Impl> impl_;

    Sheet& sheet_;
    std::unordered_set<Cell*> depends_cells_;
    std::unordered_set<Cell*> refer_cells_;
};