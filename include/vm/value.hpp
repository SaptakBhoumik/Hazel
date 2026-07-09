#pragma once
#include <cstdint>
namespace Hazel{
namespace Snap{
enum class ValueType:std::uint8_t{
    VT_INSTRUCTION,
    VT_FUNC_LOC,
    VT_LABEL_LOC,
    VT_REG,
    VT_I64,
    VT_FLOAT,
    VT_PTR,
};
struct StreamValue{
    std::int64_t value = 0;//If i64 or fixed point float then interpreted as that. If ptr then interpreted as pointer(capacity, length and element size must be stored then)
                       //Reg means register number. Instruction means instruction number. Func_loc means function location in the bytecode. Label_loc means label location in the bytecode
    std::int64_t capacity = 0;
    std::int64_t length = 0;
    std::int16_t element_size = 0;//Should be enough
    ValueType type = ValueType::VT_I64;
    bool is_missing = true;

    StreamValue() = default;
    StreamValue(ValueType type) noexcept;
    StreamValue(std::int64_t value, ValueType type) noexcept;
    StreamValue(std::int64_t value, std::int64_t capacity, std::int64_t length, std::int16_t element_size) noexcept;
};
struct BatchValue{};
}
namespace Trend{};
}