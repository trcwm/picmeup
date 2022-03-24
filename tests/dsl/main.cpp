#include <cstdlib>
#include <fstream>
#include <iomanip>
#include "lexer.h"
#include "parser.h"
#include "astmanip.h"
#include "codegen.h"
#include "disasm.h"

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

    auto vmcode = cg.m_code;

    size_t address = 0;
    while(address < vmcode.size())
    {
        auto oldflags = std::cout.flags();

        auto result = VM::disasm(&vmcode.at(address));
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address << result.m_txt;
        address += result.m_bytes;

        std::cout.flags(oldflags);
    }

    VM::Machine vm;
    vm.reset();
    if (vm.load(&vmcode.at(0), vmcode.size()))
    {
        std::cout << "VM loaded\n";
    }

    // execute a PROC send
    vm.setIP(0x000);
    vm.push(0xAA55);
    vm.push(16);

    for(uint32_t i=0; i<300; i++)
    {
        //auto oldflags = std::cout.flags();

        //auto result = VM::disasm(&vmcode.at(vm.getIP()));
        //std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << vm.getIP() << result.m_txt;

        //std::cout.flags(oldflags);

        vm.execInstruction();
        //std::cout << "  TOS:" << vm.getTOS() << "  SP:" << vm.getSP() << "  BP:" << vm.getBP() << "  new IP:" << vm.getIP() << "\n";
    }

    return EXIT_SUCCESS;
}
