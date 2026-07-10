#pragma once
#include <cstdint>
namespace Hazel{
namespace Snap{
namespace VM{
enum class ValueType:std::uint8_t{
    VT_INSTRUCTION,
    VT_FUNC_LOC,
    VT_LABEL_LOC,
    VT_REG,
    VT_I64,
    VT_FLOAT,
    VT_PTR,
};
struct Value{
    std::int64_t value = 0;//If i64 or fixed point float then interpreted as that. If ptr then interpreted as pointer(capacity, length and element size must be stored then)
                           //Reg means register number. Instruction means instruction number. Func_loc means function location in the bytecode. Label_loc means label location in the bytecode
                           //Note:We replace the value(which contains the instruction) with the address of the computed goto label. This value can contain the hot dispatch address if type == instruction
                           //Note:If the type == func_loc then we replace the value(which contains the function location) with the address of the function in the bytecode. Do same for label

    std::int64_t capacity = 0;//If the instruction is a cold instruction then we reuse the capacity field to store the cold dispatch address.
    std::int64_t length = 0;
    std::int16_t element_size = 0;//Should be enough
    ValueType type = ValueType::VT_I64;
    bool is_missing = true;

    Value() = default;
    Value(ValueType type) noexcept;
    Value(std::int64_t value, ValueType type) noexcept;
    Value(std::int64_t value, std::int64_t capacity, std::int64_t length, std::int16_t element_size) noexcept;
};
}
}
namespace Trend{};
}