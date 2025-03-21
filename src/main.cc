#include <stdio.h>
#include <iostream>

#include "frontend/lexer/Lexer.h"
#include "frontend/SourceManager.h"
#include "frontend/parser/Statement.h"
#include "frontend/parser/Expression.h"
#include "util/Logger.h"
#include "frontend/parser/Parser.h"
#include "frontend/types/Decl.h"
#include "frontend/types/Types.h"

#define DEBUG 1

void printTree(const std::unique_ptr<Entity>& node, int depth = 0) {
    if (!node) {
        std::cerr << "Null node encountered in printTree" << std::endl;
        return;
    }

    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    std::cout << "Node: " << node->getKind();
    if (auto expr = dynamic_cast<Expression*>(node.get())) {
        std::cout << " (Expression)";
    } else if (auto stmt = dynamic_cast<Statement*>(node.get())) {
        std::cout << " (Statement)";
    }
    std::cout << std::endl;

    for (const auto& child : node->children) {
        printTree(child, depth + 1);
    }
}

bool performSemanticAnalysis(const std::unique_ptr<Entity>& node) {
    if (!node) {
        std::cerr << "Semantic analysis encountered a null node" << std::endl;
        return false;
    }

    if (!node->validate()) {
        std::cerr << "Semantic error in node: " << node->getKind() << std::endl;
        return false;
    }

    for (const auto& child : node->children) {
        if (!performSemanticAnalysis(child)) {
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source_file> <times>" << std::endl;
        return 1;
    }

    SourceManager sm;
    auto buff = std::make_shared<SourceBuffer>(sm.readSource(argv[1]));

    Lexer lexer(buff);
    std::vector<std::unique_ptr<Token>> tokens;

    int times = std::stoi(argv[2]);
    for (int i = 0; i < times; i++) {
        std::unique_ptr<Token> next_tok = lexer.next();
        if (!next_tok) {
            std::cerr << "Lexer returned nullptr at position " << i << std::endl;
            break;
        }
        tokens.push_back(std::move(next_tok));

        std::cout << "[ "
                  << tokens.back()->line << ":"
                  << tokens.back()->column << " "
                  << Lexer::getTokenTypeName(tokens.back()->kind);

        switch (tokens.back()->value.index()) {
            case 1: {
                std::cout << ", " << std::get<int>(tokens.back()->value);
            } break;
            case 2: {
                std::cout << ", " << std::get<double>(tokens.back()->value);
            } break;
            case 3: {
                std::cout << ", " << std::get<std::string>(tokens.back()->value);
            } break;
            default: break;
        }

        std::cout << " ]" << std::endl;
    }

    Parser parser(std::move(tokens));
    auto root = parser.parseProgram();

    if (!root) {
        std::cerr << "Failed to build the program tree" << std::endl;
        return 1;
    }

    std::cout << "\nProgram Tree:" << std::endl;
    printTree(root);

    std::cout << "\nPerforming semantic analysis..." << std::endl;
    bool isValid = performSemanticAnalysis(root);

    if (isValid) {
        std::cout << "Semantic analysis completed successfully." << std::endl;
    } else {
        std::cerr << "Semantic analysis failed." << std::endl;
    }

    return 0;
}