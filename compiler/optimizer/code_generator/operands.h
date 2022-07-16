/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_CODEGEN_OPERANDS_H_
#define COMPILER_OPTIMIZER_CODEGEN_OPERANDS_H_

/*
Arch-feature definitions
*/
#include <bitset>
#include <cstdint>

#include "utils/arch.h"
#include "utils/arena_containers.h"
#include "utils/bit_field.h"
#include "utils/bit_utils.h"
#include "utils/regmask.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/datatype.h"
#include "utils/type_helpers.h"

#ifdef __clang_analyzer__
#ifdef PANDA_TARGET_ARM32
#define __arm__
#endif
#endif

namespace panda::compiler {
constexpr uint8_t BYTE_SIZE = 8;
constexpr uint8_t HALF_SIZE = 16;
constexpr uint8_t WORD_SIZE = 32;
constexpr uint8_t DOUBLE_WORD_SIZE = 64;
constexpr uint8_t WORD_SIZE_BYTE = 4;
constexpr uint8_t DOUBLE_WORD_SIZE_BYTE = 8;
constexpr uint8_t QUAD_WORD_SIZE_BYTE = 16;
/// Maximum possible registers count (for scalar and for vector):
constexpr uint8_t MAX_NUM_REGS = 32;
constexpr uint8_t MAX_NUM_VREGS = 32;

constexpr uint64_t NAN_DOUBLE = uint64_t(0x7ff8000000000000);
constexpr uint32_t NAN_FLOAT = uint32_t(0x7fc00000);
constexpr uint32_t NAN_FLOAT_BITS = NAN_FLOAT >> 16U;

// Constants for cast from float to int64:
// The number of the bit from which exponential part starts in float
constexpr uint8_t START_EXP_FLOAT = 23;
// Size exponential part in float
constexpr uint8_t SIZE_EXP_FLOAT = 8;
// The maximum exponential part of float that can be loaded in int64
constexpr uint32_t POSSIBLE_EXP_FLOAT = 0xbe;
// Mask say that float number is NaN by IEEE 754
constexpr uint32_t UP_BITS_NAN_FLOAT = 0xff;

// Constants for cast from double to int64:
// The number of the bit from which exponential part starts in double
constexpr uint8_t START_EXP_DOUBLE = 20;
// Size exponential part in double
constexpr uint8_t SIZE_EXP_DOUBLE = 11;
// The maximum exponential part of double that can be loaded in int64
constexpr uint32_t POSSIBLE_EXP_DOUBLE = 0x43e;
// Mask say that double number is NaN by IEEE 754
constexpr uint32_t UP_BITS_NAN_DOUBLE = 0x7ff;

constexpr uint32_t SHIFT_BITS_DOUBLE = 12;
constexpr uint32_t SHIFT_BITS_FLOAT = 9;

// Return true, if architecture can be encoded.
bool BackendSupport(Arch arch);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_MATH_LIST(DEF)  \
    DEF(Mov, UnaryOperation)   \
    DEF(Neg, UnaryOperation)   \
    DEF(Abs, UnaryOperation)   \
    DEF(Not, UnaryOperation)   \
    DEF(Add, BinaryOperation)  \
    DEF(Sub, BinaryOperation)  \
    DEF(Mul, BinaryOperation)  \
    DEF(Shl, BinaryOperation)  \
    DEF(Shr, BinaryOperation)  \
    DEF(AShr, BinaryOperation) \
    DEF(And, BinaryOperation)  \
    DEF(Or, BinaryOperation)   \
    DEF(Xor, BinaryOperation)  \
    DEF(Sqrt, UnaryOperation)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_INST_WITH_SHIFTED_OPERAND(DEF)   \
    DEF(And, BinaryShiftedRegisterOperation)    \
    DEF(Or, BinaryShiftedRegisterOperation)     \
    DEF(Xor, BinaryShiftedRegisterOperation)    \
    DEF(OrNot, BinaryShiftedRegisterOperation)  \
    DEF(AndNot, BinaryShiftedRegisterOperation) \
    DEF(XorNot, BinaryShiftedRegisterOperation) \
    DEF(Add, BinaryShiftedRegisterOperation)    \
    DEF(Sub, BinaryShiftedRegisterOperation)

//  Arch-independent access types

/**
 * Template class for identify types compile-time (nortti - can't use typeid).
 * Used in register class. Immediate class support conversion to it.
 */
class TypeInfo final {
public:
    enum TypeId : uint8_t { INT8 = 0, INT16 = 1, INT32 = 2, INT64 = 3, FLOAT32 = 4, FLOAT64 = 5, INVALID = 6 };

