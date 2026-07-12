project('Hazel', 'cpp', version: '0.1')

cpp_src = [
    'src/hazel.cpp',
]

include = include_directories('include/')

add_project_arguments('-std=c++2a', language: 'cpp')


subdir('src/utils')
subdir('src/ir')

executable(
    'hazel.elf',
    sources: cpp_src, 
    include_directories: include,
    link_with: [utils, ir_lexer, ir_ast, ir_parser]
)