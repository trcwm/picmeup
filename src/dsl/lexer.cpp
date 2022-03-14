#include <algorithm>
#include <sstream>
#include "lexer.h"

std::ostream& operator<<(std::ostream &os, Lexer::TokenType const &toktype)
{
    switch(toktype)
    {
    case Lexer::TokenType::UNKNOWN:
        return os << "UNKNOWN";
    case Lexer::TokenType::IDENT:
        return os << "IDENT";
    case Lexer::TokenType::INT:
        return os << "INTEGER";
    case Lexer::TokenType::EQUAL:
        return os << "=";
    case Lexer::TokenType::SEMICOL:
        return os << ";";
    case Lexer::TokenType::COMMA:
        return os << ",";
    case Lexer::TokenType::PERIOD:
        return os << ".";                
    case Lexer::TokenType::HASH:
        return os << "#";
    case Lexer::TokenType::LPAREN:
        return os << "(";
    case Lexer::TokenType::RPAREN:
        return os << ")";
    case Lexer::TokenType::LBRACKET:
        return os << "[";
    case Lexer::TokenType::RBRACKET:
        return os << "]";
    default:
        if (static_cast<int>(toktype) >= static_cast<int>(Lexer::TokenType::KEYWORD))
        {
            return os << "KEYWORD";
        }
    }
    return os;
}

Lexer::Lex::Lex(std::istream &is) : m_is(is)
{
}

std::vector<Lexer::Token> Lexer::Lex::process()
{
    advance();

    while(!atEnd())
    {
        char c = get();

        switch(m_state)
        {
        case LexState::IDLE:
            if (isWhitespace(c))
            {
                advance();
            }
            else if (isEOL(c))
            {
                advance();
            }
            else if (isAlpha(c))
            {
                m_state = LexState::IDENT;
                m_curTok.m_str += c;
                advance();
            }
            else if (isDigit(c))
            {
                m_state = LexState::INTEGER;
                m_curTok.m_str += c;
                advance();                
            }
            else if (c == '\'')
            {
                m_state = LexState::CHARLIT;
                advance();
            }
            else
            {
                // one-char tokens
                switch(c)
                {
                case '=':
                    m_curTok.m_type = TokenType::EQUAL;
                    emit();
                    advance();
                    break;
                case ';':
                    m_curTok.m_type = TokenType::SEMICOL;
                    emit();
                    advance();
                    break;
                case ',':
                    m_curTok.m_type = TokenType::COMMA;
                    emit();
                    advance();
                    break;
                case '.':
                    m_curTok.m_type = TokenType::PERIOD;
                    emit();
                    advance();
                    break;                                        
                case '(':
                    m_curTok.m_type = TokenType::LPAREN;
                    emit();
                    advance();
                    break;
                case ')':
                    m_curTok.m_type = TokenType::RPAREN;
                    emit();
                    advance();
                    break;
                case '[':
                    m_curTok.m_type = TokenType::LBRACKET;
                    emit();
                    advance();
                    break;
                case ']':
                    m_curTok.m_type = TokenType::RBRACKET;
                    emit();
                    advance();
                    break;                                                            
                case '#':   // line comment
                    while(!isEOL(c))
                    {
                        advance();
                        c = get();
                    }
                    break;
                default:
                    advance();  // char not used : error
                    break;
                }
            }
            break;
        case LexState::INTEGER:
            if (isDigit(c))
            {
                m_curTok.m_str += c;
                advance();
            }
            else
            {
                emit(TokenType::INT);
                m_state = LexState::IDLE;
            }
            break;
        case LexState::CHARLIT:
            m_state = LexState::CHARLIT2;
            m_charLiteral = c;
            advance();
            break;
        case LexState::CHARLIT2:
            if (c == '\'')
            {
                // expected closing single quote
                std::stringstream ss;
                ss << static_cast<int>(m_charLiteral);
                m_curTok.m_str = ss.str();
                emit(TokenType::INT);
            }
            advance();
            m_state = LexState::IDLE;
            break;            
        case LexState::IDENT:
            if (isAlphaNum(c))
            {
                m_curTok.m_str += c;
                advance();
            }
            else
            {
                // check for keyword
                auto kwIndex = getKeywordIndex(m_curTok.m_str);
                if (!kwIndex)
                {
                    emit(TokenType::IDENT);
                }
                else
                {
                    auto id = static_cast<TokenType>(static_cast<int>(TokenType::KEYWORD) + kwIndex.value());
                    emit(id);
                }
                m_state = LexState::IDLE;
            }            
            break;
        }
    }

    return m_tokens;
}

std::optional<size_t> Lexer::Lex::getKeywordIndex(const std::string_view str)
{
    size_t index = 0;
    for(auto kwstr : g_keywords)
    {
        if (str.compare(kwstr) == 0)
        {
            return index;
        }
        index++;
    }
    return std::nullopt;  /* not found */
}