    /**
     * Template constructor - use template parameter for create object.
     */
    template <class T>
    constexpr explicit TypeInfo(T /* unused */)
    {
#ifndef __clang_analyzer__
        if constexpr (std::is_same<T, uint8_t>()) {
            type_id_ = INT8;
        } else if constexpr (std::is_same<T, int8_t>()) {
            type_id_ = INT8;
        } else if constexpr (std::is_same<T, uint16_t>()) {
            type_id_ = INT16;
        } else if constexpr (std::is_same<T, int16_t>()) {
            type_id_ = INT16;
        } else if constexpr (std::is_same<T, uint32_t>()) {
            type_id_ = INT32;
        } else if constexpr (std::is_same<T, int32_t>()) {
            type_id_ = INT32;
        } else if constexpr (std::is_same<T, uint64_t>()) {
            type_id_ = INT64;
        } else if constexpr (std::is_same<T, int64_t>()) {
            type_id_ = INT64;
        } else if constexpr (std::is_same<T, float>()) {
            type_id_ = FLOAT32;
        } else if constexpr (std::is_same<T, double>()) {
            type_id_ = FLOAT64;
        } else {
            type_id_ = INVALID;
        }
#endif
    }

    constexpr explicit TypeInfo(TypeId type) : type_id_(type) {}

    DEFAULT_MOVE_SEMANTIC(TypeInfo);
    DEFAULT_COPY_SEMANTIC(TypeInfo);
    ~TypeInfo() = default;

    /**
     * Constructor for create invalid TypeInfo
     */
    constexpr TypeInfo() = default;

    /**
     * Validation check
     */
    constexpr bool IsValid() const
    {
        return type_id_ != INVALID;
    }

    /**
     * Type expected size
     */
    constexpr size_t GetSize() const
    {
        ASSERT(IsValid());
        switch (type_id_) {
            case INT8:
                return BYTE_SIZE;
            case INT16:
                return HALF_SIZE;
            case INT32:
            case FLOAT32:
                return WORD_SIZE;
            case INT64:
            case FLOAT64:
                return DOUBLE_WORD_SIZE;
            default:
                return 0;
        }
        return 0;
    }

    constexpr bool IsFloat() const
    {
        ASSERT(IsValid());
        return type_id_ == FLOAT32 || type_id_ == FLOAT64;
    }

    constexpr bool IsScalar() const
    {
        // VOID - is scalar type here
        return !IsFloat();
    }

    constexpr bool operator==(const TypeInfo &other) const
    {
        return (type_id_ == other.type_id_);
    }

    constexpr bool operator!=(const TypeInfo &other) const
    {
        return !operator==(other);
    }

    static TypeInfo FromDataType(DataType::Type type, Arch arch)
    {
        switch (type) {
            case DataType::BOOL:
            case DataType::UINT8:
            case DataType::INT8: {
                return TypeInfo(INT8);
            }
            case DataType::UINT16:
            case DataType::INT16: {
                return TypeInfo(INT16);
            }
            case DataType::UINT32:
            case DataType::INT32: {
                return TypeInfo(INT32);
            }
            case DataType::UINT64:
            case DataType::INT64:
            case DataType::ANY: {
                return TypeInfo(INT64);
            }
            case DataType::FLOAT32: {
                return TypeInfo(FLOAT32);
            }
            case DataType::FLOAT64: {
                return TypeInfo(FLOAT64);
            }
            case DataType::REFERENCE: {
                return FromDataType(DataType::GetIntTypeForReference(arch), arch);
            }
            case DataType::POINTER: {
                return Is64BitsArch(arch) ? TypeInfo(INT64) : TypeInfo(INT32);
            }
            default:
                UNREACHABLE();
        }
    }

