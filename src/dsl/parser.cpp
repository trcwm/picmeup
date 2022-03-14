#include "parser.h"

std::unique_ptr<ASTNode> Parser::Parse::process()
{
    auto astHead = std::make_unique<ASTNode>();

    if (!parsePinDefines(astHead))
    {
        return std::unique_ptr<ASTNode>();
    }

    if (!parseProcedures(astHead))
    {
        return astHead;
    }

    return astHead;
}

bool Parser::Parse::parsePinDefines(std::unique_ptr<ASTNode> &head)
{
    while(match(Lexer::TokenType::DEFINE))
    {
        auto pindefNode = std::make_unique<ASTNode>(ASTNode::NodeType::PINDEF);
        if(!match(Lexer::TokenType::IDENT))
        {
            error("Expected an indentifier following DEFINE\n");
            return false;
        }

        pindefNode->m_name = m_matchString; // pin name

        if (!match(Lexer::TokenType::EQUAL))
        {
            error("Expected '='\n");
            return false;            
        }

        if (!match(Lexer::TokenType::PIN))
        {
            error("Expected 'PIN'\n");
            return false;            
        }

        if (!match(Lexer::TokenType::INT))
        {
            error("Expected an integer\n");
            return false;            
        }     

        //FIXME: robustness
        pindefNode->m_integer = atoi(m_matchString.c_str());    // pin number

        if (match(Lexer::TokenType::OUTPUT))
        {
            pindefNode->m_iotype = IOType::OUTPUT;
        }
        else if (match(Lexer::TokenType::INPUT))
        {
            pindefNode->m_iotype = IOType::INPUT;
        }
        else if (match(Lexer::TokenType::BIDIR))
        {
            pindefNode->m_iotype = IOType::BIDIR;
        }
        else
        {
            error("Expected an 'INPUT', 'OUTPUT' or 'BIDIR'\n");
            return false;
        }

        head->m_children.emplace_back(std::move(pindefNode));
    }

    return true;
}

bool Parser::Parse::parseProcedures(std::unique_ptr<ASTNode> &head)
{
    while(match(Lexer::TokenType::PROC))
    {
        if (!match(Lexer::TokenType::IDENT))
        {
            error("Expected procedure identifier\n");
            return false;            
        }

        // create a new PROC AST node
        auto procName = m_matchString;
        auto procNode = std::make_unique<ASTNode>(ASTNode::NodeType::PROCDEF);
        procNode->m_name = procName;

        if (!match(Lexer::TokenType::LPAREN))
        {
            error("Expected '('\n");
            return false;            
        }

        if (!parseProcVarList(procNode))
        {
            return false;
        }

        if (!match(Lexer::TokenType::RPAREN))
        {
            error("Expected ')'\n");
            return false;            
        }

        if (!parseStatements(procNode))
        {
            return false;
        }        

        if (!match(Lexer::TokenType::ENDPROC))
        {
            error("Expected 'ENDPROC'\n");
            return false;            
        }

        head->m_children.emplace_back(std::move(procNode));
    }

    return true;
}

bool Parser::Parse::parseProcVarList(std::unique_ptr<ASTNode> &head)
{
    if (match(Lexer::TokenType::IDENT))
    {
        auto argNode = std::make_unique<ASTNode>(ASTNode::NodeType::ARG);
        argNode->m_name = m_matchString;
        head->m_children.emplace_back(std::move(argNode));

        while(match(Lexer::TokenType::COMMA))
        {
            if (!match(Lexer::TokenType::IDENT))
            {
                error("Expected variable name identifier\n");
                return false;            
            }
            auto argNode2 = std::make_unique<ASTNode>(ASTNode::NodeType::ARG);
            argNode2->m_name = m_matchString;
            head->m_children.emplace_back(std::move(argNode2));
        }
    }

    return true;
}

