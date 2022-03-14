#include <cstdlib>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "astmanip.h"
#include "codegen.h"

struct DumpVisitor : public AbstractConstVisitor
{
    DumpVisitor(std::ostream &os) : m_os(os) {}

    void visit(const ASTNode *node) override
    {        
        for(auto i=0; i<m_indent; i++)
        {
            m_os << "  ";
        }        
        m_os << "Node: " << node->m_type;
        if (!node->m_name.empty())
        {
            m_os << "  " << node->m_name;
        }

        if (node->m_integer >= 0)
        {
            m_os << "  int:" << node->m_integer;
        }
        m_os << "\n";

        m_indent++;
        for(auto &child : node->m_children)
        {
            visit(child.get());
        }
        m_indent--;
    }

    size_t m_indent = 0;
    std::ostream &m_os;
};

int main(int argc, char *argv[])
{
    std::ifstream ifile(argv[1]);

    if (!ifile.good())
    {
        std::cerr << "Cannot open file\n";
        return EXIT_FAILURE;
    }

    Lexer::Lex lex(ifile);

    auto toks = lex.process();

    for(auto const& tok : toks)
    {
        std::cout << tok << "\n";
    }

    Parser::Parse parse(toks);
    auto ast = parse.process();

    std::cout << "AST manipulation \n";
    ASTManip::PinDirectionCreatorVisitor pindir;
    pindir.visit(ast.get());

    std::cout << "AST dump: \n";
    DumpVisitor visitor(std::cout);
    visitor.visit(ast.get());
    std::cout << "\n\n";

    std::cout << "Codegen \n";
    CodeGen::CodeGenVisitor cg(std::cout);
    cg.visit(ast.get());
    std::cout << "\n\n";

    return EXIT_SUCCESS;
}
