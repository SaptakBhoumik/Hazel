#pragma once
#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
//std::vector<TypeExprKind> = {Type of arg1, Type of arg2, Type of arg3, ..., type of destination}. No destination type for instructions that do not have a destination
const static std::unordered_map<std::string, std::vector<std::vector<TypeExprKind>>> instruction_arg_type_map = {
    // Load instuction
    {".load",{{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr},//For string/array/struct, it copies the underlying content. For ptr, just the address is copied
                  {TypeExprKind::IntTypeExpr, TypeExprKind::DecimalTypeExpr},//For int to decimal conversion
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr},//For decimal to int conversion
                  {TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr},//For int to ptr conversion
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr},//For ptr to int conversion
                  {TypeExprKind::ArrayTypeExpr, TypeExprKind::PtrTypeExpr},//For array to ptr conversion. Use the underlying ptr of the array as the output ptr
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::ArrayTypeExpr},//For ptr to array conversion. Use the input ptr as the underlying ptr of the array. 
                  {TypeExprKind::StringTypeExpr, TypeExprKind::PtrTypeExpr},//For string to ptr conversion. Use the underlying ptr of the string as the output ptr
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::StringTypeExpr}}},//For ptr to string conversion. Use the input ptr as the underlying ptr of the string.
    
    //Load the second argument if the first argument is missing
    {".load_if_missing",{{TypeExprKind::AnyTypeExpr,TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr},//For string/array/struct, it copies the underlying content. For ptr, just the address is copied
                             {TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::DecimalTypeExpr},//For int to decimal conversion
                             {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr},//For decimal to int conversion
                             {TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr},//For int to ptr conversion
                             {TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr},//For ptr to int conversion
                             {TypeExprKind::ArrayTypeExpr,TypeExprKind::ArrayTypeExpr, TypeExprKind::PtrTypeExpr},//For array to ptr conversion. Use the underlying ptr of the array as the output ptr
                             {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::ArrayTypeExpr},//For ptr to array conversion. Use the input ptr as the underlying ptr of the array. 
                             {TypeExprKind::StringTypeExpr,TypeExprKind::StringTypeExpr, TypeExprKind::PtrTypeExpr},//For string to ptr conversion. Use the underlying ptr of the string as the output ptr
                             {TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::StringTypeExpr}}},//For ptr to string conversion. Use the input ptr as the underlying ptr of the string.

    // Binary arithmetic instruction
    {".add", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr},
                  {TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr},
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr}}},
    {".sub", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr},
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr}}},
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

    // Other arithmetic instruction

    // Binary bitwise instruction
    {".and", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".or",  {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".xor", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},

    // Comparison Instruction
    {".eq", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".ne", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".gt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".lt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".ge", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".le", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}, 
                  {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                       {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                       {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".not_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
                           {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}, 
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
    {".br_eq", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_ne", {{TypeExprKind::AnyTypeExpr, TypeExprKind::AnyTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_gt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_lt", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_ge", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_le", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                     {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                          {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                          {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_not_inrange", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                              {TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}, 
                              {TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::DecimalTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_or", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_and", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_xor", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_has_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_has_no_value", {{TypeExprKind::AnyTypeExpr,TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_true", {{TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br_false", {{TypeExprKind::IntTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr, TypeExprKind::LabelTypeExpr}}},
    {".br", {{TypeExprKind::LabelTypeExpr}}},

    // Heap and related operations
    {".alloc", {{TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr},//Element size is determined by the tye of ptr
                     {TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::ArrayTypeExpr},//Element size is determined by the size of base type and if packed or not
                     {TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::StringTypeExpr}}},//Element size is 1 byte
    {".realloc", {{TypeExprKind::PtrTypeExpr,TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr},
                       {TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::ArrayTypeExpr},
                       {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::StringTypeExpr}}},
    {".free", {{TypeExprKind::PtrTypeExpr},
                    {TypeExprKind::ArrayTypeExpr},
                    {TypeExprKind::StringTypeExpr}}},
    {".slice", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::PtrTypeExpr},
                     {TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::ArrayTypeExpr},
                     {TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::StringTypeExpr}}},
    {".pop", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr},
                   {TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".del", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr},//Delete n item(specified by last) at index specified by second item
                   {TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".push", {{TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr},
                    {TypeExprKind::ArrayTypeExpr,TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr},
                    {TypeExprKind::StringTypeExpr,TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".push_at", {{TypeExprKind::PtrTypeExpr,TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr},
                       {TypeExprKind::ArrayTypeExpr,TypeExprKind::ArrayTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr},
                       {TypeExprKind::StringTypeExpr,TypeExprKind::StringTypeExpr, TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".get_elem_no_missing", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::AnyTypeExpr}}},//Dont read missing field. Not even nested ones
    {".set_elem_no_missing", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::AnyTypeExpr}}},//Dont write missing field. Not even nested ones
    {".memcpy", {{TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".memmove", {{TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".memset", {{TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".memcmp", {{TypeExprKind::PtrTypeExpr, TypeExprKind::PtrTypeExpr, TypeExprKind::IntTypeExpr, TypeExprKind::IntTypeExpr}}},
    {".len", {{TypeExprKind::PtrTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".cap", {{TypeExprKind::PtrTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                   {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},
    {".element_size", {{TypeExprKind::PtrTypeExpr,TypeExprKind::IntTypeExpr},
                            {TypeExprKind::ArrayTypeExpr,TypeExprKind::IntTypeExpr},
                            {TypeExprKind::StringTypeExpr,TypeExprKind::IntTypeExpr}}},

    // Other operation
    {".nop",{}},
    {".halt",{}},
    {".trap",{}},
    {".trap_true",{{TypeExprKind::IntTypeExpr}}},
    {".trap_is_missing",{{TypeExprKind::AnyTypeExpr}}},
    {".trace",{}},
    {".trace_true",{{TypeExprKind::IntTypeExpr}}},
    {".trace_is_missing",{{TypeExprKind::AnyTypeExpr}}},
    {".unreachable",{}},
    {".print",{{TypeExprKind::StringTypeExpr},
                    {TypeExprKind::PtrTypeExpr}}}
};
}
}
}