bool Parser::Parse::parseStatements(std::unique_ptr<ASTNode> &head)
{
    bool matched = true;
    while(matched)
    {
        matched = false;
        if (match(Lexer::TokenType::VAR))
        {
            auto vardefNode = std::make_unique<ASTNode>(ASTNode::NodeType::VARDEF);
            if (!match(Lexer::TokenType::IDENT))
            {
                error("Expected variable name identifier\n");
                return false;
            }

            vardefNode->m_name = m_matchString;
            head->m_children.emplace_back(std::move(vardefNode));

            matched = true;
        }
        else if (match(Lexer::TokenType::REP))
        {
            auto repNode = std::make_unique<ASTNode>(ASTNode::NodeType::REP);
            if (!parseExpression(repNode))
            {
                return false;
            }

            if (!parseStatements(repNode))
            {
                return false;
            }

            if (!match(Lexer::TokenType::ENDREP))
            {
                error("Expected 'ENDREP'\n");
                return false;
            }

            auto endrepNode = std::make_unique<ASTNode>(ASTNode::NodeType::ENDREP);
            repNode->m_children.emplace_back(std::move(endrepNode));
            
            head->m_children.emplace_back(std::move(repNode));
            matched = true;
        }
        else if (match(Lexer::TokenType::WAIT))
        {
            auto waitNode = std::make_unique<ASTNode>(ASTNode::NodeType::WAIT);
            if (!parseExpression(waitNode))
            {
                return false;
            }

            head->m_children.emplace_back(std::move(waitNode));

            matched = true;
        }
        else if (match(Lexer::TokenType::RET))
        {
            auto retNode = std::make_unique<ASTNode>(ASTNode::NodeType::RETURN);
            if (!parseExpression(retNode))
            {
                return false;
            }

            head->m_children.emplace_back(std::move(retNode));

            matched = true;
        }
        else if (match(Lexer::TokenType::IDENT))
        {
            // could be variable assignment or function
            auto funcName = m_matchString;

            if (match(Lexer::TokenType::LPAREN))
            {
                // function call
                auto callNode = std::make_unique<ASTNode>(ASTNode::NodeType::CALL);
                callNode->m_name = funcName;

                // argument list
                if (parseExpression(callNode))
                {
                    while(match(Lexer::TokenType::COMMA))
                    {
                        if (!parseExpression(callNode))
                        {
                            error("Expected an expression\n");
                            return false;
                        }
                    }
                }
                
                if (!match(Lexer::TokenType::RPAREN))
                {
                    error("Expected ')'\n");
                    return false;
                }

                head->m_children.emplace_back(std::move(callNode));
            }
            else
            {
                auto assignNode = std::make_unique<ASTNode>(ASTNode::NodeType::ASSIGN);

                assignNode->m_name = m_matchString;

                // assignment 
                if (match(Lexer::TokenType::LBRACKET))
                {
                    // optional bit identifier
                    if (!match(Lexer::TokenType::INT))
                    {
                        error("Integer expected \n");
                        return false;
                    }

                    // FIXME: robustness
                    assignNode->m_integer = atoi(m_matchString.c_str());

                    if (!match(Lexer::TokenType::RBRACKET))
                    {
                        error("Expected ']'\n");
                        return false;
                    }
                }

                if (!match(Lexer::TokenType::EQUAL))
                {
                    error("Expected '='\n");
                    return false;
                }

                if (!parseExpression(assignNode))
                {
                    error("Malformed expression\n");
                    return false;
                }

                head->m_children.emplace_back(std::move(assignNode));
            }
            matched = true;
        }        
    }

    return true;
}

bool Parser::Parse::parseExpression(std::unique_ptr<ASTNode> &head)
{
    if (match(Lexer::TokenType::IDENT))
    {
        auto funcName = m_matchString;

        // variable or function
        if (match(Lexer::TokenType::LPAREN))
        {
            // function call
            auto callNode = std::make_unique<ASTNode>(ASTNode::NodeType::CALL);
            callNode->m_name = funcName;

            // argument list
            if (parseExpression(callNode))
            {
                while(match(Lexer::TokenType::COMMA))
                {
                    if (!parseExpression(callNode))
                    {
                        error("Expected an expression\n");
                        return false;
                    }
                }
            }

            if (!match(Lexer::TokenType::RPAREN))
            {
                error("Expected ')'\n");
                return false;
            }

            head->m_children.emplace_back(std::move(callNode));
        }
        else
        {       
            auto varNode = std::make_unique<ASTNode>(ASTNode::NodeType::VARIABLE);
            varNode->m_name = m_matchString;

            // variable, optional bitindex
            if (match(Lexer::TokenType::LBRACKET))
            {
                if (!match(Lexer::TokenType::INT))
                {
                    error("Expected bit index integer\n");
                    return false;
                }

                //FIXME: robustness
                varNode->m_integer = atoi(m_matchString.c_str());

                if (!match(Lexer::TokenType::RBRACKET))
                {
                    error("Expected ']'\n");
                    return false;
                }
            }

            head->m_children.emplace_back(std::move(varNode));
        }
        return true;
    }

    if (match(Lexer::TokenType::INT))
    {
        auto intNode = std::make_unique<ASTNode>(ASTNode::NodeType::INT);
        
        // FIXME: make more robust
        intNode->m_integer = atoi(m_matchString.c_str());
        head->m_children.emplace_back(std::move(intNode));
        return true;
    }

    if (match(Lexer::TokenType::INPUT))
    {
        auto ioDirNode = std::make_unique<ASTNode>(ASTNode::NodeType::IODIR);
        ioDirNode->m_iotype = IOType::INPUT;
        head->m_children.emplace_back(std::move(ioDirNode));
        return true;
    }

    if (match(Lexer::TokenType::OUTPUT))
    {
        auto ioDirNode = std::make_unique<ASTNode>(ASTNode::NodeType::IODIR);
        ioDirNode->m_iotype = IOType::OUTPUT;
        head->m_children.emplace_back(std::move(ioDirNode));
        return true;
    }

    if (match(Lexer::TokenType::SHR))
    {
        auto shrNode = std::make_unique<ASTNode>(ASTNode::NodeType::SHR);
        if (!parseExpression(shrNode))
        {
            return false;
        }

        head->m_children.emplace_back(std::move(shrNode));

        return true;
    }

    return false;
}
