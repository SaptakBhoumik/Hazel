#pragma once
#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
//std::vector<TypeExprKind> = {Type of arg1, Type of arg2, Type of arg3, ..., type of destination}. If no destination then destination type is VoidTypeExpr
const static std::unordered_map<std::string, std::vector<std::vector<TypeExprKind>>> instruction_arg_type_map = {
    // Local and related instuction
    {".local",{{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr},//For string/array/struct, it copies the underlying content i.e deep copy. For ptr, just the address is copied
                  {TypeExprKind::IntTypeExpr, TypeExprKind::DecimalTypeExpr},//For int to decimal conversion
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr},//For decimal to int conversion
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::ArrayTypeExpr},//For ptr to array conversion. Use the input ptr as the underlying ptr of the array. 
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::StringTypeExpr},//For ptr to string conversion. Use the input ptr as the underlying ptr of the string.
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::StructTypeExpr}}},//For ptr to struct conversion. Use the input ptr as the underlying ptr of the struct.
    //Load the second argument if the first argument is missing
    {".local_if_missing",{{TypeExprKind::AnyTypeExpr,TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr},//For string/array/struct, it copies the underlying content. For ptr, just the address is copied
                          {TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::DecimalTypeExpr},//For int to decimal conversion
                          {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr},//For decimal to int conversion
                          {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::ArrayTypeExpr},//For ptr to array conversion. Use the input ptr as the underlying ptr of the array. 
                          {TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::StringTypeExpr},//For ptr to string conversion. Use the input ptr as the underlying ptr of the string.                        
                          {TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::StructTypeExpr}}},//For ptr to struct conversion. Use the input ptr as the underlying ptr of the struct.
    
    //THe integer for the following is the idx at which to start reading/writing(Offset = element size * index)
    {".load", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::AnyTypeExpr}}},//If AnyTypeExpr is array/struct/str then it reads the ptr at that offset and returns it as a array/struct/str. Doesnt deepcopy(Use load afterwards if u need that) 
    {".load_no_missing", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::AnyTypeExpr}}},//Same rule as above+doesnt read missing value of AnyTypeExpr(Assume no such value) but may read the missing value of the base type if the base type of AnyTypeExpr allowed to have
    {".store", {{TypeExprKind::PtrTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr}}},//If AnyTypeExpr is array/struct/str then it just writes the underlying ptr without copys. Returns the same ptr after storing(Maybe mising if out of bound store)
    {".store_no_missing", {{TypeExprKind::PtrTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr}}},//Same rule as above. Doesnt write the missing value of AnyTypeExpr(Returns missing ptr if AnyTypeExpr is missing else the same ptr unless out of bound)  but may write the missing value of the base type if the base type of AnyTypeExpr allowed to have missing
    
    // Binary arithmetic instruction
    {".add", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".sub", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".mul", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".div", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".mod", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".min", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},
    {".max", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr}}},

    // Binary bitwise instruction
    {".and", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".or",  {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".xor", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},

    // Comparison Instruction
    {".eq", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".ne", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".gt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".lt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".ge", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".le", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                       {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".not_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                           {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    
    // Unary Instruction
    {".abs", {{TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr}, 
                   {TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".neg", {{TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr}, 
                   {TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".not", {{TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".ceil", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".floor", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".integral_part", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".fractional_part", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".round_nearest", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".round_even", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".sqrt", {{TypeExprKind::DecimalTypeExpr,TypeExprKind::DecimalTypeExpr}}},
    {".has_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".has_no_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::IntTypeExpr}}},

    // Branching operation
    {".br_eq", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_ne", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_gt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_lt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_ge", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_le", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                          {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_not_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}, 
                              {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_or", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_and", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_xor", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_has_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_has_no_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_true", {{TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br_false", {{TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".br", {{TypeExprKind::LabelTypeExpr,TypeExprKind::VoidTypeExpr}}},

    // Heap and related operations
    {".alloc", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::ArrayTypeExpr},//Element size is determined by the size of base type and if packed or not
                     {TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::StringTypeExpr},
                     {TypeExprKind::StructTypeExpr}}},//Element size is 1 byte
    {".free", {{TypeExprKind::PtrTypeExpr, TypeExprKind::VoidTypeExpr},
                    {TypeExprKind::ArrayTypeExpr, TypeExprKind::VoidTypeExpr},
                    {TypeExprKind::StringTypeExpr, TypeExprKind::VoidTypeExpr},
                    {TypeExprKind::StructTypeExpr, TypeExprKind::VoidTypeExpr}}},
    {".pop", {{TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::ArrayTypeExpr},//Return the same ptr after popping. Returns missing if out of bound
                   {TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::StringTypeExpr}}},//Return the same ptr after popping. Returns missing if out of bound
    {".del", {{TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::ArrayTypeExpr},//Reutrn the same ptr to array after deleting. Returns missing if out of bound
                   {TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::StringTypeExpr}}},//Reutrn the same ptr to string after deleting. Returns missing if out of bound
    {".extend", {{TypeExprKind::ArrayTypeExpr,TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::ArrayTypeExpr},
                    {TypeExprKind::StringTypeExpr,TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::StringTypeExpr}}},
    {".extend_at", {{TypeExprKind::ArrayTypeExpr,TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::ArrayTypeExpr},
                       {TypeExprKind::StringTypeExpr,TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::StringTypeExpr}}},
    {".memcmp", {{TypeExprKind::StringTypeExpr, TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".len", {{TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".cap", {{TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".element_size", {{TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                            {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},

    // Other operation
    {".nop",{{TypeExprKind::VoidTypeExpr}}},
    {".halt",{{TypeExprKind::VoidTypeExpr}}},
    {".trap",{{TypeExprKind::VoidTypeExpr}}},
    {".trap_true",{{TypeExprKind::IntTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".trap_is_missing",{{TypeExprKind::AnyTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".trace",{{TypeExprKind::VoidTypeExpr}}},
    {".trace_true",{{TypeExprKind::IntTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".trace_is_missing",{{TypeExprKind::AnyTypeExpr,TypeExprKind::VoidTypeExpr}}},
    {".unreachable",{{TypeExprKind::VoidTypeExpr}}},
    {".print",{{TypeExprKind::StringTypeExpr,TypeExprKind::VoidTypeExpr}}},
};

const std::unordered_set<std::string> terminator_inst = {
    ".br_eq",
    ".br_ne",
    ".br_gt",
    ".br_lt",
    ".br_ge",
    ".br_le",
    ".br_inrange",
    ".br_not_inrange",
    ".br_or",
    ".br_and",
    ".br_xor",
    ".br_has_value",
    ".br_has_no_value",
    ".br_true",
    ".br_false",
    ".br",
    ".halt",
    ".ret",
    ".trap",
    ".trap_true",
    ".trap_is_missing",
    ".unreachable"
};
}
}
}