    DataType::Type ToDataType() const
    {
        switch (type_id_) {
            case INT8:
                return DataType::INT8;
            case INT16:
                return DataType::INT16;
            case INT32:
                return DataType::INT32;
            case INT64:
                return DataType::INT64;
            case FLOAT32:
                return DataType::FLOAT32;
            case FLOAT64:
                return DataType::FLOAT64;
            default:
                UNREACHABLE();
        }
    }

    static constexpr TypeInfo GetScalarTypeBySize(size_t size);

    void Dump()
    {
        std::cerr << "TypeInfo:";
        switch (type_id_) {
            case INT8:
                std::cerr << "INT8";
                break;
            case INT16:
                std::cerr << "INT16";
                break;
            case INT32:
                std::cerr << "INT32";
                break;
            case FLOAT32:
                std::cerr << "FLOAT32";
                break;
            case INT64:
                std::cerr << "INT64";
                break;
            case FLOAT64:
                std::cerr << "FLOAT64";
                break;
            default:
                std::cerr << "INVALID";
                break;
        }
        std::cerr << ", size = " << GetSize();
    }

private:
    TypeId type_id_ {INVALID};
};

constexpr TypeInfo INT8_TYPE {TypeInfo::INT8};
constexpr TypeInfo INT16_TYPE {TypeInfo::INT16};
constexpr TypeInfo INT32_TYPE {TypeInfo::INT32};
constexpr TypeInfo INT64_TYPE {TypeInfo::INT64};
constexpr TypeInfo FLOAT32_TYPE {TypeInfo::FLOAT32};
constexpr TypeInfo FLOAT64_TYPE {TypeInfo::FLOAT64};
constexpr TypeInfo INVALID_TYPE;

constexpr TypeInfo TypeInfo::GetScalarTypeBySize(size_t size)
{
    auto type = INT64_TYPE;
    if (size == BYTE_SIZE) {
        type = INT8_TYPE;
    } else if (size == HALF_SIZE) {
        type = INT16_TYPE;
    } else if (size == WORD_SIZE) {
        type = INT32_TYPE;
    }
    return type;
}

// Mapping model for registers:
// reg-reg - support getters for small parts of registers
// reg-other - mapping between types of registers
enum RegMapping : uint32_t {
    SCALAR_SCALAR = 1UL << 0UL,
    SCALAR_VECTOR = 1UL << 1UL,
    SCALAR_FLOAT = 1UL << 2UL,
    VECTOR_VECTOR = 1UL << 3UL,
    VECTOR_FLOAT = 1UL << 4UL,
    FLOAT_FLOAT = 1UL << 5UL
};

constexpr uint8_t INVALID_REG_ID = std::numeric_limits<uint8_t>::max();
constexpr uint8_t ACC_REG_ID = INVALID_REG_ID - 1U;

class Reg final {
public:
    using RegIDType = uint8_t;
    using RegSizeType = size_t;

    constexpr Reg() = default;
    DEFAULT_MOVE_SEMANTIC(Reg);
    DEFAULT_COPY_SEMANTIC(Reg);
    ~Reg() = default;

    // Default register constructor
    constexpr Reg(RegIDType id, TypeInfo type) : id_(id), type_(type) {}

    constexpr RegIDType GetId() const
    {
        return id_;
    }

    constexpr size_t GetMask() const
    {
        return (1U << id_);
    }

    constexpr TypeInfo GetType() const
    {
        return type_;
    }

    RegSizeType GetSize() const
    {
        return GetType().GetSize();
    }

    bool IsScalar() const
    {
        return GetType().IsScalar();
    }

    bool IsFloat() const
    {
        return GetType().IsFloat();
    }

    constexpr bool IsValid() const
    {
        return type_ != INVALID_TYPE && id_ != INVALID_REG_ID;
    }

    Reg As(TypeInfo type) const
    {
        return Reg(GetId(), type);
    }

    constexpr bool operator==(Reg other) const
    {
        return (GetId() == other.GetId()) && (GetType() == other.GetType());
    }

    constexpr bool operator!=(Reg other) const
    {
        return !operator==(other);
    }

