project(dsl_test)

add_executable(dsl_test        
    ../../src/dsl/lexer.cpp
    ../../src/dsl/parser.cpp
    ../../src/dsl/codegen.cpp
    ../../src/dsl/astmanip.cpp
    ../../src/dsl/ast.cpp
    ../../src/dsl/vm.cpp
    main.cpp
)

target_include_directories(dsl_test PUBLIC ../../src/dsl)
