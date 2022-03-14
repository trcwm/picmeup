#include "astmanip.h"

void ASTManip::PinDirectionCreatorVisitor::visit(ASTNode *node)
{
    if (node->m_type == ASTNode::NodeType::ASSIGN)
    {
        if ((node->m_children.size() == 1) && (node->m_children.at(0)->m_type == ASTNode::NodeType::IODIR))
        {
            node->m_type = ASTNode::NodeType::SETPINDIR;
            node->m_iotype = node->m_children.at(0)->m_iotype;
            node->m_children.clear();
        }
    }

    for(auto const& child : node->m_children)
    {
        visit(child.get());
    }
}