    void Dump()
    {
        std::cerr << " Reg: id = " << static_cast<int64_t>(id_) << ", ";
        type_.Dump();
        std::cerr << "\n";
    }

private:
    RegIDType id_ {INVALID_REG_ID};
    TypeInfo type_ {INVALID_TYPE};
};  // Reg

constexpr Reg INVALID_REGISTER = Reg();

static_assert(!INVALID_REGISTER.IsValid());
static_assert(sizeof(Reg) <= sizeof(uintptr_t));

/**
 * Immediate class may hold only int or float values (maybe vectors in future).
 * It knows nothing about pointers and bools (bools maybe be in future).
 */
class Imm final {
    static inline constexpr uint8_t BITS_PER_BYTE = 8;
    static constexpr size_t UNDEFINED_SIZE = 0;
    static constexpr size_t INT8_SIZE = 8;
    static constexpr size_t INT16_SIZE = 16;
    static constexpr size_t INT32_SIZE = 32;
    static constexpr size_t INT64_SIZE = 64;
    static constexpr size_t FLOAT32_SIZE = 32;
    static constexpr size_t FLOAT64_SIZE = 64;

    enum VariantID {
        // Pointer used for invalidate variants
        V_INT8 = 1,
        V_INT16 = 2,
        V_INT32 = 3,
        V_INT64 = 4,
        V_FLOAT32 = 5,
        V_FLOAT64 = 6,
    };

    template <class T>
    constexpr bool CheckVariantID() const
    {
#ifndef __clang_analyzer__
        // Immediate could be only signed (int/float)
        // look at value_-type.
        static_assert(std::is_signed<T>::value);
        if constexpr (std::is_same<T, int8_t>()) {
            return value_.index() == V_INT8;
        }
        if constexpr (std::is_same<T, int16_t>()) {
            return value_.index() == V_INT16;
        }
        if constexpr (std::is_same<T, int32_t>()) {
            return value_.index() == V_INT32;
        }
        if constexpr (std::is_same<T, int64_t>()) {
            return value_.index() == V_INT64;
        }
        if constexpr (std::is_same<T, float>()) {
            return value_.index() == V_FLOAT32;
        }
        if constexpr (std::is_same<T, double>()) {
            return value_.index() == V_FLOAT64;
        }
        return false;
#else
        return true;
#endif
    }

public:
    // Invalid constructor
    constexpr Imm() = default;

    // Special type constructor
    template <class T>
    constexpr explicit Imm(T value) : value_(value)
    {
    }

    // Partial template specialization
    constexpr explicit Imm(uint8_t value) : value_(static_cast<int8_t>(value)) {};

    constexpr explicit Imm(uint16_t value) : value_(static_cast<int16_t>(value)) {};

    constexpr explicit Imm(uint32_t value) : value_(static_cast<int32_t>(value)) {};

    constexpr explicit Imm(uint64_t value) : value_(static_cast<int64_t>(value)) {};

#if (PANDA_TARGET_MACOS)
    constexpr explicit Imm(size_t value) : value_(static_cast<int64_t>(value)) {};

    constexpr explicit Imm(long value) : value_(static_cast<int64_t>(value)) {};
#endif

    DEFAULT_MOVE_SEMANTIC(Imm);
    DEFAULT_COPY_SEMANTIC(Imm);
    ~Imm() = default;

    template <class T>
    T GetValue() const
    {
        ASSERT(CheckVariantID<T>());
        ASSERT(sizeof(T) * BITS_PER_BYTE == GetSize());
        return std::get<T>(value_);
    }

    void Inc(size_t value)
    {
        switch (value_.index()) {
            case V_INT8:
                value_ = static_cast<int8_t>(std::get<int8_t>(value_) + value);
                break;
            case V_INT16:
                value_ = static_cast<int16_t>(std::get<int16_t>(value_) + value);
                break;
            case V_INT32:
                value_ = static_cast<int32_t>(std::get<int32_t>(value_) + value);
                break;
            case V_INT64:
                value_ = static_cast<int64_t>(std::get<int64_t>(value_) + value);
                break;
            case V_FLOAT32:
                value_ = static_cast<float>(std::get<float>(value_) + value);
                break;
            case V_FLOAT64:
                value_ = static_cast<double>(std::get<double>(value_) + value);
                break;
            default:
                // Check before increment
                UNREACHABLE();
                break;
        }
    }

