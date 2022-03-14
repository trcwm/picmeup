#pragma once
#include <vector>
#include <memory>
#include "ast.h"

namespace ASTManip
{

struct PinDirectionCreatorVisitor : public AbstractVisitor
{
    PinDirectionCreatorVisitor() {}

    void visit(ASTNode *node) override;

};

}; // namespace