#include "ir/lexer/lexer.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
namespace Hazel {
namespace Snap{
namespace IR {
bool Lexer::has_errors() const{
    for (const auto& d : this->diagnostics){
        if(d.level == Utils::DiagLevel::error){
            return true;
        }
    }
    return false;
}

const std::vector<Utils::Diagnostic>& Lexer::get_diagnostics() const{
    return this->diagnostics;
}


Utils::Location Lexer::here() const{
    return Utils::Location{
        this->line_num,
        this->col,
        this->filename,
        this->curr_line
    };
}

void Lexer::report(Utils::DiagLevel level,const std::string& message,const std::string& sub_message,const std::string& code){
    this->diagnostics.push_back(Utils::Diagnostic{
        this->here(),
        message,
        sub_message,
        code,
        level
    });
}

void Lexer::report_error(const std::string& msg, const std::string& sub){
    this->report(Utils::DiagLevel::error, msg, sub);
}

void Lexer::report_warning(const std::string& msg, const std::string& sub){
    this->report(Utils::DiagLevel::warning, msg, sub);
}
}
}
}