    void Dec(size_t value)
    {
        switch (value_.index()) {
            case V_INT8:
                value_ = static_cast<int8_t>(std::get<int8_t>(value_) - value);
                break;
            case V_INT16:
                value_ = static_cast<int16_t>(std::get<int16_t>(value_) - value);
                break;
            case V_INT32:
                value_ = static_cast<int32_t>(std::get<int32_t>(value_) - value);
                break;
            case V_INT64:
                value_ = static_cast<int64_t>(std::get<int64_t>(value_) - value);
                break;
            case V_FLOAT32:
                value_ = static_cast<float>(std::get<float>(value_) - value);
                break;
            case V_FLOAT64:
                value_ = static_cast<double>(std::get<double>(value_) - value);
                break;
            default:
                // Check before decrement
                UNREACHABLE();
                break;
        }
    }

    TypeInfo GetType() const
    {
        switch (value_.index()) {
            case V_INT8:
                return INT8_TYPE;
            case V_INT16:
                return INT16_TYPE;
            case V_INT32:
                return INT32_TYPE;
            case V_INT64:
                return INT64_TYPE;
            case V_FLOAT32:
                return FLOAT32_TYPE;
            case V_FLOAT64:
                return FLOAT64_TYPE;
            default:
                UNREACHABLE();
                return INVALID_TYPE;
        }
    }

    constexpr size_t GetSize() const
    {
        switch (value_.index()) {
            case V_INT8:
                return INT8_SIZE;
            case V_INT16:
                return INT16_SIZE;
            case V_INT32:
                return INT32_SIZE;
            case V_INT64:
                return INT64_SIZE;
            case V_FLOAT32:
                return FLOAT32_SIZE;
            case V_FLOAT64:
                return FLOAT64_SIZE;
            default:
                return UNDEFINED_SIZE;
        }
    }

    bool IsZero() const
    {
        if (std::holds_alternative<float>(value_)) {
            return std::get<float>(value_) == 0.0;
        }
        if (std::holds_alternative<double>(value_)) {
            return std::get<double>(value_) == 0.0;
        }
        if (std::holds_alternative<int8_t>(value_)) {
            return std::get<int8_t>(value_) == 0;
        }
        if (std::holds_alternative<int16_t>(value_)) {
            return std::get<int16_t>(value_) == 0;
        }
        if (std::holds_alternative<int32_t>(value_)) {
            return std::get<int32_t>(value_) == 0;
        }
        if (std::holds_alternative<int64_t>(value_)) {
            return std::get<int64_t>(value_) == 0;
        }
        return true;
    }

    bool IsFloat() const
    {
        return std::holds_alternative<float>(value_) || std::holds_alternative<double>(value_);
    }

    bool IsScalar() const
    {
        return std::holds_alternative<int8_t>(value_) || std::holds_alternative<int16_t>(value_) ||
               std::holds_alternative<int32_t>(value_) || std::holds_alternative<int64_t>(value_);
    }

    bool IsValid() const
    {
        bool hold_data = std::holds_alternative<int8_t>(value_) || std::holds_alternative<int16_t>(value_) ||
                         std::holds_alternative<int32_t>(value_) || std::holds_alternative<int64_t>(value_) ||
                         std::holds_alternative<float>(value_) || std::holds_alternative<double>(value_);
        return (GetSize() != 0) && hold_data;
    }

    unsigned GetShift()
    {
        if (GetType() == INT64_TYPE) {
            return GetValue<int64_t>();
        }
        if (GetType() == INT32_TYPE) {
            return GetValue<int32_t>();
        }
        if (GetType() == INT16_TYPE) {
            return GetValue<int16_t>();
        }
        if (GetType() == INT8_TYPE) {
            return GetValue<int8_t>();
        }
        UNREACHABLE();
        return 0;
    }

    bool operator==(Imm other) const
    {
        return value_ == other.value_;
    }

