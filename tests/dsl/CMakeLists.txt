project(dsl_test)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

add_executable(dsl_test        
    ../../src/dsl/lexer.cpp
    ../../src/dsl/parser.cpp
    ../../src/dsl/codegen.cpp
    ../../src/dsl/astmanip.cpp
    ../../src/dsl/ast.cpp
    ../../src/dsl/vm.cpp
    ../../src/dsl/disasm.cpp
    main.cpp
)

target_include_directories(dsl_test PUBLIC ../../src/dsl)
