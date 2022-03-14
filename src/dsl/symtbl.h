#pragma once
#include <algorithm>
#include <deque>
#include "dsltypes.h"

namespace SymTbl
{

    struct Symbol
    {
        enum class Type
        {
            UNKNOWN = 0,
            PIN,        /* global scope only */
            VARIABLE,   /* local variable only */
            CONSTANT,
            PROC
        };

        Type        m_type      = Type::UNKNOWN;
        int         m_integer   = -1;   // constant, pin number
        size_t      m_address   = 0;    // offset from base pointer
        std::string m_name;             // var name, proc name
        IOType      m_iotype    = IOType::UNKNOWN;
        int         m_scopeLevel = 0;
    };

    struct Scope
    {
        int     m_scopeLevel    = 0;
        size_t  m_address       = 0;
        std::deque<Symbol>  m_syms;

        std::optional<Symbol> find(const std::string &name) const
        {
            auto iter = std::find_if(m_syms.begin(), m_syms.end(),
                [&name](auto const& s)
                {
                    return s.m_name == name;
                }
            );

            if (iter == m_syms.end())
            {
                return std::nullopt;
            }

            return *iter;
        }
    };

    class Table
    {
    public:
        Table()
        {
            // create a global scope
            m_scopes.emplace_front();
            m_scopes.front().m_scopeLevel = 0;
        }

        Symbol& create(const std::string &name, Symbol::Type t)
        {
            auto &currentScope = m_scopes.front();
            currentScope.m_syms.emplace_front();
            currentScope.m_syms.front().m_name = name;
            currentScope.m_syms.front().m_type = t;
            currentScope.m_syms.front().m_address = currentScope.m_address;
            currentScope.m_syms.front().m_scopeLevel = currentScope.m_scopeLevel;

            currentScope.m_address++;

            return currentScope.m_syms.front();
        }

        std::optional<Symbol> find(const std::string &name) const
        {
            for(auto const& scope : m_scopes)
            {
                auto opt = scope.find(name);
                if (opt)
                {
                    return opt;
                }
            }
            return std::nullopt;
        }

        void enterScope()
        {
            auto const& prevScope = m_scopes.front();
            m_scopes.emplace_front();
            m_scopes.front().m_scopeLevel = prevScope.m_scopeLevel+1;
        }

        void leaveScope()
        {
            m_scopes.pop_front();
        }

    protected:
        std::deque<Scope>   m_scopes;
    };
};