    bool operator!=(Imm other) const
    {
        return !(operator==(other));
    }

private:
    std::variant<void *, int8_t, int16_t, int32_t, int64_t, float, double> value_ {nullptr};
};  // Imm

constexpr Imm INVALID_IMM = Imm();

// Why memory ref - because you may create one link for one encode-session
// And when you see this one - you can easy understand, what type of memory
//   you use. But if you load/store dirrectly address - you need to decode it
//   each time, when you read code
// model -> base + index<<scale + disp
class MemRef final {
public:
    MemRef() = default;

    explicit MemRef(Reg base) : MemRef(base, 0) {}
    MemRef(Reg base, ssize_t disp) : MemRef(base, INVALID_REGISTER, 0, disp) {}
    MemRef(Reg base, Reg index, uint16_t scale) : MemRef(base, index, scale, 0) {}
    MemRef(Reg base, Reg index, uint16_t scale, ssize_t disp) : disp_(disp), scale_(scale), base_(base), index_(index)
    {
        CHECK_LE(disp, std::numeric_limits<decltype(disp_)>::max());
        CHECK_LE(scale, std::numeric_limits<decltype(scale_)>::max());
    }
    DEFAULT_MOVE_SEMANTIC(MemRef);
    DEFAULT_COPY_SEMANTIC(MemRef);
    ~MemRef() = default;

    Reg GetBase() const
    {
        return base_;
    }
    Reg GetIndex() const
    {
        return index_;
    }
    auto GetScale() const
    {
        return scale_;
    }
    auto GetDisp() const
    {
        return disp_;
    }

    bool HasBase() const
    {
        return base_.IsValid();
    }
    bool HasIndex() const
    {
        return index_.IsValid();
    }
    bool HasScale() const
    {
        return HasIndex() && scale_ != 0;
    }
    bool HasDisp() const
    {
        return disp_ != 0;
    }
    // Ref must contain at least one of field
    bool IsValid() const
    {
        return HasBase() || HasIndex() || HasScale() || HasDisp();
    }

    // return true if mem doesn't has index and scalar
    bool IsOffsetMem() const
    {
        return !HasIndex() && !HasScale();
    }

