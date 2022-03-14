#pragma once
#include "dsltypes.h"
#include "lexer.h"
#include "symtbl.h"
#include "ast.h"

namespace Parser
{

class Parse
{
public:
    Parse(const std::vector<Lexer::Token> &tokens) : m_tokens(tokens) {}

    std::unique_ptr<ASTNode> process();

protected:
    
    bool parsePinDefines(std::unique_ptr<ASTNode> &head);
    bool parseProcedures(std::unique_ptr<ASTNode> &head);
    bool parseStatements(std::unique_ptr<ASTNode> &head);
    bool parseProcVarList(std::unique_ptr<ASTNode> &head);
    bool parseExpression(std::unique_ptr<ASTNode> &head);

    auto const& get() const
    {
        if (atEnd())
        {
            return m_eofToken;
        }
        return m_tokens.at(m_idx);
    }

    constexpr void advance()
    {
        m_idx++;
    }

    bool atEnd() const
    {
        return (m_idx >= m_tokens.size());
    }

    bool match(Lexer::TokenType tok)
    {
        if (get().m_type == tok)
        {
            m_matchString = get().m_str;
            advance();
            return true;
        }
        return false;
    }

    void error(const std::string &str)
    {
        auto location = get().m_pos;        
        std::cerr << "Error on line: " << location.m_line << " " << str;
    }

    size_t m_idx = 0;
    std::string m_matchString;
    const std::vector<Lexer::Token> &m_tokens;
    Lexer::Token m_eofToken;
};

};