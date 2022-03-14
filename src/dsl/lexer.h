#pragma once

#include <string>
#include <vector>
#include <array>
#include <optional>
#include <iostream>

namespace Lexer
{

enum class TokenType : int
{
    UNKNOWN = 0,
    IDENT,
    INT,  
    EQUAL,
    SEMICOL,
    HASH,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,    
    COMMA,
    PERIOD,
    KEYWORD = 100,
    DEFINE  = 100,
    PIN,
    OUTPUT,
    INPUT,
    BIDIR,
    PROC,
    ENDPROC,
    REP,
    ENDREP,
    WAIT,
    SHR,
    RET,
    VAR
};

constexpr static std::array<const char*, 13> g_keywords =
{
    "DEFINE",
    "PIN",
    "OUTPUT",
    "INPUT",
    "BIDIR",
    "PROC",
    "ENDPROC",
    "REP",
    "ENDREP",
    "WAIT",
    "SHR",
    "RET",
    "VAR"
};

};


std::ostream& operator<<(std::ostream &os, Lexer::TokenType const &toktype);


namespace Lexer
{
struct SourcePos
{
    size_t m_line;
    size_t m_col;
};

struct Token
{
    TokenType   m_type = TokenType::UNKNOWN;
    std::string m_str;
    SourcePos   m_pos = {0,0};

    friend std::ostream& operator<<(std::ostream &os, Token const &tok)
    {
        os << tok.m_pos.m_line << "," << tok.m_pos.m_col << " " << tok.m_type;
        if (!tok.m_str.empty())
        {
            os << " ('" << tok.m_str << "')";
        }
        return os;
    }
};


class Lex
{
public:
    Lex(std::istream &is);

    std::vector<Token> process();

protected:

    enum class LexState
    {
        IDLE = 0,
        IDENT,
        INTEGER,
        CHARLIT,
        CHARLIT2
    } m_state = LexState::IDLE;

    bool atEnd() const
    {
        return (!m_is.good()) || m_is.eof();
    }

    void advance() noexcept
    {
        m_pos.m_col++;
        m_c = m_is.get();
        if (m_c == 10)
        {
            m_pos.m_col = 1;
            m_pos.m_line++;
        }
    }

    char get() const noexcept
    {
        return m_c;
    }

    constexpr bool isWhitespace(char c) const
    {
        return (c == ' ') || (c == '\t');
    }

    constexpr bool isEOL(char c) const
    {
        return (c == 10) || (c == 13);
    }

    constexpr bool isDigit(char c) const
    {
        return ((c >= '0') && (c <= '9'));
    }

    constexpr bool isAlpha(char c) const
    {
        return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '_');
    }

    constexpr bool isAlphaNum(char c) const
    {
        return isDigit(c) || isAlpha(c);
    }

    void emit()
    {
        m_curTok.m_pos  = m_pos;
        m_tokens.push_back(m_curTok);
        m_curTok.m_str.clear();
        m_curTok.m_type = TokenType::UNKNOWN;
    }

    void emit(TokenType tokType)
    {
        m_curTok.m_type = tokType;
        emit();
    }

    std::optional<size_t> getKeywordIndex(const std::string_view str);

    SourcePos           m_pos = {0,1};
    std::istream        &m_is;

    char                m_c;
    char                m_charLiteral;
    Token               m_curTok;
    std::vector<Token>  m_tokens;
};

};