    bool operator==(MemRef other) const
    {
        return (base_ == other.base_) && (index_ == other.index_) && (scale_ == other.scale_) && (disp_ == other.disp_);
    }
    bool operator!=(MemRef other) const
    {
        return !(operator==(other));
    }

private:
    ssize_t disp_ {0};
    uint16_t scale_ {0};
    Reg base_ {INVALID_REGISTER};
    Reg index_ {INVALID_REGISTER};
};  // MemRef

inline ArenaVector<std::pair<uint8_t, uint8_t>> ResoveParameterSequence(
    ArenaVector<std::pair<uint8_t, uint8_t>> *moved_registers, uint8_t tmp, ArenaAllocator *allocator)
{
    constexpr uint8_t INVALID_FIST = -1;
    constexpr uint8_t INVALID_SECOND = -2;

    moved_registers->emplace_back(std::pair<uint8_t, uint8_t>(INVALID_FIST, INVALID_SECOND));
    /*
        Example:
        1. mov x0 <- x3
        2. mov x1 <- x0
        3. mov x2 <- x3
        4. mov x3 <- x2
        Agreement - in dst can't be multipy same registers (double move to one register)
                  - src for movs can hold same register multiply times

        Algorithm:
            1. Find handing edges (x1 - just in dst)
                emit "2. mov x1 <- x0"
                goto 1.
                emit "1. mov x0 <- x3"
            2. Assert all registers used just one time (loop from registers sequence)
               All multiply-definitions must be resolved on previous step
                emit ".. mov xtmp <- x2" (strore xtmp == x3)
                emit "3. mov x2 <- x3"
                emit "4. mov x3 <- xtmp" (ASSERT(4->GetReg == x3) - there is no other possible situations here)
    */
    // Calculate weigth
    ArenaVector<std::pair<uint8_t, uint8_t>> result(allocator->Adapter());
    // --moved_registers->end() - for remove marker-element
    for (auto pair = moved_registers->begin(); pair != --moved_registers->end();) {
        auto conflict = std::find_if(moved_registers->begin(), moved_registers->end(), [pair](auto in_pair) {
            return (in_pair.second == pair->first && (in_pair != *pair));
        });
        if (conflict == moved_registers->end()) {
            // emit immediate - there are no another possible combinations
            result.emplace_back(*pair);
            moved_registers->erase(pair);
            pair = moved_registers->begin();
        } else {
            ++pair;
        }
    }
    // Here just loops
    for (;;) {
        /* Need support single mov x1 <- x1:
           ASSERT(moved_registers->size() != 1);
     */

        auto curr_pair = moved_registers->begin();

        if (curr_pair->first == INVALID_FIST && curr_pair->second == INVALID_SECOND) {
            moved_registers->erase(curr_pair);
            break;
            // Finish algorithm - only marker in vector
        }
        auto saved_reg = curr_pair->first;
        result.emplace_back(std::pair<uint8_t, uint8_t>(tmp, curr_pair->first));
        result.emplace_back(*curr_pair);  // we already save dst_register

        // Remove current instruction
        auto curr_reg = curr_pair->second;
        moved_registers->erase(curr_pair);

        for (; curr_pair != moved_registers->end();) {
            curr_pair = std::find_if(moved_registers->begin(), moved_registers->end(),
                                     [curr_reg](auto in_pair) { return in_pair.first == curr_reg; });
            if (curr_pair != moved_registers->end()) {
                if (curr_pair->second == saved_reg) {
                    result.emplace_back(std::pair<uint8_t, uint8_t>(curr_pair->first, tmp));
                    moved_registers->erase(curr_pair);
                    break;
                    // exit from loop
                };
                result.emplace_back(*curr_pair);
                curr_reg = curr_pair->second;
                moved_registers->erase(curr_pair);
            } else {
                ASSERT(curr_pair != moved_registers->end());
            }
        }
    }
    return result;
}

// Condition also used for tell comparison registers type
enum Condition {
    EQ,  // equal to 0
    NE,  // not equal to 0
    // signed
    LT,  // less
    LE,  // less than or equal
    GT,  // greater
    GE,  // greater than or equal
    // unsigned - checked from registers
    LO,  // less
    LS,  // less than or equal
    HI,  // greater
    HS,  // greater than or equal
    // Special arch-dependecy TODO (igorban) Fix them
    MI,  // N set            Negative
    PL,  // N clear          Positive or zero
    VS,  // V set            Overflow.
    VC,  // V clear          No overflow.
    AL,  //                  Always.
    NV,  // Behaves as always/al.

    TST_EQ,
    TST_NE,

    INVALID_COND
};

static inline bool IsTestCc(Condition cond)
{
    return cond == TST_EQ || cond == TST_NE;
}

class Shift final {
public:
    explicit Shift(Reg base, ShiftType type, uint32_t scale) : scale_(scale), base_(base), type_(type) {}
    explicit Shift(Reg base, uint32_t scale) : Shift(base, ShiftType::LSL, scale) {}

    DEFAULT_MOVE_SEMANTIC(Shift);
    DEFAULT_COPY_SEMANTIC(Shift);
    ~Shift() = default;

    Reg GetBase() const
    {
        return base_;
    }

    ShiftType GetType() const
    {
        return type_;
    }

    uint32_t GetScale() const
    {
        return scale_;
    }

private:
    uint32_t scale_ {0};
    Reg base_;
    ShiftType type_ {INVALID_SHIFT};
};

inline int64_t GetIntValue(Imm imm)
{
    int64_t value {0};
    auto type = imm.GetType();
    if (type == INT32_TYPE) {
        value = imm.GetValue<int32_t>();
    } else if (type == INT64_TYPE) {
        value = imm.GetValue<int64_t>();
    } else if (type == INT16_TYPE) {
        value = imm.GetValue<int16_t>();
    } else if (type == INT8_TYPE) {
        value = imm.GetValue<int8_t>();
    } else {
        // Inconsistent int-type
        UNREACHABLE();
    }
    return value;
}
}  // namespace panda::compiler
#endif  // COMPILER_OPTIMIZER_CODEGEN_REGISTERS_H_
