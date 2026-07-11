#pragma once
#include "value.hpp"
#include "opcode.hpp"
#include <cstddef>
#include <vector>

#pragma once
#include <cstdint>
namespace Hazel{
namespace Snap{
namespace VM{
using  ExternFuncType = std::int64_t (*)(Value** args, Value** ret_value);//ret_value is pointer to the stack frame

class VM{
    std::vector<Value> code;
    std::size_t frame_buffer_size;
    std::size_t max_function_call_depth;
    public:
    VM(std::vector<Value> code, std::size_t frame_buffer_size, std::size_t max_function_call_depth) noexcept;
};
}
}
namespace Trend{};
}