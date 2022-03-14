#pragma once

#include <stack>
#include <iomanip>
#include "ast.h"
#include "symtbl.h"

namespace CodeGen
{

struct CodeGenVisitor : public AbstractConstVisitor
{
    CodeGenVisitor(std::ostream &os) : m_os(os) {}

    void visit(const ASTNode *node) override;

    void emitLiteral(uint16_t value)
    {
        
    }

    void emit(const std::string &txt)
    {
        std::ios_base::fmtflags f(m_os.flags());
        m_os << std::setfill('0') << std::setw(4) << m_address;
        m_os.flags(f);
        
        m_os << "  " << txt;
        m_address++;
    }

    void emit(const std::string &txt, int16_t v)
    {
        std::ios_base::fmtflags f(m_os.flags());
        m_os << std::setfill('0') << std::setw(4) << m_address;
        m_os.flags(f);
        
        m_os << "  " << txt << " " << v << ",";
        m_address++;
    }

    void emit(int16_t v)
    {
        m_os << v << "\n";
        m_address++;
    }

    void emitLabel(int16_t v)
    {
        m_os << ":loc_" << v << "\n";
    }

    void comment(const std::string &txt)
    {
        m_os << txt;
    }

    void comment(int v)
    {
        m_os << v;
    }

    SymTbl::Table   m_syms;
    int             m_address = 0;
    int             m_labelCount = 0;
    std::stack<int> m_labels;
    std::ostream    &m_os;
};

};
