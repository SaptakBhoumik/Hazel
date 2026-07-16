ir_lexer_src = [
    'lexer/snap/error.cpp',
    'lexer/snap/lexer.cpp',
    'lexer/snap/token.cpp',
    'lexer/snap/utils.cpp'
]

ir_ast_src = [
    'ast/snap/ast_literal_node.cpp',
    'ast/snap/ast_type_node.cpp',
    'ast/snap/ast.cpp',
]

ir_parser_src = [
    'parser/snap/parse_type_expr.cpp',
    'parser/snap/parse_literal_expr.cpp',
    'parser/snap/parser.cpp',
    'parser/snap/utils.cpp',
    'parser/snap/basic_typechecker.cpp',
]

ir_include = include_directories('../../include/')
ir_lexer = static_library('lexer', sources: ir_lexer_src, include_directories: ir_include)
ir_ast = static_library('ast', sources: ir_ast_src, include_directories: ir_include)
ir_parser = static_library('parser', sources: ir_parser_src, include_directories: ir_include)