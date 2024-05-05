#pragma once
#include <cstdint>

namespace arline::types
{
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float;
    using f64 = double;

    using b8 = bool;
    using c8 = char;
    using v0 = void;
}

namespace arline
{
    using arline::types::u8;
    using arline::types::u16;
    using arline::types::u32;
    using arline::types::u64;
    using arline::types::i8;
    using arline::types::i16;
    using arline::types::i32;
    using arline::types::i64;
    using arline::types::f32;
    using arline::types::f64;
    using arline::types::b8;
    using arline::types::c8;
    using arline::types::v0;
}