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

#ifndef COMPILER_OPTIMIZER_IR_INST_H
#define COMPILER_OPTIMIZER_IR_INST_H

#include <array>
#include <vector>
#include <iostream>
#include "constants.h"
#include "datatype.h"
#include "ir-dyn-base-types.h"
#include "marker.h"
#include "utils/arena_containers.h"
#include "utils/span.h"
#include "utils/bit_field.h"
#include "utils/bit_utils.h"
#include "utils/bit_vector.h"
#include "macros.h"
#include "mem/arena_allocator.h"
#include "opcodes.h"
#include "compiler_options.h"
#include "runtime_interface.h"
#include "spill_fill_data.h"

namespace panda::compiler {
class Inst;
class BasicBlock;
class Graph;
class GraphVisitor;
class VnObject;
class SaveStateItem;
class LocationsInfo;
using InstVector = ArenaVector<Inst *>;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, base, ...) class base;
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
OPCODE_LIST(INST_DEF)
#undef INST_DEF

/*
 * Condition code, used in Compare, If[Imm] and Select[Imm] instructions.
 *
 * N.B. BranchElimination and Peephole rely on the order of these codes. Change carefully.
 */
enum ConditionCode {
    // All types.
    CC_EQ = 0,  // ==
    CC_NE,      // !=
    // Signed integers and floating-point numbers.
    CC_LT,  // <
    CC_LE,  // <=
    CC_GT,  // >
    CC_GE,  // >=
    // Unsigned integers.
    CC_B,   // <
    CC_BE,  // <=
    CC_A,   // >
    CC_AE,  // >=
    // Compare result of bitwise AND with zero
    CC_TST_EQ,  // (lhs AND rhs) == 0
    CC_TST_NE,  // (lhs AND rhs) != 0
    // First and last aliases.
    CC_FIRST = CC_EQ,
    CC_LAST = CC_TST_NE,
};

inline ConditionCode GetInverseConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
            return ConditionCode::CC_NE;
        case ConditionCode::CC_NE:
            return ConditionCode::CC_EQ;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_GE;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_LE;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_LT;

        case ConditionCode::CC_B:
            return ConditionCode::CC_AE;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_A;
        case ConditionCode::CC_A:
            return ConditionCode::CC_BE;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_B;

        case ConditionCode::CC_TST_EQ:
            return ConditionCode::CC_TST_NE;
        case ConditionCode::CC_TST_NE:
            return ConditionCode::CC_TST_EQ;

        default:
            UNREACHABLE();
    }
}

inline ConditionCode InverseSignednessConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
            return ConditionCode::CC_EQ;
        case ConditionCode::CC_NE:
            return ConditionCode::CC_NE;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_B;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_BE;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_A;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_AE;

        case ConditionCode::CC_B:
            return ConditionCode::CC_LT;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_LE;
        case ConditionCode::CC_A:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_GE;

        case ConditionCode::CC_TST_EQ:
            return ConditionCode::CC_TST_EQ;
        case ConditionCode::CC_TST_NE:
            return ConditionCode::CC_TST_NE;

        default:
            UNREACHABLE();
    }
}

inline bool IsSignedConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_LT:
        case ConditionCode::CC_LE:
        case ConditionCode::CC_GT:
        case ConditionCode::CC_GE:
            return true;

        case ConditionCode::CC_EQ:
        case ConditionCode::CC_NE:
        case ConditionCode::CC_B:
        case ConditionCode::CC_BE:
        case ConditionCode::CC_A:
        case ConditionCode::CC_AE:
        case ConditionCode::CC_TST_EQ:
        case ConditionCode::CC_TST_NE:
            return false;

        default:
            UNREACHABLE();
    }
}

inline ConditionCode SwapOperandsConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
        case ConditionCode::CC_NE:
            return code;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_GE;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_LT;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_LE;

        case ConditionCode::CC_B:
            return ConditionCode::CC_A;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_AE;
        case ConditionCode::CC_A:
            return ConditionCode::CC_B;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_BE;

        case ConditionCode::CC_TST_EQ:
        case ConditionCode::CC_TST_NE:
            return code;

        default:
            UNREACHABLE();
    }
}

enum class Opcode {
    INVALID = -1,
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, ...) opcode,
    OPCODE_LIST(INST_DEF)

#undef INST_DEF
        NUM_OPCODES
};

/**
 * Convert opcode to its string representation
 */
constexpr std::array<const char *const, static_cast<size_t>(Opcode::NUM_OPCODES)> OPCODE_NAMES = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, ...) #opcode,
    OPCODE_LIST(INST_DEF)
#undef INST_DEF
};

constexpr const char *GetOpcodeString(Opcode opc)
{
    ASSERT(static_cast<int>(opc) < static_cast<int>(Opcode::NUM_OPCODES));
    return OPCODE_NAMES[static_cast<int>(opc)];
}

/**
 * Instruction flags. See `instrutions.yaml` section `flags` for more information.
 */
namespace inst_flags {
namespace internal {
enum FlagsIndex {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FLAG_DEF(flag) flag##_INDEX,
    FLAGS_LIST(FLAG_DEF)
#undef FLAG_DEF
        FLAGS_COUNT
};
}  // namespace internal

enum Flags : uint32_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FLAG_DEF(flag) flag = (1U << internal::flag##_INDEX),
    FLAGS_LIST(FLAG_DEF)
#undef FLAG_DEF
        FLAGS_COUNT = internal::FLAGS_COUNT,
    NONE = 0
};

inline constexpr uintptr_t GetFlagsMask(Opcode opcode)
{
#define INST_DEF(OPCODE, BASE, FLAGS) FLAGS,  // NOLINT(cppcoreguidelines-macro-usage)
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    constexpr std::array<uintptr_t, static_cast<int>(Opcode::NUM_OPCODES)> INST_FLAGS_TABLE = {OPCODE_LIST(INST_DEF)};
#undef INST_DEF
    return INST_FLAGS_TABLE[static_cast<size_t>(opcode)];
}
}  // namespace inst_flags

#ifndef NDEBUG
namespace inst_modes {
namespace internal {
enum ModeIndex {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MODE_DEF(mode) mode##_INDEX,
    MODES_LIST(MODE_DEF)
#undef MODE_DEF
        MODES_COUNT
};
}  // namespace internal

enum Mode : uint8_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MODE_DEF(mode) mode = (1U << internal::mode##_INDEX),
    MODES_LIST(MODE_DEF)
#undef MODE_DEF
        MODES_COUNT = internal::MODES_COUNT,
};

inline constexpr uint8_t GetModesMask(Opcode opcode)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    constexpr std::array<uint8_t, static_cast<int>(Opcode::NUM_OPCODES)> INST_MODES_TABLE = {INST_MODES_LIST};
    return INST_MODES_TABLE[static_cast<size_t>(opcode)];
}
}  // namespace inst_modes
#endif

namespace internal {
inline constexpr std::array<const char *, ShiftType::INVALID_SHIFT + 1> SHIFT_TYPE_NAMES = {"LSL", "LSR", "ASR", "ROR",
                                                                                            "INVALID"};
}  // namespace internal

inline const char *GetShiftTypeStr(ShiftType type)
{
    ASSERT(type <= INVALID_SHIFT);
    return internal::SHIFT_TYPE_NAMES[type];
}

/**
 * Describes type of the object produced by an instruction.
 */
class ObjectTypeInfo {
public:
    using ClassType = RuntimeInterface::ClassPtr;

    ObjectTypeInfo() = default;
    explicit ObjectTypeInfo(ClassType v) : class_(v) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator bool() const
    {
        return class_ != ClassType();
    }

    ClassType GetClass() const
    {
        return class_;
    }

    bool IsValid() const
    {
        return class_ != ClassType {};
    }

private:
    ClassType class_ {};
};

/**
 * Class for storing panda bytecode's virtual register
 */
class VirtualRegister final {
public:
    using ValueType = uint16_t;
    static constexpr unsigned BITS_FOR_VREG = (sizeof(ValueType) * BITS_PER_BYTE) - 1;
    static constexpr ValueType INVALID = std::numeric_limits<ValueType>::max();

    VirtualRegister() = default;
    explicit VirtualRegister(uint16_t v, bool is_acc) : value_(v)
    {
        IsAccFlag::Set(is_acc, &value_);
    }

    explicit operator uint16_t() const
    {
        return value_;
    }

    uint16_t Value() const
    {
        return ValueField::Get(value_);
    }

    bool IsAccumulator() const
    {
        return IsAccFlag::Get(value_);
    }

private:
    uint16_t value_ {INVALID};

    using ValueField = BitField<unsigned, 0, BITS_FOR_VREG>;
    using IsAccFlag = ValueField::NextFlag;
};

// How many bits will be used in Inst's bit fields for number of inputs.
constexpr size_t BITS_PER_INPUTS_NUM = 3;
// Maximum number of static inputs
constexpr size_t MAX_STATIC_INPUTS = (1U << BITS_PER_INPUTS_NUM) - 1;

/**
 * Currently Input class is just a wrapper for the Inst class.
 */
class Input final {
public:
    Input() = default;
    explicit Input(Inst *inst) : inst_(inst) {}

    Inst *GetInst()
    {
        return inst_;
    }
    const Inst *GetInst() const
    {
        return inst_;
    }

    static inline uint8_t GetPadding(Arch arch, uint32_t inputs_count)
    {
        return static_cast<uint8_t>(!Is64BitsArch(arch) && inputs_count % 2U == 1U);
    }

private:
    Inst *inst_ {nullptr};
};

/**
 * User is a intrusive list node, thus it stores pointers to next and previous users.
 * Also user has properties value to determine owner instruction and corresponding index of the input.
 */
class User final {
public:
    User() = default;
    User(bool is_static, unsigned index, unsigned size)
        : properties_(IsStaticFlag::Encode(is_static) | IndexField::Encode(index) | SizeField::Encode(size) |
                      BbNumField::Encode(BbNumField::MaxValue()))
    {
        ASSERT(index < 1U << (BITS_FOR_INDEX - 1U));
        ASSERT(size < 1U << (BITS_FOR_SIZE - 1U));
    }
    ~User() = default;

    // Copy/move semantic is disabled because we use tricky pointer arithmetic based on 'this' value
    NO_COPY_SEMANTIC(User);
    NO_MOVE_SEMANTIC(User);

    Inst *GetInst();
    const Inst *GetInst() const
    {
        return const_cast<User *>(this)->GetInst();
    }

    Inst *GetInput();
    const Inst *GetInput() const;

    bool IsDynamic() const
    {
        return !IsStaticFlag::Decode(properties_);
    }
    unsigned GetIndex() const
    {
        return IndexField::Decode(properties_);
    }
    unsigned GetSize() const
    {
        return SizeField::Decode(properties_);
    }

    VirtualRegister GetVirtualRegister() const
    {
        ASSERT(IsDynamic());
        return VirtualRegister(VregField::Decode(properties_), IsAccFlag::Decode(properties_));
    }

    void SetVirtualRegister(VirtualRegister reg)
    {
        static_assert(sizeof(reg) <= sizeof(uintptr_t), "Consider passing the register by reference");
        ASSERT(IsDynamic());
        VregField::Set(reg.Value(), &properties_);
        IsAccFlag::Set(reg.IsAccumulator(), &properties_);
    }

    uint32_t GetBbNum() const
    {
        ASSERT(IsDynamic());
        return BbNumField::Decode(properties_);
    }

    void SetBbNum(uint32_t bb_num)
    {
        ASSERT(IsDynamic());
        BbNumField::Set(bb_num, &properties_);
    }

    auto GetNext() const
    {
        return next_;
    }

    auto GetPrev() const
    {
        return prev_;
    }

    void SetNext(User *next)
    {
        next_ = next;
    }

    void SetPrev(User *prev)
    {
        prev_ = prev;
    }

    void Remove()
    {
        if (prev_ != nullptr) {
            prev_->next_ = next_;
        }
        if (next_ != nullptr) {
            next_->prev_ = prev_;
        }
    }

private:
    static constexpr unsigned BITS_FOR_INDEX = 21;
    static constexpr unsigned BITS_FOR_SIZE = BITS_FOR_INDEX;
    static constexpr unsigned BITS_FOR_BB_NUM = 20;
    using IndexField = BitField<unsigned, 0, BITS_FOR_INDEX>;
    using SizeField = IndexField::NextField<unsigned, BITS_FOR_SIZE>;
    using IsStaticFlag = SizeField::NextFlag;

    using BbNumField = IsStaticFlag::NextField<uint32_t, BITS_FOR_BB_NUM>;

    using VregField = IsStaticFlag::NextField<unsigned, VirtualRegister::BITS_FOR_VREG>;
    using IsAccFlag = VregField::NextFlag;

    uint64_t properties_ {0};
    User *next_ {nullptr};
    User *prev_ {nullptr};
};

/**
 * List of users. Intended for range loop.
 * @tparam T should be User or const User
 */
template <typename T>
class UserList {
    template <typename U>
    struct UserIterator {
        UserIterator() = default;
        explicit UserIterator(U *u) : user_(u) {}

        UserIterator &operator++()
        {
            user_ = user_->GetNext();
            return *this;
        }
        bool operator!=(const UserIterator &other)
        {
            return user_ != other.user_;
        }
        U &operator*()
        {
            return *user_;
        }
        U *operator->()
        {
            return user_;
        }

    private:
        U *user_ {nullptr};
    };

public:
    using Iterator = UserIterator<T>;
    using ConstIterator = UserIterator<const T>;
    using PointerType = std::conditional_t<std::is_const_v<T>, T *const *, T **>;

    explicit UserList(PointerType head) : head_(head) {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator begin()
    {
        return Iterator(*head_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator end()
    {
        return Iterator(nullptr);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator begin() const
    {
        return ConstIterator(*head_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator end() const
    {
        return ConstIterator(nullptr);
    }
    bool Empty() const
    {
        return *head_ == nullptr;
    }
    T &Front()
    {
        return **head_;
    }
    const T &Front() const
    {
        return **head_;
    }

private:
    PointerType head_ {nullptr};
};

inline bool operator==(const User &lhs, const User &rhs)
{
    return lhs.GetInst() == rhs.GetInst();
}

/**
 * Operands class for instructions with fixed inputs count.
 * Actually, this class do absolutely nothing except that we can get sizeof of it when allocating memory.
 */
template <int N>
struct Operands {
    static_assert(N < MAX_STATIC_INPUTS, "Invalid inputs number");

    std::array<User, N> users;
    std::array<Input, N> inputs;
};

/**
 * Specialized version for instructions with variable inputs count.
 * Users and inputs are stored outside of this class.
 */
class DynamicOperands {
public:
    explicit DynamicOperands(ArenaAllocator *allocator) : allocator_(allocator) {}

    User *Users()
    {
        return users_;
    }

    Input *Inputs()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<Input *>(users_ + capacity_) + 1;
    }

    /// Append new input (and user accordingly)
    unsigned Append(Inst *inst);

    /// Remove input and user with index `index`.
    void Remove(unsigned index);

    /// Reallocate inputs/users storage to a new one with specified capacity.
    void Reallocate(size_t new_capacity = 0);

    /// Get instruction to which these operands belongs to.
    Inst *GetOwnerInst() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<Inst *>(const_cast<DynamicOperands *>(this) + 1);
    }

    User *GetUser(unsigned index)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &users_[capacity_ - index - 1];
    }

    Input *GetInput(unsigned index)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &Inputs()[index];
    }

    void SetInput(unsigned index, Input input)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Inputs()[index] = input;
    }

    size_t Size() const
    {
        return size_;
    }

private:
    User *users_ {nullptr};
    size_t size_ {0};
    size_t capacity_ {0};
    ArenaAllocator *allocator_ {nullptr};
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_INST(TYPE) void Accept(GraphVisitor *v) override

/**
 * Base class for all instructions, should not be instantiated directly
 */
class InstBase {
    NO_COPY_SEMANTIC(InstBase);
    NO_MOVE_SEMANTIC(InstBase);

public:
    virtual ~InstBase() = default;

public:
    virtual void Accept(GraphVisitor *v) = 0;

    ALWAYS_INLINE void operator delete([[maybe_unused]] void *unused, [[maybe_unused]] size_t size)
    {
        UNREACHABLE();
    }
    ALWAYS_INLINE void *operator new([[maybe_unused]] size_t size, void *ptr) noexcept
    {
        return ptr;
    }
    ALWAYS_INLINE void operator delete([[maybe_unused]] void *unused1, [[maybe_unused]] void *unused2) noexcept {}

    void *operator new([[maybe_unused]] size_t size) = delete;

protected:
    InstBase() = default;
};

/**
 * Base instruction class
 */
class Inst : public MarkerSet, public InstBase {
public:
    DECLARE_INST(Inst);

public:
    /**
     * Create new instruction. All instructions must be created with this method.
     * It allocates additional space before Inst object for def-use structures.
     *
     * @tparam InstType - concrete type of instruction, shall be derived from Inst
     * @tparam Args - constructor arguments types
     * @param allocator - allocator for memory allocating
     * @param args - constructor arguments
     * @return - new instruction
     */
    template <typename InstType, typename... Args>
    [[nodiscard]] static InstType *New(ArenaAllocator *allocator, Args &&... args);

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, base, ...) inline const base *CastTo##opcode() const;
    OPCODE_LIST(INST_DEF)
#undef INST_DEF

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, base, ...) inline base *CastTo##opcode();
    OPCODE_LIST(INST_DEF)
#undef INST_DEF

    // Methods for instruction chaining inside basic blocks.
    Inst *GetNext()
    {
        return next_;
    }
    const Inst *GetNext() const
    {
        return next_;
    }
    Inst *GetPrev()
    {
        return prev_;
    }
    const Inst *GetPrev() const
    {
        return prev_;
    }
    void SetNext(Inst *next)
    {
        next_ = next;
    }
    void SetPrev(Inst *prev)
    {
        prev_ = prev;
    }

    // Id accessors
    auto GetId() const
    {
        return id_;
    }
    void SetId(int id)
    {
        id_ = id;
    }

    auto GetLinearNumber() const
    {
        return linear_number_;
    }
    void SetLinearNumber(LinearNumber number)
    {
        linear_number_ = number;
    }

    auto GetCloneNumber() const
    {
        return clone_number_;
    }
    void SetCloneNumber(int32_t number)
    {
        clone_number_ = number;
    }

    // Opcode accessors
    Opcode GetOpcode() const
    {
        return opcode_;
    }
    void SetOpcode(Opcode opcode)
    {
        opcode_ = opcode;
        SetField<FieldFlags>(inst_flags::GetFlagsMask(opcode));
    }
    const char *GetOpcodeStr() const
    {
        return GetOpcodeString(GetOpcode());
    }

    // Bytecode PC accessors
    uint32_t GetPc() const
    {
        return pc_;
    }
    void SetPc(uint32_t pc)
    {
        pc_ = pc;
    }

    // Type accessors
    DataType::Type GetType() const
    {
        return FieldType::Get(bit_fields_);
    }
    void SetType(DataType::Type type)
    {
        FieldType::Set(type, &bit_fields_);
    }
    bool HasType() const
    {
        return GetType() != DataType::Type::NO_TYPE;
    }

    // Parent basic block accessors
    BasicBlock *GetBasicBlock()
    {
        return bb_;
    }
    const BasicBlock *GetBasicBlock() const
    {
        return bb_;
    }
    void SetBasicBlock(BasicBlock *bb)
    {
        bb_ = bb;
    }

    // Instruction properties getters
    bool IsControlFlow() const
    {
        return GetFlag(inst_flags::CF);
    }
    bool IsVirtualCall() const
    {
        return GetOpcode() == Opcode::CallVirtual || GetOpcode() == Opcode::UnresolvedCallVirtual;
    }
    bool IsStaticCall() const
    {
        return GetOpcode() == Opcode::CallStatic || GetOpcode() == Opcode::UnresolvedCallStatic;
    }
    bool IsInitObject() const
    {
        return GetOpcode() == Opcode::InitObject;
    }
    bool IsMultiArray() const
    {
        return GetOpcode() == Opcode::MultiArray;
    }
    bool IsDynamicCall() const
    {
        return GetOpcode() == Opcode::CallDynamic;
    }
    bool IsIndirectCall() const
    {
        return GetOpcode() == Opcode::CallIndirect;
    }
    bool IsIntrinsic() const
    {
        /* Opcode::Builtin is left for backward compatibility, the compiler
         * itself should never generate an instruction with such an opcode */
        return GetOpcode() == Opcode::Intrinsic || GetOpcode() == Opcode::Builtin;
    }

    /* IsBuiltin actual meaning would be "it MAY be inlined by the CG"
     * however, since we do not make guarantees about whether it will
     * actually be inlined nor the safety of the intrinsic itself, just
     * checking the instruction flags to see if it is suitable for any
     * particular optimization seems to be a better approach
     */
    static bool IsBuiltin()
    {
        return false;
    }

    bool IsCall() const
    {
        return GetFlag(inst_flags::CALL);
    }

    bool IsSpillFill() const
    {
        return GetOpcode() == Opcode::SpillFill;
    }

    bool IsNullCheck() const
    {
        return GetOpcode() == Opcode::NullCheck;
    }

    bool IsNullPtr() const
    {
        return GetOpcode() == Opcode::NullPtr;
    }

    bool IsUnresolved() const
    {
        switch (GetOpcode()) {
            case Opcode::UnresolvedCallStatic:
            case Opcode::UnresolvedCallVirtual:
            case Opcode::UnresolvedLoadAndInitClass:
            case Opcode::UnresolvedLoadType:
            case Opcode::UnresolvedLoadStatic:
            case Opcode::UnresolvedStoreStatic:
            case Opcode::UnresolvedLoadObject:
            case Opcode::UnresolvedStoreObject:
                return true;
            default:
                return false;
        }
    }
    bool IsLoad() const
    {
        return GetFlag(inst_flags::LOAD);
    }
    bool IsStore() const
    {
        return GetFlag(inst_flags::STORE);
    }
    bool IsAccRead() const;
    bool IsAccWrite() const;
    bool IsMemory() const
    {
        return IsLoad() || IsStore();
    }
    bool CanThrow() const
    {
        return GetFlag(inst_flags::CAN_THROW);
    }
    bool IsCheck() const
    {
        return GetFlag(inst_flags::IS_CHECK);
    }
    bool RequireState() const
    {
        return GetFlag(inst_flags::REQUIRE_STATE);
    }
    // Returns true if the instruction not removable in DCE
    bool IsNotRemovable() const
    {
        return GetFlag(inst_flags::NO_DCE);
    }

    // Returns true if the instruction doesn't have destination register
    bool NoDest() const
    {
        return GetFlag(inst_flags::PSEUDO_DST) || GetFlag(inst_flags::NO_DST) || GetType() == DataType::VOID;
    }

    bool HasPseudoDestination() const
    {
        return GetFlag(inst_flags::PSEUDO_DST);
    }

    bool HasImplicitRuntimeCall() const
    {
        return GetFlag(inst_flags::IMPLICIT_RUNTIME_CALL);
    }

    bool CanDeoptimize() const
    {
        return GetFlag(inst_flags::CAN_DEOPTIMIZE);
    }

    // Returns true if the instruction is low-level
    bool IsLowLevel() const
    {
        return GetFlag(inst_flags::LOW_LEVEL);
    }

    // Returns true if the instruction not hoistable
    bool IsNotHoistable() const
    {
        return GetFlag(inst_flags::NO_HOIST);
    }

    // Returns true Cse can't be applied to the instruction
    bool IsNotCseApplicable() const
    {
        return GetFlag(inst_flags::NO_CSE);
    }

    // Returns true if the instruction is a barrier
    virtual bool IsBarrier() const
    {
        return GetFlag(inst_flags::BARRIER);
    }

    // Returns true if opcode can not be moved throught runtime calls (REFERENCE type only)
    bool IsRefSpecial() const
    {
        bool result = GetFlag(inst_flags::REF_SPECIAL);
        ASSERT(!result || GetType() == DataType::Type::REFERENCE);
        return result;
    }

    // Returns true if the instruction is a commutative
    bool IsCommutative() const
    {
        return GetFlag(inst_flags::COMMUTATIVE);
    }

    // Returns true if the instruction allocates a new object on the heap
    bool IsAllocation() const
    {
        return GetFlag(inst_flags::ALLOC);
    }

    // Returns true if the instruction can be used in if-conversion
    bool IsIfConvertable() const
    {
        return GetFlag(inst_flags::IFCVT);
    }

    virtual bool IsRuntimeCall() const
    {
        return GetFlag(inst_flags::RUNTIME_CALL);
    }

    virtual bool IsPropagateLiveness() const;

    // Returns true if the instruction doesn't have side effects(call runtime, throw e.t.c.)
    virtual bool IsSafeInst() const
    {
        return false;
    }

    bool RequireRegMap() const;

    ObjectTypeInfo GetObjectTypeInfo() const
    {
        return object_type_info_;
    }

    bool HasObjectTypeInfo() const
    {
        return object_type_info_.IsValid();
    }

    void SetObjectTypeInfo(ObjectTypeInfo o)
    {
        object_type_info_ = o;
    }

    Inst *GetDataFlowInput(int index) const
    {
        return GetDataFlowInput(GetInput(index).GetInst());
    }
    Inst *GetDataFlowInput(Inst *input_inst) const;

    bool IsPrecedingInSameBlock(const Inst *other) const;

    bool IsDominate(const Inst *other) const;

    bool InSameBlockOrDominate(const Inst *other) const;

    const SaveStateInst *GetSaveState() const
    {
        return const_cast<Inst *>(this)->GetSaveState();
    }

    SaveStateInst *GetSaveState()
    {
        if (!RequireState()) {
            return nullptr;
        }
        if (GetInputsCount() == 0) {
            return nullptr;
        }
        auto ss = GetInput(GetInputsCount() - 1).GetInst();
        if (ss->GetOpcode() == Opcode::SaveStateDeoptimize) {
            return ss->CastToSaveStateDeoptimize();
        }
        if (ss->GetOpcode() != Opcode::SaveState) {
            return nullptr;
        }

        return ss->CastToSaveState();
    }

    void SetSaveState(Inst *inst)
    {
        ASSERT(RequireState());
        SetInput(GetInputsCount() - 1, inst);
    }

    bool IsZeroRegInst() const;

    /**
     * Return instruction clone
     */
    virtual Inst *Clone(const Graph *targetGraph) const;

    uintptr_t GetFlagsMask() const
    {
        return GetField<FieldFlags>();
    }

    bool GetFlag(inst_flags::Flags flag) const
    {
        return (GetFlagsMask() & flag) != 0;
    }

    void SetFlag(inst_flags::Flags flag)
    {
        SetField<FieldFlags>(GetFlagsMask() | flag);
    }

    void ClearFlag(inst_flags::Flags flag)
    {
        SetField<FieldFlags>(GetFlagsMask() & ~static_cast<uintptr_t>(flag));
    }

#ifndef NDEBUG
    uint8_t GetModesMask() const
    {
        return inst_modes::GetModesMask(opcode_);
    }

    bool SupportsMode(inst_modes::Mode mode) const
    {
        return (GetModesMask() & mode) != 0;
    }
#endif

    void SetTerminator()
    {
        SetFlag(inst_flags::Flags::TERMINATOR);
    }

    void InsertBefore(Inst *inst);
    void InsertAfter(Inst *inst);

    /**
     * Return true if instruction has dynamic operands storage.
     */
    bool IsOperandsDynamic() const
    {
        return GetField<InputsCount>() == MAX_STATIC_INPUTS;
    }

    /**
     * Add user to the instruction.
     * @param user - pointer to User object
     */
    void AddUser(User *user)
    {
        ASSERT(user && user->GetInst());
        user->SetNext(first_user_);
        user->SetPrev(nullptr);
        if (first_user_ != nullptr) {
            ASSERT(first_user_->GetPrev() == nullptr);
            first_user_->SetPrev(user);
        }
        first_user_ = user;
    }

    /**
     * Remove instruction from users.
     * @param user - pointer to User object
     */
    void RemoveUser(User *user)
    {
        ASSERT(user);
        ASSERT(HasUsers());
        if (user == first_user_) {
            first_user_ = user->GetNext();
        }
        user->Remove();
    }

    /**
     * Set input instruction in specified index.
     * Old input will be removed.
     * @param index - index of input to be set
     * @param inst - new input instruction TODO sherstennikov: currently it can be nullptr, is it correct?
     */
    void SetInput(unsigned index, Inst *inst)
    {
        CHECK_LT(index, GetInputsCount());
        auto &input = GetInputs()[index];
        auto user = GetUser(index);
        if (input.GetInst() != nullptr && input.GetInst()->HasUsers()) {
            input.GetInst()->RemoveUser(user);
        }
        if (inst != nullptr) {
            inst->AddUser(user);
        }
        input = Input(inst);
    }

    /**
     * Replace all inputs that points to specified instruction by new one.
     * @param old_input - instruction that should be replaced
     * @param new_input - new input instruction
     */
    void ReplaceInput(Inst *old_input, Inst *new_input)
    {
        unsigned index = 0;
        for (auto input : GetInputs()) {
            if (input.GetInst() == old_input) {
                SetInput(index, new_input);
            }
            index++;
        }
    }

    /**
     * Replace inputs that point to this instruction by given instruction.
     * @param inst - new input instruction
     */
    void ReplaceUsers(Inst *inst)
    {
        ASSERT(inst != this);
        ASSERT(inst != nullptr);
        for (auto it = GetUsers().begin(); it != GetUsers().end(); it = GetUsers().begin()) {
            it->GetInst()->SetInput(it->GetIndex(), inst);
        }
    }

    /**
     * Swap the operands of the instruction.
     * NB! Don't swap inputs while iterating over instruction's users:
     * for (auto user : instruction.GetUsers()) {
     *     // Don't do this!
     *     user.GetInst()->SwapInputs();
     * }
     */
    void SwapInputs()
    {
#ifndef NDEBUG
        constexpr auto INPUTS_COUNT_2 = 2;
#endif
        ASSERT(GetInputsCount() == INPUTS_COUNT_2);
        auto input0 = GetInput(0).GetInst();
        auto input1 = GetInput(1).GetInst();
        SetInput(0, input1);
        SetInput(1, input0);
    }

    /**
     * Append input instruction.
     * Available only for variadic inputs instructions, such as PHI.
     * @param input - input instruction
     * @return index in inputs container where new input is placed
     */
    unsigned AppendInput(Inst *input)
    {
        ASSERT(input != nullptr);
        ASSERT(IsOperandsDynamic());
        DynamicOperands *operands = GetDynamicOperands();
        return operands->Append(input);
    }

    unsigned AppendInput(Input input)
    {
        static_assert(sizeof(Input) <= sizeof(uintptr_t));  // Input become larger, so pass it by reference then
        return AppendInput(input.GetInst());
    }

    /**
     * Remove input from inputs container
     * Available only for variadic inputs instructions, such as PHI.
     * @param index - index of input in inputs container
     */
    virtual void RemoveInput(unsigned index)
    {
        ASSERT(IsOperandsDynamic());
        DynamicOperands *operands = GetDynamicOperands();
        ASSERT(index < operands->Size());
        operands->Remove(index);
    }

    /**
     * Remove all inputs
     */
    void RemoveInputs()
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            for (auto inputs_count = GetInputsCount(); inputs_count != 0; --inputs_count) {
                RemoveInput(inputs_count - 1);
            }
        } else {
            for (size_t i = 0; i < GetInputsCount(); ++i) {
                SetInput(i, nullptr);
            }
        }
    }

    /**
     * Remove all users
     */
    template <bool with_inputs = false>
    void RemoveUsers()
    {
        auto users = GetUsers();
        while (!users.Empty()) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (with_inputs) {
                auto &user = users.Front();
                user.GetInst()->RemoveInput(user.GetIndex());
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                RemoveUser(&users.Front());
            }
        }
    }

    /**
     * Get input by index
     * @param index - index of input
     * @return input instruction
     */
    Input GetInput(unsigned index)
    {
        ASSERT(index < GetInputsCount());
        return GetInputs()[index];
    }

    Input GetInput(unsigned index) const
    {
        ASSERT(index < GetInputsCount());
        return GetInputs()[index];
    }

    Span<Input> GetInputs()
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            DynamicOperands *operands = GetDynamicOperands();
            return Span<Input>(operands->Inputs(), operands->Size());
        }

        auto inputs_count {GetField<InputsCount>()};
        return Span<Input>(
            reinterpret_cast<Input *>(reinterpret_cast<uintptr_t>(this) -
                                      (inputs_count + Input::GetPadding(RUNTIME_ARCH, inputs_count)) * sizeof(Input)),
            inputs_count);
    }
    Span<const Input> GetInputs() const
    {
        return Span<const Input>(const_cast<Inst *>(this)->GetInputs());
    }

    virtual DataType::Type GetInputType([[maybe_unused]] size_t index) const
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    UserList<User> GetUsers()
    {
        return UserList<User>(&first_user_);
    }
    UserList<const User> GetUsers() const
    {
        return UserList<const User>(&first_user_);
    }

    size_t GetInputsCount() const
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return GetDynamicOperands()->Size();
        }
        return GetInputs().Size();
    }

    bool HasUsers() const
    {
        return first_user_ != nullptr;
    };

    bool HasSingleUser() const
    {
        return first_user_ != nullptr && first_user_->GetNext() == nullptr;
    }

    /// Reserve space in dataflow storage for specified inputs count
    void ReserveInputs(size_t capacity);

    virtual void SetLocation([[maybe_unused]] size_t index, [[maybe_unused]] Location location) {}

    virtual Location GetLocation([[maybe_unused]] size_t index) const
    {
        return Location::RequireRegister();
    }

    virtual Location GetDstLocation() const
    {
        return Location::MakeRegister(GetDstReg(), GetType());
    }

    virtual bool CanBeNull() const
    {
        ASSERT_PRINT(GetType() == DataType::Type::REFERENCE, "CanBeNull only applies to reference types");
        return true;
    }

    virtual uint32_t Latency() const
    {
        return options.GetCompilerSchedLatency();
    }

    template <typename Accessor>
    typename Accessor::ValueType GetField() const
    {
        return Accessor::Get(bit_fields_);
    }

    template <typename Accessor>
    void SetField(typename Accessor::ValueType value)
    {
        Accessor::Set(value, &bit_fields_);
    }

    uint64_t GetAllFields() const
    {
        return bit_fields_;
    }

    bool IsPhi() const
    {
        return opcode_ == Opcode::Phi;
    }

    bool IsCatchPhi() const
    {
        return opcode_ == Opcode::CatchPhi;
    }

    bool IsConst() const
    {
        return opcode_ == Opcode::Constant;
    }

    bool IsParameter() const
    {
        return opcode_ == Opcode::Parameter;
    }

    virtual bool IsBoolConst() const
    {
        return false;
    }

    bool IsSaveState() const
    {
        return opcode_ == Opcode::SaveState || opcode_ == Opcode::SafePoint || opcode_ == Opcode::SaveStateOsr ||
               opcode_ == Opcode::SaveStateDeoptimize;
    }

    bool IsClassInst() const
    {
        return opcode_ == Opcode::InitClass || opcode_ == Opcode::LoadClass || opcode_ == Opcode::LoadAndInitClass ||
               opcode_ == Opcode::UnresolvedLoadAndInitClass;
    }

    virtual size_t GetHashCode() const
    {
        // TODO (Aleksandr Popov) calculate hash code
        return 0;
    }

    virtual void SetVnObject([[maybe_unused]] VnObject *vn_obj) {}

    Register GetDstReg() const
    {
        return dst_reg_;
    }

    void SetDstReg(Register reg)
    {
        dst_reg_ = reg;
    }

    uint32_t GetVN() const
    {
        return vn_;
    }

    void SetVN(uint32_t vn)
    {
        vn_ = vn;
    }
    void Dump(std::ostream *out, bool new_line = true) const;
    virtual bool DumpInputs(std::ostream * /* out */) const;
    virtual void DumpOpcode(std::ostream * /* out */) const;

    virtual void SetDstReg([[maybe_unused]] unsigned index, Register reg)
    {
        ASSERT(index == 0);
        SetDstReg(reg);
    }

    virtual Register GetDstReg([[maybe_unused]] unsigned index) const
    {
        ASSERT(index == 0);
        return GetDstReg();
    }

    virtual size_t GetDstCount() const
    {
        return 1;
    }

    virtual uint32_t GetSrcRegIndex() const
    {
        return 0;
    }

    virtual void SetSrcReg([[maybe_unused]] unsigned index, [[maybe_unused]] Register reg) {}

    virtual Register GetSrcReg([[maybe_unused]] unsigned index) const
    {
        return INVALID_REG;
    }

    User *GetFirstUser() const
    {
        return first_user_;
    }

protected:
    using InstBase::InstBase;
    static constexpr int INPUT_COUNT = 0;

    Inst() = default;

    explicit Inst(Opcode opcode) : Inst(opcode, DataType::Type::NO_TYPE, INVALID_PC) {}

    explicit Inst(Opcode opcode, DataType::Type type, uint32_t pc) : pc_(pc), opcode_(opcode)
    {
        bit_fields_ = inst_flags::GetFlagsMask(opcode);
        SetField<FieldType>(type);
    }

protected:
    using FieldFlags = BitField<uint32_t, 0, MinimumBitsToStore(1U << inst_flags::FLAGS_COUNT)>;
    using FieldType = FieldFlags::NextField<DataType::Type, MinimumBitsToStore(DataType::LAST)>;
    using InputsCount = FieldType::NextField<uint32_t, BITS_PER_INPUTS_NUM>;
    using LastField = InputsCount;

    DynamicOperands *GetDynamicOperands() const
    {
        return reinterpret_cast<DynamicOperands *>(reinterpret_cast<uintptr_t>(this) - sizeof(DynamicOperands));
    }

private:
    User *GetUser(unsigned index)
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return GetDynamicOperands()->GetUser(index);
        }
        auto inputs_count {GetField<InputsCount>()};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<User *>(reinterpret_cast<Input *>(this) -
                                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                        (inputs_count + Input::GetPadding(RUNTIME_ARCH, inputs_count))) -
               // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               index - 1;
    }

    size_t OperandsStorageSize() const
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return sizeof(DynamicOperands);
        }

        auto inputs_count {GetField<InputsCount>()};
        return inputs_count * (sizeof(Input) + sizeof(User)) +
               Input::GetPadding(RUNTIME_ARCH, inputs_count) * sizeof(Input);
    }

private:
    /// Basic block this instruction belongs to
    BasicBlock *bb_ {nullptr};

    /// Next instruction within basic block
    Inst *next_ {nullptr};

    /// Previous instruction within basic block
    Inst *prev_ {nullptr};

    /// First user in users chain
    User *first_user_ {nullptr};

    /// This value hold properties of the instruction. It accessed via BitField types(f.e. FieldType).
    uint64_t bit_fields_ {0};

    /// Unique id of instruction
    uint32_t id_ {INVALID_ID};

    /// Unique id of instruction
    uint32_t vn_ {INVALID_VN};

    /// Bytecode pc
    uint32_t pc_ {INVALID_PC};

    /// Number used in cloning
    uint32_t clone_number_ {0};

    /// Instruction number getting while visiting graph
    LinearNumber linear_number_ {INVALID_LINEAR_NUM};

    ObjectTypeInfo object_type_info_ {};

    /// Opcode, see opcodes.def
    Opcode opcode_ {Opcode::INVALID};

    // Destination register type - defined in FieldType
    Register dst_reg_ {INVALID_REG};
};

/**
 * Proxy class that injects new field - type of the source operands - into property field of the instruction.
 * Should be used when instruction has sources of the same type and type of the instruction is not match to type of
 * sources. Examples: Cmp, Compare
 * @tparam T Base instruction class after which this mixin is injected
 */
template <typename T>
class InstWithOperandsType : public T {
public:
    using T::T;

    void SetOperandsType(DataType::Type type)
    {
        T::template SetField<FieldOperandsType>(type);
    }
    virtual DataType::Type GetOperandsType() const
    {
        return T::template GetField<FieldOperandsType>();
    }

protected:
    using FieldOperandsType =
        typename T::LastField::template NextField<DataType::Type, MinimumBitsToStore(DataType::LAST)>;
    using LastField = FieldOperandsType;
};

/**
 * Mixin for NeedBarrier flag.
 * @tparam T Base instruction class after which this mixin is injected
 */
template <typename T>
class NeedBarrierMixin : public T {
public:
    using T::T;

    void SetNeedBarrier(bool v)
    {
        T::template SetField<NeedBarrierFlag>(v);
    }
    bool GetNeedBarrier() const
    {
        return T::template GetField<NeedBarrierFlag>();
    }

protected:
    using NeedBarrierFlag = typename T::LastField::NextFlag;
    using LastField = NeedBarrierFlag;
};

/**
 * This mixin aims to implement type id accessors.
 */
class TypeIdMixin {
public:
    TypeIdMixin() = default;
    NO_COPY_SEMANTIC(TypeIdMixin);
    NO_MOVE_SEMANTIC(TypeIdMixin);
    virtual ~TypeIdMixin() = default;

    void SetTypeId(uint32_t id)
    {
        type_id_ = id;
    }

    auto GetTypeId() const
    {
        return type_id_;
    }

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }
    auto GetMethod() const
    {
        return method_;
    }

private:
    uint32_t type_id_ {0};
    // The pointer to the method in which this instruction is executed(inlined method)
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/**
 * This mixin aims to implement type of klass.
 */
template <typename T>
class ClassTypeMixin : public T {
public:
    using T::T;

    void SetClassType(ClassType class_type)
    {
        T::template SetField<ClassTypeField>(class_type);
    }

    ClassType GetClassType() const
    {
        return T::template GetField<ClassTypeField>();
    }

protected:
    using ClassTypeField = typename T::LastField::template NextField<ClassType, MinimumBitsToStore(ClassType::COUNT)>;
    using LastField = ClassTypeField;
};

/**
 * Mixin to check if null check inside CheckCast and IsInstance can be omitted.
 */
template <typename T>
class OmitNullCheckMixin : public T {
public:
    using T::T;

    void SetOmitNullCheck(bool omit_null_check)
    {
        T::template SetField<OmitNullCheckFlag>(omit_null_check);
    }

    bool GetOmitNullCheck() const
    {
        return T::template GetField<OmitNullCheckFlag>();
    }

protected:
    using OmitNullCheckFlag = typename T::LastField::NextFlag;
    using LastField = OmitNullCheckFlag;
};

template <typename T>
class ScaleMixin : public T {
public:
    using T::T;

    void SetScale(uint32_t scale)
    {
        ASSERT(scale <= MAX_SCALE);
        T::template SetField<ScaleField>(scale);
    }

    uint32_t GetScale() const
    {
        return T::template GetField<ScaleField>();
    }

protected:
    using ScaleField = typename T::LastField::template NextField<uint32_t, MinimumBitsToStore(MAX_SCALE)>;
    using LastField = ScaleField;
};

/**
 * This mixin aims to implement field accessors.
 */
class FieldMixin {
public:
    FieldMixin() = default;
    NO_COPY_SEMANTIC(FieldMixin);
    NO_MOVE_SEMANTIC(FieldMixin);
    virtual ~FieldMixin() = default;

    void SetObjField(RuntimeInterface::FieldPtr field)
    {
        field_ = field;
    }
    auto GetObjField() const
    {
        return field_;
    }

private:
    RuntimeInterface::FieldPtr field_ {nullptr};
};

/**
 * This mixin aims to implement volatile accessors.
 */
template <typename T>
class VolatileMixin : public T {
public:
    using T::T;

    void SetVolatile(bool is_volatile)
    {
        T::template SetField<IsVolatileFlag>(is_volatile);
    }
    bool GetVolatile() const
    {
        return T::template GetField<IsVolatileFlag>();
    }

protected:
    using IsVolatileFlag = typename T::LastField::NextFlag;
    using LastField = IsVolatileFlag;
};
/**
 * Mixin for Inlined calls/returns.
 */
template <typename T>
class InlinedInstMixin : public T {
public:
    using T::T;

    void SetInlined(bool v)
    {
        T::template SetField<IsInlinedFlag>(v);
    }
    bool IsInlined() const
    {
        return T::template GetField<IsInlinedFlag>();
    }

protected:
    using IsInlinedFlag = typename T::LastField::NextFlag;
    using LastField = IsInlinedFlag;
};

/**
 * Mixin for Array/String instruction
 */
template <typename T>
class ArrayInstMixin : public T {
public:
    using T::T;

    void SetIsArray(bool v)
    {
        T::template SetField<IsStringFlag>(!v);
    }

    void SetIsString(bool v)
    {
        T::template SetField<IsStringFlag>(v);
    }

    bool IsArray() const
    {
        return !(T::template GetField<IsStringFlag>());
    }

    bool IsString() const
    {
        return T::template GetField<IsStringFlag>();
    }

protected:
    using IsStringFlag = typename T::LastField::NextFlag;
    using LastField = IsStringFlag;
};

/**
 * Mixin for instructions with immediate constant value
 */
class ImmediateMixin {
public:
    explicit ImmediateMixin(uint64_t immediate) : immediate_(immediate) {}

    NO_COPY_SEMANTIC(ImmediateMixin);
    NO_MOVE_SEMANTIC(ImmediateMixin);
    virtual ~ImmediateMixin() = default;

    void SetImm(uint64_t immediate)
    {
        immediate_ = immediate;
    }
    auto GetImm() const
    {
        return immediate_;
    }

protected:
    ImmediateMixin() = default;

private:
    uint64_t immediate_ {0};
};

/**
 * Mixin for instructions with ConditionCode
 */
template <typename T>
class ConditionMixin : public T {
public:
    enum class Prediction { NONE, LIKELY, UNLIKELY, SIZE = UNLIKELY };

    using T::T;
    explicit ConditionMixin(ConditionCode cc)
    {
        T::template SetField<CcFlag>(cc);
    }
    NO_COPY_SEMANTIC(ConditionMixin);
    NO_MOVE_SEMANTIC(ConditionMixin);
    ~ConditionMixin() override = default;

    auto GetCc() const
    {
        return T::template GetField<CcFlag>();
    }
    void SetCc(ConditionCode cc)
    {
        T::template SetField<CcFlag>(cc);
    }
    void InverseConditionCode()
    {
        SetCc(GetInverseConditionCode(GetCc()));
        if (IsLikely()) {
            SetUnlikely();
        } else if (IsUnlikely()) {
            SetLikely();
        }
    }

    bool IsLikely() const
    {
        return T::template GetField<PredictionFlag>() == Prediction::LIKELY;
    }
    bool IsUnlikely() const
    {
        return T::template GetField<PredictionFlag>() == Prediction::UNLIKELY;
    }
    void SetLikely()
    {
        T::template SetField<PredictionFlag>(Prediction::LIKELY);
    }
    void SetUnlikely()
    {
        T::template SetField<PredictionFlag>(Prediction::UNLIKELY);
    }

protected:
    ConditionMixin() = default;

    using CcFlag = typename T::LastField::template NextField<ConditionCode, MinimumBitsToStore(ConditionCode::CC_LAST)>;
    using PredictionFlag = typename CcFlag::template NextField<Prediction, MinimumBitsToStore(Prediction::SIZE)>;
    using LastField = PredictionFlag;
};

/**
 * Mixin for instrucion with ShiftType
 */
class ShiftTypeMixin {
public:
    explicit ShiftTypeMixin(ShiftType shift_type) : shift_type_(shift_type) {}
    NO_COPY_SEMANTIC(ShiftTypeMixin);
    NO_MOVE_SEMANTIC(ShiftTypeMixin);
    virtual ~ShiftTypeMixin() = default;

    void SetShiftType(ShiftType shift_type)
    {
        shift_type_ = shift_type;
    }

    ShiftType GetShiftType() const
    {
        return shift_type_;
    }

protected:
    ShiftTypeMixin() = default;

private:
    ShiftType shift_type_ {INVALID_SHIFT};
};

/**
 * Mixin for instructions with multiple return values
 */
template <typename T, size_t N>
class MultipleOutputMixin : public T {
public:
    using T::T;

    Register GetDstReg(unsigned index) const override
    {
        ASSERT(index < N);
        if (index == 0) {
            return T::GetDstReg();
        }
        return dst_regs_[index - 1];
    }

    void SetDstReg(unsigned index, Register reg) override
    {
        ASSERT(index < N);
        if (index == 0) {
            T::SetDstReg(reg);
        } else {
            dst_regs_[index - 1] = reg;
        }
    }

    size_t GetDstCount() const override
    {
        return N;
    }

private:
    std::array<Register, N - 1> dst_regs_;
};

/**
 * Mixin for Deoptimize instructions
 */
template <typename T>
class DeoptimizeTypeMixin : public T {
public:
    using T::T;

    void SetDeoptimizeType(DeoptimizeType deopt_type)
    {
        T::template SetField<DeoptimizeTypeField>(deopt_type);
    }

    DeoptimizeType GetDeoptimizeType() const
    {
        return T::template GetField<DeoptimizeTypeField>();
    }

protected:
    using DeoptimizeTypeField =
        typename T::LastField::template NextField<DeoptimizeType, MinimumBitsToStore(DeoptimizeType::COUNT)>;
    using LastField = DeoptimizeTypeField;
};

/**
 * Instruction with fixed number of inputs.
 * Shall not be instantiated directly, only through derived classes.
 */
template <size_t N>
class FixedInputsInst : public Inst {
public:
    using Inst::Inst;

    static constexpr int INPUT_COUNT = N;

    void SetSrcReg(unsigned index, Register reg) override
    {
        ASSERT(index < N);
        src_regs_[index] = reg;
    }

    Register GetSrcReg(unsigned index) const override
    {
        ASSERT(index < N);
        return src_regs_[index];
    }

    Location GetLocation(size_t index) const override
    {
        return Location::MakeRegister(GetSrcReg(index), GetInputType(index));
    }

    void SetLocation(size_t index, Location location) override
    {
        SetSrcReg(index, location.GetValue());
    }

    void SetDstLocation(Location location)
    {
        SetDstReg(location.GetValue());
    }

    Inst *Clone(const Graph *targetGraph) const override;

private:
    template <typename T, std::size_t... Is>
    constexpr auto CreateArray(T value, [[maybe_unused]] std::index_sequence<Is...> unused)
    {
        return std::array<T, sizeof...(Is)> {(static_cast<void>(Is), value)...};
    }

private:
    std::array<Register, N> src_regs_ = CreateArray(INVALID_REG, std::make_index_sequence<INPUT_COUNT>());
};

/**
 * Instructions with fixed static inputs
 * We need to explicitly declare these proxy classes because some code can't work with the templated inst classes, for
 * example DEFINE_INST macro.
 */
class FixedInputsInst0 : public FixedInputsInst<0> {
public:
    DECLARE_INST(FixedInputsInst0);
    using FixedInputsInst::FixedInputsInst;

    NO_COPY_SEMANTIC(FixedInputsInst0);
    NO_MOVE_SEMANTIC(FixedInputsInst0);
    ~FixedInputsInst0() override = default;
};

class FixedInputsInst1 : public FixedInputsInst<1> {
public:
    DECLARE_INST(FixedInputsInst1);
    using FixedInputsInst::FixedInputsInst;

    NO_COPY_SEMANTIC(FixedInputsInst1);
    NO_MOVE_SEMANTIC(FixedInputsInst1);
    ~FixedInputsInst1() override = default;
};

class FixedInputsInst2 : public FixedInputsInst<2U> {
public:
    DECLARE_INST(FixedInputsInst2);
    using FixedInputsInst::FixedInputsInst;

    NO_COPY_SEMANTIC(FixedInputsInst2);
    NO_MOVE_SEMANTIC(FixedInputsInst2);
    ~FixedInputsInst2() override = default;
};

class FixedInputsInst3 : public FixedInputsInst<3U> {
public:
    DECLARE_INST(FixedInputsInst3);
    using FixedInputsInst::FixedInputsInst;
};

class FixedInputsInst4 : public FixedInputsInst<4U> {
public:
    DECLARE_INST(FixedInputsInst4);
    using FixedInputsInst::FixedInputsInst;
};

/**
 * Instruction with variable inputs count
 */
class DynamicInputsInst : public Inst {
public:
    DECLARE_INST(DynamicInputsInst);
    using Inst::Inst;

    static constexpr int INPUT_COUNT = MAX_STATIC_INPUTS;

    Location GetLocation(size_t index) const override
    {
        if (locations_ == nullptr) {
            return Location::Invalid();
        }
        return locations_->GetLocation(index);
    }

    Location GetDstLocation() const override
    {
        if (locations_ == nullptr) {
            return Location::Invalid();
        }
        return locations_->GetDstLocation();
    }

    void SetLocation(size_t index, Location location) override
    {
        ASSERT(locations_ != nullptr);
        locations_->SetLocation(index, location);
    }

    void SetDstLocation(Location location)
    {
        ASSERT(locations_ != nullptr);
        locations_->SetDstLocation(location);
    }

    void SetLocationsInfo(LocationsInfo *info)
    {
        locations_ = info;
    }

    Register GetSrcReg(unsigned index) const override
    {
        return GetLocation(index).GetValue();
    }

    void SetSrcReg(unsigned index, Register reg) override
    {
        SetLocation(index, Location::MakeRegister(reg, GetInputType(index)));
    }

private:
    LocationsInfo *locations_ {nullptr};
};

/**
 * Unary operation instruction
 */
class UnaryOperation : public FixedInputsInst<1> {
public:
    DECLARE_INST(UnaryOperation);
    using FixedInputsInst::FixedInputsInst;
    UnaryOperation(Opcode opcode, DataType::Type type, uint32_t pc, Inst *input) : FixedInputsInst(opcode, type, pc)
    {
        SetInput(0, input);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (GetOpcode() == Opcode::Cast) {
            return GetInput(0).GetInst()->GetType();
        }
        return GetType();
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    void SetVnObject(VnObject *vn_obj) override;

    Inst *Evaluate();
};

/**
 * Binary operation instruction
 */
class BinaryOperation : public FixedInputsInst<2U> {
public:
    DECLARE_INST(BinaryOperation);
    using FixedInputsInst::FixedInputsInst;

    uint32_t Latency() const override
    {
        if (GetOpcode() == Opcode::Div) {
            return options.GetCompilerSchedLatencyLong();
        }
        return options.GetCompilerSchedLatency();
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    Inst *Evaluate();
};

/**
 * Binary operation instruction with c immidiate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BinaryImmOperation : public FixedInputsInst<1>, public ImmediateMixin {
public:
    DECLARE_INST(BinaryImmOperation);
    using FixedInputsInst::FixedInputsInst;

    explicit BinaryImmOperation(Opcode opcode, uint64_t imm) : FixedInputsInst(opcode), ImmediateMixin(imm) {}
    explicit BinaryImmOperation(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm)
        : FixedInputsInst(opcode, type, pc), ImmediateMixin(imm)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    void SetVnObject(VnObject *vn_obj) override;
    bool DumpInputs(std::ostream * /* out */) const override;

    bool IsSafeInst() const override
    {
        return true;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<BinaryImmOperation *>(clone)->SetImm(GetImm());
        return clone;
    }
};

/**
 * Unary operation that shifts its own operand prior the application.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class UnaryShiftedRegisterOperation : public FixedInputsInst<1>, public ImmediateMixin, public ShiftTypeMixin {
public:
    DECLARE_INST(UnaryShiftedRegisterOperation);
    using FixedInputsInst::FixedInputsInst;

    explicit UnaryShiftedRegisterOperation(Opcode opcode, ShiftType shift_type, uint64_t imm)
        : FixedInputsInst(opcode), ImmediateMixin(imm), ShiftTypeMixin(shift_type)
    {
    }
    explicit UnaryShiftedRegisterOperation(Opcode opcode, DataType::Type type, uint32_t pc, ShiftType shift_type,
                                           uint64_t imm)
        : FixedInputsInst(opcode, type, pc), ImmediateMixin(imm), ShiftTypeMixin(shift_type)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    void SetVnObject(VnObject *vn_obj) override;
    bool DumpInputs(std::ostream * /* out */) const override;
    Inst *Clone(const Graph *targetGraph) const override;
};

/**
 * Binary operation that shifts its second operand prior the application.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BinaryShiftedRegisterOperation : public FixedInputsInst<2U>, public ImmediateMixin, public ShiftTypeMixin {
public:
    DECLARE_INST(BinaryShiftedRegisterOperation);
    using FixedInputsInst::FixedInputsInst;

    explicit BinaryShiftedRegisterOperation(Opcode opcode, ShiftType shift_type, uint64_t imm)
        : FixedInputsInst(opcode), ImmediateMixin(imm), ShiftTypeMixin(shift_type)
    {
    }
    explicit BinaryShiftedRegisterOperation(Opcode opcode, DataType::Type type, uint32_t pc, ShiftType shift_type,
                                            uint64_t imm)
        : FixedInputsInst(opcode, type, pc), ImmediateMixin(imm), ShiftTypeMixin(shift_type)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    void SetVnObject(VnObject *vn_obj) override;
    bool DumpInputs(std::ostream * /* out */) const override;
    Inst *Clone(const Graph *targetGraph) const override;
};

class SpillFillInst;

/**
 * Mixin to hold location data
 */
class LocationDataMixin {
public:
    void SetLocationData(SpillFillData location_data)
    {
        location_data_ = location_data;
    }

    auto GetLocationData() const
    {
        return location_data_;
    }

    auto &GetLocationData()
    {
        return location_data_;
    }

protected:
    LocationDataMixin() = default;
    NO_COPY_SEMANTIC(LocationDataMixin);
    NO_MOVE_SEMANTIC(LocationDataMixin);
    virtual ~LocationDataMixin() = default;

private:
    SpillFillData location_data_ {};
};

/**
 * Mixin to hold input types of call instruction
 */
class InputTypesMixin {
public:
    InputTypesMixin() = default;
    NO_COPY_SEMANTIC(InputTypesMixin);
    NO_MOVE_SEMANTIC(InputTypesMixin);
    virtual ~InputTypesMixin() = default;

    void AllocateInputTypes(ArenaAllocator *allocator, size_t capacity)
    {
        ASSERT(allocator != nullptr);
        ASSERT(input_types_ == nullptr);
        input_types_ = allocator->New<ArenaVector<DataType::Type>>(allocator->Adapter());
        ASSERT(input_types_ != nullptr);
        input_types_->reserve(capacity);
        ASSERT(input_types_->capacity() >= capacity);
    }
    void AddInputType(DataType::Type type)
    {
        ASSERT(input_types_ != nullptr);
        input_types_->push_back(type);
    }
    ArenaVector<DataType::Type> *GetInputTypes()
    {
        return input_types_;
    }
    void CloneTypes(ArenaAllocator *allocator, InputTypesMixin *target_inst) const
    {
        if (UNLIKELY(input_types_ == nullptr)) {
            return;
        }
        target_inst->AllocateInputTypes(allocator, input_types_->size());
        for (auto input_type : *input_types_) {
            target_inst->AddInputType(input_type);
        }
    }

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ArenaVector<DataType::Type> *input_types_ {nullptr};
};

/**
 * Call instruction
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CallInst : public InlinedInstMixin<DynamicInputsInst>, public InputTypesMixin {
    static constexpr uint32_t INVALID_METHOD_ID = std::numeric_limits<uint32_t>::max();

public:
    DECLARE_INST(CallInst);
    using Base = InlinedInstMixin<DynamicInputsInst>;
    using Base::Base;

    CallInst(Opcode opcode, DataType::Type type, uint32_t pc, uint32_t method_id)
        : Base(opcode, type, pc), method_id_(method_id)
    {
    }

    void SetCallMethodId(uint32_t id)
    {
        method_id_ = id;
    }
    auto GetCallMethodId() const
    {
        return method_id_;
    }
    void SetCallMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }
    RuntimeInterface::MethodPtr GetCallMethod() const
    {
        return method_;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(input_types_ != nullptr);
        ASSERT(index < input_types_->size());
        ASSERT(index < GetInputsCount());
        return (*input_types_)[index];
    }

    void DumpOpcode(std::ostream *out) const override;

    void SetCanNativeException(bool is_native)
    {
        SetField<IsNativeExceptionFlag>(is_native);
    }

    bool GetCanNativeException() const
    {
        return GetField<IsNativeExceptionFlag>();
    }

    Inst *Clone(const Graph *targetGraph) const override;

    bool IsRuntimeCall() const override
    {
        return !IsInlined();
    }

protected:
    using IsNativeExceptionFlag = LastField::NextFlag;
    using LastField = IsNativeExceptionFlag;

private:
    uint32_t method_id_ {INVALID_METHOD_ID};
    RuntimeInterface::MethodPtr method_ {nullptr};
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CallIndirectInst : public DynamicInputsInst, public InputTypesMixin {
public:
    DECLARE_INST(CallIndirectInst);
    using Base = DynamicInputsInst;
    using Base::Base;

    CallIndirectInst(Opcode opcode, DataType::Type type, uint32_t pc) : Base(opcode, type, pc) {}

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(input_types_ != nullptr);
        ASSERT(index < input_types_->size());
        ASSERT(index < GetInputsCount());
        return (*input_types_)[index];
    }

    Inst *Clone(const Graph *target_graph) const override;
};

/**
 * Length methods instruction
 */
class LengthMethodInst : public ArrayInstMixin<FixedInputsInst1> {
public:
    DECLARE_INST(LengthMethodInst);
    using Base = ArrayInstMixin<FixedInputsInst1>;
    using Base::Base;

    explicit LengthMethodInst(Opcode opcode, bool is_array = true) : Base(opcode)
    {
        SetIsArray(is_array);
    }
    LengthMethodInst(Opcode opcode, DataType::Type type, uint32_t pc, bool is_array = true) : Base(opcode, type, pc)
    {
        SetIsArray(is_array);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::REFERENCE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<LengthMethodInst *>(clone)->SetIsArray(IsArray());
        return clone;
    }
};

/**
 * Compare instruction
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CompareInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst2>> {
public:
    DECLARE_INST(CompareInst);
    using BaseInst = InstWithOperandsType<ConditionMixin<FixedInputsInst2>>;
    using BaseInst::BaseInst;

    CompareInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc) : BaseInst(opcode, type, pc)
    {
        SetCc(cc);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }
    void DumpOpcode(std::ostream * /* unused */) const override;

    void SetVnObject(VnObject *vn_obj) override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCompare()->SetCc(GetCc());
        clone->CastToCompare()->SetOperandsType(GetOperandsType());
        return clone;
    }
};

/**
 * Mixin for AnyTypeMixin instructions
 */
template <typename T>
class AnyTypeMixin : public T {
public:
    using T::T;

    void SetAnyType(AnyBaseType any_type)
    {
        T::template SetField<AnyBaseTypeField>(any_type);
    }

    AnyBaseType GetAnyType() const
    {
        return T::template GetField<AnyBaseTypeField>();
    }

protected:
    using AnyBaseTypeField =
        typename T::LastField::template NextField<AnyBaseType, MinimumBitsToStore(AnyBaseType::COUNT)>;
    using LastField = AnyBaseTypeField;
};

/**
 * CompareAnyTypeInst instruction
 */
class CompareAnyTypeInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    DECLARE_INST(CompareAnyTypeInst);
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CompareAnyTypeInst(Opcode opcode, uint32_t pc, AnyBaseType any_type) : BaseInst(opcode, DataType::Type::BOOL, pc)
    {
        SetAnyType(any_type);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCompareAnyType()->SetAnyType(GetAnyType());
        return clone;
    }
};

/**
 * CastAnyTypeValueInst instruction
 */
class CastAnyTypeValueInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    DECLARE_INST(CastAnyTypeValueInst);
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CastAnyTypeValueInst(Opcode opcode, uint32_t pc, AnyBaseType any_type)
        : BaseInst(opcode, AnyBaseTypeToDataType(any_type), pc)
    {
        SetAnyType(any_type);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    DataType::Type GetDeducedType() const
    {
        return AnyBaseTypeToDataType(GetAnyType());
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToCastAnyTypeValue();
        AnyBaseType any_type = GetAnyType();
        clone->SetAnyType(any_type);
        clone->SetType(GetType());
        return clone;
    }
};

/**
 * CastValueToAnyTypeInst instruction
 */
class CastValueToAnyTypeInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    DECLARE_INST(CastValueToAnyTypeInst);
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CastValueToAnyTypeInst(Opcode opcode, uint32_t pc) : BaseInst(opcode, DataType::ANY, pc) {}

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToCastValueToAnyType();
        auto any_type = GetAnyType();
        clone->SetAnyType(any_type);
        clone->SetType(GetType());
        return clone;
    }
};

/**
 * AnyTypeCheckInst instruction
 */
class AnyTypeCheckInst : public AnyTypeMixin<FixedInputsInst2> {
public:
    DECLARE_INST(AnyTypeCheckInst);
    using BaseInst = AnyTypeMixin<FixedInputsInst2>;
    using BaseInst::BaseInst;

    AnyTypeCheckInst(Opcode opcode, uint32_t pc, AnyBaseType any_type) : BaseInst(opcode, DataType::ANY, pc)
    {
        SetAnyType(any_type);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return (index == 0) ? DataType::ANY : DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToAnyTypeCheck()->SetAnyType(GetAnyType());
        return clone;
    }
};

/**
 * ConstantInst represent constant value.
 *
 * Available types: INT64, FLOAT32, FLOAT64, ANY. All integer types are stored as INT64 value.
 * Once type of constant is set, it can't be changed anymore.
 */
class ConstantInst : public Inst {
public:
    DECLARE_INST(ConstantInst);
    using Inst::Inst;

    template <typename T>
    explicit ConstantInst(Opcode /* unused */, T value, bool support_int32 = false) : Inst(Opcode::Constant)
    {
        ASSERT(GetTypeFromCType<T>() != DataType::NO_TYPE);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
        if constexpr (GetTypeFromCType<T>() == DataType::FLOAT64) {
            value_ = bit_cast<uint64_t, double>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (GetTypeFromCType<T>() == DataType::FLOAT32) {
            value_ = bit_cast<uint32_t, float>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (GetTypeFromCType<T>() == DataType::ANY) {
            value_ = value.Raw();
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if (GetTypeFromCType<T>(support_int32) == DataType::INT32) {
            value_ = static_cast<int32_t>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else {
            value_ = value;
        }

        SetType(GetTypeFromCType<T>(support_int32));
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    uint64_t GetRawValue() const
    {
        return value_;
    }

    uint32_t GetInt32Value() const
    {
        ASSERT(GetType() == DataType::INT32);
        return static_cast<uint32_t>(value_);
    }

    uint64_t GetInt64Value() const
    {
        ASSERT(GetType() == DataType::INT64);
        return value_;
    }

    uint64_t GetIntValue() const
    {
        ASSERT(GetType() == DataType::INT64 || GetType() == DataType::INT32);
        return value_;
    }

    float GetFloatValue() const
    {
        ASSERT(GetType() == DataType::FLOAT32);
        return bit_cast<float, uint32_t>(static_cast<uint32_t>(value_));
    }

    double GetDoubleValue() const
    {
        ASSERT(GetType() == DataType::FLOAT64);
        return bit_cast<double, uint64_t>(value_);
    }

    ConstantInst *GetNextConst()
    {
        return next_const_;
    }
    void SetNextConst(ConstantInst *next_const)
    {
        next_const_ = next_const;
    }

    template <typename T>
    static constexpr DataType::Type GetTypeFromCType(bool support_int32 = false)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
        if constexpr (std::is_integral_v<T>) {
            if (support_int32 && sizeof(T) == sizeof(uint32_t)) {
                return DataType::INT32;
            }
            return DataType::INT64;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, float>) {
            return DataType::FLOAT32;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, double>) {
            return DataType::FLOAT64;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, DataType::Any>) {
            return DataType::ANY;
        }
        return DataType::NO_TYPE;
    }

    inline bool IsEqualConst(double value, [[maybe_unused]] bool support_int32 = false)
    {
        return IsEqualConst(DataType::FLOAT64, bit_cast<uint64_t, double>(value));
    }
    inline bool IsEqualConst(float value, [[maybe_unused]] bool support_int32 = false)
    {
        return IsEqualConst(DataType::FLOAT32, bit_cast<uint32_t, float>(value));
    }
    inline bool IsEqualConst(DataType::Any value, [[maybe_unused]] bool support_int32 = false)
    {
        return IsEqualConst(DataType::ANY, value.Raw());
    }
    inline bool IsEqualConst(DataType::Type type, uint64_t value)
    {
        return GetType() == type && value_ == value;
    }
    template <typename T>
    inline bool IsEqualConst(T value, bool support_int32 = false)
    {
        static_assert(GetTypeFromCType<T>() == DataType::INT64);
        if (support_int32 && sizeof(T) == sizeof(uint32_t)) {
            return (GetType() == DataType::INT32 && static_cast<int32_t>(value_) == static_cast<int32_t>(value));
        }
        return (GetType() == DataType::INT64 && value_ == static_cast<uint64_t>(value));
    }

    inline bool IsEqualConstAllTypes(int64_t value, bool support_int32 = false)
    {
        return IsEqualConst(value, support_int32) || IsEqualConst(static_cast<float>(value)) ||
               IsEqualConst(static_cast<double>(value));
    }

    bool IsBoolConst() const override
    {
        ASSERT(IsConst());
        return GetType() == DataType::INT64 && (GetIntValue() == 0 || GetIntValue() == 1);
    }

    void SetImmTableSlot(ImmTableSlot imm_slot)
    {
        imm_slot_ = imm_slot;
    }

    auto GetImmTableSlot() const
    {
        return imm_slot_;
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override;

private:
    uint64_t value_ {0};
    ConstantInst *next_const_ {nullptr};
    ImmTableSlot imm_slot_ {INVALID_IMM_TABLE_SLOT};
};

// Type describing the purpose of the SpillFillInst.
// RegAlloc may use this information to preserve correct order of several SpillFillInst
// instructions placed along each other in the graph.
enum SpillFillType {
    UNKNOWN,
    INPUT_FILL,
    CONNECT_SPLIT_SIBLINGS,
    SPLIT_MOVE,
};

class SpillFillInst : public FixedInputsInst0 {
public:
    DECLARE_INST(SpillFillInst);

    explicit SpillFillInst(ArenaAllocator *allocator, Opcode opcode)
        : FixedInputsInst0(opcode), spill_fills_(allocator->Adapter())
    {
    }

    void AddMove(Register src, Register dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeRegister(src, type), Location::MakeRegister(dst, type), type);
    }

    void AddSpill(Register src, StackSlot dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeRegister(src, type), Location::MakeStackSlot(dst), type);
    }

    void AddFill(StackSlot src, Register dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeStackSlot(src), Location::MakeRegister(dst, type), type);
    }

    void AddMemCopy(StackSlot src, StackSlot dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeStackSlot(src), Location::MakeStackSlot(dst), type);
    }

    void AddSpillFill(const SpillFillData &spill_fill)
    {
        spill_fills_.emplace_back(spill_fill);
    }

    void AddSpillFill(const Location &src, const Location &dst, DataType::Type type)
    {
        spill_fills_.emplace_back(SpillFillData {src.GetKind(), dst.GetKind(), src.GetValue(), dst.GetValue(), type});
    }

    const ArenaVector<SpillFillData> &GetSpillFills() const
    {
        return spill_fills_;
    }

    ArenaVector<SpillFillData> &GetSpillFills()
    {
        return spill_fills_;
    }

    const SpillFillData &GetSpillFill(size_t n) const
    {
        ASSERT(n < spill_fills_.size());
        return spill_fills_[n];
    }

    SpillFillData &GetSpillFill(size_t n)
    {
        ASSERT(n < spill_fills_.size());
        return spill_fills_[n];
    }

    void RemoveSpillFill(size_t n)
    {
        ASSERT(n < spill_fills_.size());
        spill_fills_.erase(spill_fills_.begin() + n);
    }

    // Get register number, holded by n-th spill-fill
    Register GetInputReg(size_t n) const
    {
        ASSERT(n < spill_fills_.size());
        ASSERT(spill_fills_[n].SrcType() == LocationType::REGISTER);
        return spill_fills_[n].SrcValue();
    }

    void ClearSpillFills()
    {
        spill_fills_.clear();
    }

    SpillFillType GetSpillFillType() const
    {
        return sf_type_;
    }

    void SetSpillFillType(SpillFillType type)
    {
        sf_type_ = type;
    }

    bool DumpInputs(std::ostream * /* out */) const override;

#ifndef NDEBUG
    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToSpillFill();
        for (auto spill_fill : spill_fills_) {
            clone->AddSpillFill(spill_fill);
        }
        return clone;
    }
#endif

private:
    ArenaVector<SpillFillData> spill_fills_;
    SpillFillType sf_type_ {UNKNOWN};
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ParameterInst : public Inst, public LocationDataMixin {
public:
    DECLARE_INST(ParameterInst);
    using Inst::Inst;

    explicit ParameterInst(Opcode /* unused */, uint16_t arg_number) : Inst(Opcode::Parameter), arg_number_(arg_number)
    {
    }
    uint16_t GetArgNumber() const
    {
        return arg_number_;
    }

    void SetArgNumber(uint16_t arg_number)
    {
        arg_number_ = arg_number;
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override;

private:
    uint16_t arg_number_ {0};
};

inline bool IsZeroConstant(const Inst *inst)
{
    return inst->IsConst() && inst->GetType() == DataType::INT64 && inst->CastToConstant()->GetIntValue() == 0;
}

inline bool IsZeroConstantOrNullPtr(const Inst *inst)
{
    return IsZeroConstant(inst) || inst->GetOpcode() == Opcode::NullPtr;
}

/**
 * Phi instruction
 */
class PhiInst : public AnyTypeMixin<DynamicInputsInst> {
public:
    DECLARE_INST(PhiInst);
    using BaseInst = AnyTypeMixin<DynamicInputsInst>;
    using BaseInst::BaseInst;
    /// Get basic block corresponding to given input index. Returned pointer to basic block, can't be nullptr
    BasicBlock *GetPhiInputBb(unsigned index);
    const BasicBlock *GetPhiInputBb(unsigned index) const
    {
        return (const_cast<PhiInst *>(this))->GetPhiInputBb(index);
    }

    uint32_t GetPhiInputBbNum(unsigned index) const
    {
        ASSERT(index < GetInputsCount());
        return GetDynamicOperands()->GetUser(index)->GetBbNum();
    }

    void SetPhiInputBbNum(unsigned index, uint32_t bb_num)
    {
        ASSERT(index < GetInputsCount());
        GetDynamicOperands()->GetUser(index)->SetBbNum(bb_num);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = DynamicInputsInst::Clone(targetGraph);
        clone->CastToPhi()->SetAnyType(GetAnyType());
        return clone;
    }

    AnyBaseType GetAssumedAnyType()
    {
        return GetAnyType();
    }

    void SetAssumedAnyType(AnyBaseType type)
    {
        SetAnyType(type);
    }

    /// Get input instruction corresponding to the given basic block, can't be null.
    Inst *GetPhiInput(BasicBlock *bb);
    Inst *GetPhiDataflowInput(BasicBlock *bb);
    bool DumpInputs(std::ostream * /* out */) const override;

    // Get index of the given block in phi inputs
    size_t GetPredBlockIndex(const BasicBlock *block) const;

protected:
    using FlagIsLive = LastField::NextFlag;
    using LastField = FlagIsLive;
};

/**
 * Immediate for SavaState:
 * value - constant value to be stored
 * vreg - virtual register number
 */
struct SaveStateImm {
    uint64_t value;
    uint16_t vreg;
    DataType::Type type;
    bool is_acc;
};

/**
 * Frame state saving instruction
 * Aims to save pbc registers before calling something that can raise exception
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SaveStateInst : public DynamicInputsInst {
public:
    DECLARE_INST(SaveStateInst);
    using DynamicInputsInst::DynamicInputsInst;

    bool DumpInputs(std::ostream *out) const override;

    void SetVirtualRegister(size_t index, VirtualRegister reg)
    {
        static_assert(sizeof(reg) <= sizeof(uintptr_t), "Consider passing the register by reference");
        ASSERT(index < GetInputsCount());
        GetDynamicOperands()->GetUser(index)->SetVirtualRegister(reg);
    }

    VirtualRegister GetVirtualRegister(size_t index) const
    {
        ASSERT(index < GetInputsCount());
        return GetDynamicOperands()->GetUser(index)->GetVirtualRegister();
    }

    bool Verify() const
    {
        for (size_t i {0}; i < GetInputsCount(); ++i) {
            if (static_cast<uint16_t>(GetVirtualRegister(i)) == VirtualRegister::INVALID) {
                return false;
            }
        }
        return true;
    }

    bool RemoveNumericInputs()
    {
        size_t idx = 0;
        size_t inputs_count = GetInputsCount();
        bool removed = false;
        while (idx < inputs_count) {
            auto input_inst = GetInput(idx).GetInst();
            if (DataType::IsTypeNumeric(input_inst->GetType())) {
                RemoveInput(idx);
                inputs_count--;
                removed = true;
            } else {
                idx++;
            }
        }
        return removed;
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::NO_TYPE;
    }
    auto GetMethod() const
    {
        return method_;
    }
    auto SetMethod(void *method)
    {
        method_ = method;
    }

    auto GetCallerInst() const
    {
        return caller_inst_;
    }
    auto SetCallerInst(CallInst *inst)
    {
        caller_inst_ = inst;
    }

    void AppendImmediate(uint64_t imm, uint16_t vreg, DataType::Type type, bool is_acc);

    const ArenaVector<SaveStateImm> *GetImmediates() const
    {
        return immediates_;
    }

    const SaveStateImm &GetImmediate(size_t index) const
    {
        ASSERT(immediates_ != nullptr && index < immediates_->size());
        return (*immediates_)[index];
    }

    void AllocateImmediates(ArenaAllocator *allocator, size_t size = 0);

    size_t GetImmediatesCount() const
    {
        if (immediates_ == nullptr) {
            return 0;
        }
        return immediates_->size();
    }

    void SetRootsRegMaskBit(size_t reg)
    {
        ASSERT(reg < roots_regs_mask_.size());
        roots_regs_mask_.set(reg);
    }

    void SetRootsStackMaskBit(size_t slot)
    {
        if (roots_stack_mask_ != nullptr) {
            roots_stack_mask_->SetBit(slot);
        }
    }

    ArenaBitVector *GetRootsStackMask()
    {
        return roots_stack_mask_;
    }

    auto &GetRootsRegsMask()
    {
        return roots_regs_mask_;
    }

    void CreateRootsStackMask(ArenaAllocator *allocator)
    {
        ASSERT(roots_stack_mask_ == nullptr);
        roots_stack_mask_ = allocator->New<ArenaBitVector>(allocator);
        roots_stack_mask_->Reset();
    }

    Inst *Clone(const Graph *targetGraph) const override;
#ifndef NDEBUG
    void SetInputsWereDeleted()
    {
        SetField<FlagInputsWereDeleted>(true);
    }

    bool GetInputsWereDeleted()
    {
        return GetField<FlagInputsWereDeleted>();
    }
#endif

protected:
#ifndef NDEBUG
    using FlagInputsWereDeleted = LastField::NextFlag;
    using LastField = FlagInputsWereDeleted;
#endif

private:
    ArenaVector<SaveStateImm> *immediates_ {nullptr};
    void *method_ {nullptr};
    /// If instruction is in the inlined graph, this variable points to the inliner's call instruction.
    CallInst *caller_inst_ {nullptr};
    ArenaBitVector *roots_stack_mask_ {nullptr};
    std::bitset<BITS_PER_UINT32> roots_regs_mask_ {0};
};

/**
 * Load value from array or string
 */
class LoadInst : public ArrayInstMixin<NeedBarrierMixin<FixedInputsInst2>> {
public:
    DECLARE_INST(LoadInst);
    using Base = ArrayInstMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    explicit LoadInst(Opcode opcode, bool is_array = true) : Base(opcode)
    {
        SetIsArray(is_array);
    }
    LoadInst(Opcode opcode, DataType::Type type, uint32_t pc, bool is_array = true) : Base(opcode, type, pc)
    {
        SetIsArray(is_array);
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = NeedBarrierMixin<FixedInputsInst2>::Clone(targetGraph);
        static_cast<LoadInst *>(clone)->SetIsArray(IsArray());
        return clone;
    }
};

class LoadCompressedStringCharInst : public FixedInputsInst3 {
public:
    DECLARE_INST(LoadCompressedStringCharInst);
    using Base = FixedInputsInst3;
    using Base::Base;

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetLength() const
    {
        return GetInput(2U).GetInst();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            case 2U:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadCompressedStringCharInstI : public FixedInputsInst2, public ImmediateMixin {
public:
    DECLARE_INST(LoadCompressedStringCharInstI);
    using Base = FixedInputsInst2;
    using Base::Base;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};
/**
 * Store value into array element
 */
class StoreInst : public NeedBarrierMixin<FixedInputsInst3> {
public:
    DECLARE_INST(StoreInst);
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetStoredValue()
    {
        return GetInput(2U).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            case 2U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }

    // StoreArray call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/**
 * Load value from array, using array index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadInstI : public VolatileMixin<ArrayInstMixin<NeedBarrierMixin<FixedInputsInst1>>>, public ImmediateMixin {
public:
    DECLARE_INST(LoadInstI);
    using Base = VolatileMixin<ArrayInstMixin<NeedBarrierMixin<FixedInputsInst1>>>;
    using Base::Base;

    LoadInstI(Opcode opcode, uint64_t imm, bool is_array = true) : Base(opcode), ImmediateMixin(imm)
    {
        SetIsArray(is_array);
    }
    LoadInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm, bool is_array = true)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
        SetIsArray(is_array);
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        return DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<LoadInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        clone->SetIsArray(IsArray());
        clone->SetVolatile(GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Load value from pointer with offset
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadMemInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>, public ImmediateMixin {
public:
    DECLARE_INST(LoadMemInstI);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    LoadMemInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}
    LoadMemInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
    }

    Inst *GetPointer()
    {
        return GetInput(0).GetInst();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        auto input_0_type = GetInput(0).GetInst()->GetType();
        ASSERT(input_0_type == DataType::POINTER || input_0_type == DataType::REFERENCE);
        return input_0_type;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<LoadInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        clone->SetVolatile(GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Store value into array element, using array index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>, public ImmediateMixin {
public:
    DECLARE_INST(StoreInstI);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    StoreInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}
    StoreInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetStoredValue()
    {
        return GetInput(1).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return GetType();
            default:
                UNREACHABLE();
        }
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<StoreInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        clone->SetVolatile(GetVolatile());
        return clone;
    }

    // StoreArrayI call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/**
 * Store value into pointer by offset
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreMemInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>, public ImmediateMixin {
public:
    DECLARE_INST(StoreMemInstI);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    StoreMemInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}
    StoreMemInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetPointer()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetStoredValue()
    {
        return GetInput(1).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0: {
                auto input_0_type = GetInput(0).GetInst()->GetType();
                ASSERT(input_0_type == DataType::POINTER || input_0_type == DataType::REFERENCE);
                return input_0_type;
            }
            case 1:
                return GetType();
            default:
                UNREACHABLE();
        }
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<StoreInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        clone->SetVolatile(GetVolatile());
        return clone;
    }
};

/**
 * Bounds check, using array index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BoundsCheckInstI : public ArrayInstMixin<FixedInputsInst<2U>>, public ImmediateMixin {
public:
    DECLARE_INST(BoundsCheckInstI);
    using Base = ArrayInstMixin<FixedInputsInst<2U>>;
    using Base::Base;

    BoundsCheckInstI(Opcode opcode, uint64_t imm, bool is_array = true) : Base(opcode), ImmediateMixin(imm)
    {
        SetIsArray(is_array);
    }

    BoundsCheckInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm, bool is_array = true)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
        SetIsArray(is_array);
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToBoundsCheckI()->SetImm(GetImm());
        clone->CastToBoundsCheckI()->SetIsArray(IsArray());
        return clone;
    }
};

/**
 * Bounds check instruction
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BoundsCheckInst : public ArrayInstMixin<FixedInputsInst<3U>> {
public:
    DECLARE_INST(BoundsCheckInst);
    using Base = ArrayInstMixin<FixedInputsInst<3U>>;
    using Base::Base;

    explicit BoundsCheckInst(Opcode opcode, bool is_array = true) : Base(opcode)
    {
        SetIsArray(is_array);
    }

    BoundsCheckInst(Opcode opcode, DataType::Type type, uint32_t pc, bool is_array = true) : Base(opcode, type, pc)
    {
        SetIsArray(is_array);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToBoundsCheck()->SetIsArray(IsArray());
        return clone;
    }
};

class NullCheckInst : public FixedInputsInst2 {
public:
    DECLARE_INST(NullCheckInst);
    using Base = FixedInputsInst2;
    using Base::Base;

    bool IsImplicit() const
    {
        return GetField<IsImplicitFlag>();
    }

    void SetImplicit(bool is_implicit = true)
    {
        SetField<IsImplicitFlag>(is_implicit);
    }

private:
    using IsImplicitFlag = LastField::NextFlag;
    using LastField = IsImplicitFlag;
};

/**
 * Return immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ReturnInstI : public FixedInputsInst<0>, public ImmediateMixin {
public:
    DECLARE_INST(ReturnInstI);
    using FixedInputsInst::FixedInputsInst;

    ReturnInstI(Opcode opcode, uint64_t imm) : FixedInputsInst(opcode), ImmediateMixin(imm) {}
    ReturnInstI(Opcode opcode, DataType::Type type, uint32_t pc, uint64_t imm)
        : FixedInputsInst(opcode, type, pc), ImmediateMixin(imm)
    {
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToReturnI()->SetImm(GetImm());
        return clone;
    }
};

class ReturnInlinedInst : public FixedInputsInst<1> {
public:
    DECLARE_INST(ReturnInlinedInst);
    using FixedInputsInst::FixedInputsInst;

    bool IsExtendedLiveness() const
    {
        return GetField<IsExtendedLivenessFlag>();
    }

    void SetExtendedLiveness(bool is_extened_liveness = true)
    {
        SetField<IsExtendedLivenessFlag>(is_extened_liveness);
    }

private:
    using IsExtendedLivenessFlag = LastField::NextFlag;
    using LastField = IsExtendedLivenessFlag;
};

/**
 * Monitor instruction
 */
class MonitorInst : public FixedInputsInst2 {
public:
    DECLARE_INST(MonitorInst);
    using Base = FixedInputsInst2;
    using Base::Base;

    bool IsExit() const
    {
        return GetField<Exit>();
    }

    bool IsEntry() const
    {
        return !GetField<Exit>();
    }

    void SetExit()
    {
        SetField<Exit>(true);
    }

    void SetEntry()
    {
        SetField<Exit>(false);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        return DataType::REFERENCE;
    }

    void DumpOpcode(std::ostream * /* unused */) const override;

protected:
    using Exit = LastField::NextFlag;
    using LastField = Exit;
};

#include "intrinsics_flags.inl"

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class IntrinsicInst : public InlinedInstMixin<DynamicInputsInst>, public InputTypesMixin {
public:
    DECLARE_INST(IntrinsicInst);
    using Base = InlinedInstMixin<DynamicInputsInst>;
    using Base::Base;
    using IntrinsicId = RuntimeInterface::IntrinsicId;

    IntrinsicInst(Opcode opcode, IntrinsicId intrinsic_id) : Base(opcode), intrinsic_id_(intrinsic_id)
    {
        AdjustFlags(intrinsic_id, this);
    }

    IntrinsicInst(Opcode opcode, DataType::Type type, uint32_t pc, IntrinsicId intrinsic_id)
        : Base(opcode, type, pc), intrinsic_id_(intrinsic_id)
    {
        AdjustFlags(intrinsic_id, this);
    }

    IntrinsicId GetIntrinsicId() const
    {
        return intrinsic_id_;
    }

    void SetIntrinsicId(IntrinsicId intrinsic_id)
    {
        intrinsic_id_ = intrinsic_id;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(input_types_ != nullptr);
        ASSERT(index < input_types_->size());
        ASSERT(index < GetInputsCount());
        return (*input_types_)[index];
    }

    const ArenaVector<uint32_t> &GetImms()
    {
        return *imms_;
    }

    const ArenaVector<uint32_t> &GetImms() const
    {
        return *imms_;
    }

    bool HasImms() const
    {
        return imms_ != nullptr;
    }

    void AddImm(ArenaAllocator *allocator, uint32_t imm)
    {
        if (imms_ == nullptr) {
            imms_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        }
        imms_->push_back(imm);
    }

    bool IsNativeCall() const;

    bool HasArgumentsOnStack() const
    {
        return GetField<ArgumentsOnStack>();
    }

    void SetArgumentsOnStack()
    {
        SetField<ArgumentsOnStack>(true);
    }

    Inst *Clone(const Graph *targetGraph) const override;

    bool CanBeInlined()
    {
        return IsInlined();
    }

    void SetRelocate()
    {
        SetField<Relocate>(true);
    }

    bool GetRelocate() const
    {
        return GetField<Relocate>();
    }

    void DumpOpcode(std::ostream *out) const override;

protected:
    using ArgumentsOnStack = LastField::NextFlag;
    using Relocate = ArgumentsOnStack::NextFlag;
    using LastField = Relocate;

private:
    IntrinsicId intrinsic_id_ {RuntimeInterface::IntrinsicId::COUNT};
    ArenaVector<uint32_t> *imms_ {nullptr};  // record imms appeared in intrinsics
};

#include <get_intrinsics_names.inl>
#include <intrinsics_enum.inl>
#include <can_encode_builtin.inl>

/**
 *  Cast instruction
 */
class CastInst : public InstWithOperandsType<FixedInputsInst1> {
public:
    DECLARE_INST(CastInst);
    using BaseInst = InstWithOperandsType<FixedInputsInst1>;
    using BaseInst::BaseInst;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        return GetOperandsType();
    }

    void SetVnObject(VnObject *vn_obj) override;

    void DumpOpcode(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCast()->SetOperandsType(GetOperandsType());
        return clone;
    }
};

/**
 * Cmp instruction
 */
class CmpInst : public InstWithOperandsType<FixedInputsInst2> {
public:
    DECLARE_INST(CmpInst);
    using BaseInst = InstWithOperandsType<FixedInputsInst2>;
    using BaseInst::BaseInst;

    bool IsFcmpg() const
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        return GetField<Fcmpg>();
    }
    bool IsFcmpl() const
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        return !GetField<Fcmpg>();
    }
    void SetFcmpg()
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(true);
    }
    void SetFcmpg(bool v)
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(v);
    }
    void SetFcmpl()
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(false);
    }
    void SetFcmpl(bool v)
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(!v);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    void SetVnObject(VnObject *vn_obj) override;

    void DumpOpcode(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCmp()->SetOperandsType(GetOperandsType());
        return clone;
    }

protected:
    using Fcmpg = LastField::NextFlag;
    using LastField = Fcmpg;
};

/**
 * Load value from instance field
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadObjectInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>, public TypeIdMixin, public FieldMixin {
public:
    DECLARE_INST(LoadObjectInst);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 1);
        return DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadObject()->SetTypeId(GetTypeId());
        clone->CastToLoadObject()->SetMethod(GetMethod());
        clone->CastToLoadObject()->SetObjField(GetObjField());
        clone->CastToLoadObject()->SetVolatile(GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Load value from memory by offset
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadMemInst : public ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>> {
public:
    DECLARE_INST(LoadMemInst);
    using Base = ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>>;
    using Base::Base;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 2U);
        if (index == 1U) {
            return DataType::UINT32;
        }

        ASSERT(index == 0U);
        auto input_0_type = GetInput(0).GetInst()->GetType();
        ASSERT(input_0_type == DataType::POINTER || input_0_type == DataType::REFERENCE);
        return input_0_type;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    void DumpOpcode(std::ostream *out) const override;
    bool DumpInputs(std::ostream * /* unused */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoad()->SetVolatile(GetVolatile());
        clone->CastToLoad()->SetScale(GetScale());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Load value from unresolved instance field
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class UnresolvedLoadObjectInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    DECLARE_INST(UnresolvedLoadObjectInst);
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 2U);
        if (index == 1) {
            // This is SaveState input
            return DataType::NO_TYPE;
        }
        ASSERT(index == 0);
        return DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return true;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToUnresolvedLoadObject()->SetTypeId(GetTypeId());
        clone->CastToUnresolvedLoadObject()->SetMethod(GetMethod());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Store value into instance field
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreObjectInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>,
                        public TypeIdMixin,
                        public FieldMixin {
public:
    DECLARE_INST(StoreObjectInst);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 2U);
        return index == 0 ? DataType::REFERENCE : GetType();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreObject()->SetTypeId(GetTypeId());
        clone->CastToStoreObject()->SetMethod(GetMethod());
        clone->CastToStoreObject()->SetObjField(GetObjField());
        clone->CastToStoreObject()->SetVolatile(GetVolatile());
        return clone;
    }

    // StoreObject call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/**
 * Store value into unresolved instance field
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class UnresolvedStoreObjectInst : public NeedBarrierMixin<FixedInputsInst3>, public TypeIdMixin {
public:
    DECLARE_INST(UnresolvedStoreObjectInst);
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return true;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index == 2U) {
            // This is SaveState input
            return DataType::NO_TYPE;
        }
        return index == 0 ? DataType::REFERENCE : GetType();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToUnresolvedStoreObject()->SetTypeId(GetTypeId());
        clone->CastToUnresolvedStoreObject()->SetMethod(GetMethod());
        return clone;
    }
};

/**
 * Store value in memory by offset
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreMemInst : public ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>> {
public:
    DECLARE_INST(StoreMemInst);
    using Base = ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    static constexpr size_t STORED_INPUT_INDEX = 2;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index == 1U) {
            return DataType::UINT32;
        }
        if (index == 2U) {
            return GetType();
        }

        ASSERT(index == 0U);
        auto input_0_type = GetInput(0).GetInst()->GetType();
        ASSERT(input_0_type == DataType::POINTER || input_0_type == DataType::REFERENCE);
        return input_0_type;
    }

    void DumpOpcode(std::ostream *out) const override;
    bool DumpInputs(std::ostream * /* unused */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStore()->SetVolatile(GetVolatile());
        clone->CastToStore()->SetScale(GetScale());
        return clone;
    }
};

/**
 * Load static field from class.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadStaticInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>, public TypeIdMixin, public FieldMixin {
public:
    DECLARE_INST(LoadStaticInst);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(index == 0);
        return DataType::REFERENCE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadStatic()->SetTypeId(GetTypeId());
        clone->CastToLoadStatic()->SetMethod(GetMethod());
        clone->CastToLoadStatic()->SetObjField(GetObjField());
        clone->CastToLoadStatic()->SetVolatile(GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Load unresolved static field from class.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class UnresolvedLoadStaticInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    DECLARE_INST(UnresolvedLoadStaticInst);
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return true;
    }

    void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        // This is SaveState input
        return DataType::NO_TYPE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToUnresolvedLoadStatic()->SetTypeId(GetTypeId());
        clone->CastToUnresolvedLoadStatic()->SetMethod(GetMethod());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Store value into static field.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreStaticInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>,
                        public TypeIdMixin,
                        public FieldMixin {
public:
    DECLARE_INST(StoreStaticInst);
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return GetType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreStatic()->SetTypeId(GetTypeId());
        clone->CastToStoreStatic()->SetMethod(GetMethod());
        clone->CastToStoreStatic()->SetObjField(GetObjField());
        clone->CastToStoreStatic()->SetVolatile(GetVolatile());
        return clone;
    }

    // StoreStatic call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/**
 * Store value into unresolved static field.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class UnresolvedStoreStaticInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    DECLARE_INST(UnresolvedStoreStaticInst);
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return true;
    }

    void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 1) {
            // This is SaveState input
            return DataType::NO_TYPE;
        }
        ASSERT(index == 0);
        return GetType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToUnresolvedStoreStatic()->SetTypeId(GetTypeId());
        clone->CastToUnresolvedStoreStatic()->SetMethod(GetMethod());
        return clone;
    }
};

/**
 * Create new object
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class NewObjectInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    DECLARE_INST(NewObjectInst);
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToNewObject()->SetTypeId(GetTypeId());
        clone->CastToNewObject()->SetMethod(GetMethod());
        return clone;
    }
};

/**
 * Create new array
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class NewArrayInst : public NeedBarrierMixin<FixedInputsInst3>, public TypeIdMixin {
public:
    DECLARE_INST(NewArrayInst);
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    static constexpr size_t INDEX_CLASS = 0;
    static constexpr size_t INDEX_SIZE = 1;
    static constexpr size_t INDEX_SAVE_STATE = 2;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case INDEX_CLASS:
                return GetInput(0).GetInst()->GetType();
            case INDEX_SIZE:
                return DataType::INT32;
            case INDEX_SAVE_STATE:
                // This is SaveState input
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToNewArray()->SetTypeId(GetTypeId());
        clone->CastToNewArray()->SetMethod(GetMethod());
        return clone;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadConstArrayInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    DECLARE_INST(LoadConstArrayInst);
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadConstArray()->SetTypeId(GetTypeId());
        clone->CastToLoadConstArray()->SetMethod(GetMethod());
        return clone;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class FillConstArrayInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin, public ImmediateMixin {
public:
    DECLARE_INST(FillConstArrayInst);
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return index == 0 ? DataType::REFERENCE : DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToFillConstArray()->SetTypeId(GetTypeId());
        clone->CastToFillConstArray()->SetMethod(GetMethod());
        clone->CastToFillConstArray()->SetImm(GetImm());
        return clone;
    }
};

/**
 * Checkcast
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CheckCastInst : public OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>,
                      public TypeIdMixin {
public:
    DECLARE_INST(CheckCastInst);
    using Base = OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index < 2U) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCheckCast()->SetTypeId(GetTypeId());
        clone->CastToCheckCast()->SetMethod(GetMethod());
        clone->CastToCheckCast()->SetClassType(GetClassType());
        clone->CastToCheckCast()->SetOmitNullCheck(GetOmitNullCheck());
        return clone;
    }
};

/**
 * Is instance
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class IsInstanceInst : public OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>,
                       public TypeIdMixin {
public:
    DECLARE_INST(IsInstanceInst);
    using Base = OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index < 2U) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToIsInstance()->SetTypeId(GetTypeId());
        clone->CastToIsInstance()->SetMethod(GetMethod());
        clone->CastToIsInstance()->SetClassType(GetClassType());
        clone->CastToIsInstance()->SetOmitNullCheck(GetOmitNullCheck());
        return clone;
    }
};

/**
 * Load data from constant pool.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadFromPool : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    DECLARE_INST(LoadFromPool);
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::NO_TYPE;
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<LoadFromPool *>(clone)->SetTypeId(GetTypeId());
        static_cast<LoadFromPool *>(clone)->SetMethod(GetMethod());
        return clone;
    }
};

/**
 * Initialization or loading of the class.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ClassInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    DECLARE_INST(ClassInst);
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<ClassInst *>(clone)->SetTypeId(GetTypeId());
        static_cast<ClassInst *>(clone)->SetMethod(GetMethod());
        static_cast<ClassInst *>(clone)->SetClass(GetClass());
        return clone;
    }

    RuntimeInterface::ClassPtr GetClass() const
    {
        return klass_;
    }

    void SetClass(RuntimeInterface::ClassPtr klass)
    {
        klass_ = klass;
    }

private:
    RuntimeInterface::ClassPtr klass_ {nullptr};
};

/**
 * Get class pointer from the specific source.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ClassImmediateInst : public Inst {
public:
    DECLARE_INST(ClassImmediateInst);
    using Base = Inst;
    using Base::Base;

    ClassImmediateInst(Opcode opcode, DataType::Type type, uint32_t pc, RuntimeInterface::ClassPtr cls)
        : Base(opcode, type, pc), class_(cls)
    {
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = Inst::Clone(targetGraph);
        clone->CastToClassImmediate()->class_ = class_;
        return clone;
    }

    RuntimeInterface::ClassPtr GetClassPtr() const
    {
        return class_;
    }

    void DumpOpcode(std::ostream * /* unused */) const override;

private:
    RuntimeInterface::ClassPtr class_ {nullptr};
};

/**
 * Select instruction
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SelectInst : public ConditionMixin<InstWithOperandsType<FixedInputsInst<4U>>> {
public:
    DECLARE_INST(SelectInst);
    using Base = ConditionMixin<InstWithOperandsType<FixedInputsInst<4U>>>;
    using Base::Base;

    SelectInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc) : Base(opcode, type, pc)
    {
        SetCc(cc);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index < 2U) {
            return GetType();
        }
        return GetOperandsType();
    }

    void DumpOpcode(std::ostream * /* unused */) const override;
    void SetVnObject(VnObject *vn_obj) override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToSelect()->SetCc(GetCc());
        clone->CastToSelect()->SetOperandsType(GetOperandsType());
        return clone;
    }
};

/**
 * SelectImm with comparison with immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SelectImmInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst3>>, public ImmediateMixin {
public:
    DECLARE_INST(SelectImmInst);
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst3>>;
    using Base::Base;

    SelectImmInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc, uint64_t imm)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
        SetCc(cc);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index < 2U) {
            return GetType();
        }
        return GetOperandsType();
    }

    void DumpOpcode(std::ostream * /* unused */) const override;
    bool DumpInputs(std::ostream * /* unused */) const override;
    void SetVnObject(VnObject *vn_obj) override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToSelectImm()->SetCc(GetCc());
        clone->CastToSelectImm()->SetImm(GetImm());
        clone->CastToSelectImm()->SetOperandsType(GetOperandsType());
        return clone;
    }
};

/**
 * Conditional jump instruction
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class IfInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst2>> {
public:
    DECLARE_INST(IfInst);
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst2>>;
    using Base::Base;

    IfInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc) : Base(opcode, type, pc)
    {
        SetCc(cc);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    void DumpOpcode(std::ostream * /* unused */) const override;

    void SetVnObject(VnObject *vn_obj) override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<IfInst *>(clone)->SetCc(GetCc());
        static_cast<IfInst *>(clone)->SetOperandsType(GetOperandsType());
        static_cast<IfInst *>(clone)->SetMethod(GetMethod());
        return clone;
    }

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        return method_;
    }

private:
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/**
 * IfImm instruction with immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class IfImmInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst1>>, public ImmediateMixin {
public:
    DECLARE_INST(IfImmInst);
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst1>>;
    using Base::Base;

    IfImmInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc, uint64_t imm)
        : Base(opcode, type, pc), ImmediateMixin(imm)
    {
        SetCc(cc);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    void DumpOpcode(std::ostream * /* unused */) const override;
    bool DumpInputs(std::ostream * /* unused */) const override;
    void SetVnObject(VnObject *vn_obj) override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToIfImm()->SetCc(GetCc());
        clone->CastToIfImm()->SetImm(GetImm());
        clone->CastToIfImm()->SetOperandsType(GetOperandsType());
        clone->CastToIfImm()->SetMethod(GetMethod());
        return clone;
    }

    BasicBlock *GetEdgeIfInputTrue();
    BasicBlock *GetEdgeIfInputFalse();

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        return method_;
    }

private:
    size_t GetTrueInputEdgeIdx();
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/**
 * Load element from a pair of values, using index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadPairPartInst : public FixedInputsInst1, public ImmediateMixin {
public:
    DECLARE_INST(LoadPairPartInst);
    using FixedInputsInst1::FixedInputsInst1;

    explicit LoadPairPartInst(Opcode opcode, uint64_t imm) : FixedInputsInst1(opcode), ImmediateMixin(imm) {}

    uint32_t GetSrcRegIndex() const override
    {
        return GetImm();
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadPairPart()->SetImm(GetImm());
        return clone;
    }

    uint32_t Latency() const override
    {
        return options.GetCompilerSchedLatencyLong();
    }
};

/**
 * Load a pair of consecutive values from array
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadArrayPairInst : public NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst2, 2U>> {
public:
    DECLARE_INST(LoadArrayPairInst);
    using Base = NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst2, 2U>>;
    using Base::Base;

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToLoadArrayPair();
#ifndef NDEBUG
        for (size_t i = 0; i < GetDstCount(); ++i) {
            clone->SetDstReg(i, GetDstReg(i));
        }
#endif
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return 0;
    }
};

/**
 * Store a pair of consecutive values to array
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreArrayPairInst : public NeedBarrierMixin<FixedInputsInst<4U>> {
public:
    DECLARE_INST(StoreVectorInst);
    using Base = NeedBarrierMixin<FixedInputsInst<4U>>;
    using Base::Base;

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetStoredValue(uint64_t index)
    {
        return GetInput(2U + index).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            case 2U:
            case 3U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }

    // StoreArrayPair call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }
};

/**
 * Load a pair of consecutive values from array, using array index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadArrayPairInstI : public NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>, public ImmediateMixin {
public:
    DECLARE_INST(LoadArrayPairInstI);
    using Base = NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>;
    using Base::Base;

    explicit LoadArrayPairInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }
    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToLoadArrayPairI();
        clone->SetImm(GetImm());
#ifndef NDEBUG
        for (size_t i = 0; i < GetDstCount(); ++i) {
            clone->SetDstReg(i, GetDstReg(i));
        }
#endif
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    uint32_t Latency() const override
    {
        return 0;
    }
};

/**
 * Store a pair of consecutive values to array, using array index as immediate
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class StoreArrayPairInstI : public NeedBarrierMixin<FixedInputsInst3>, public ImmediateMixin {
public:
    DECLARE_INST(StoreArrayPairInstI);
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    explicit StoreArrayPairInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetFirstValue()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetSecondValue()
    {
        return GetInput(2U).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
            case 2U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }

    // StoreArrayPairI call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    bool DumpInputs(std::ostream * /* out */) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreArrayPairI()->SetImm(GetImm());
        return clone;
    }
};

/**
 * CatchPhiInst instruction
 */
class CatchPhiInst : public DynamicInputsInst {
public:
    DECLARE_INST(CatchPhiInst);
    using DynamicInputsInst::DynamicInputsInst;

    const ArenaVector<const Inst *> *GetThrowableInsts() const
    {
        return throw_insts_;
    }

    const Inst *GetThrowableInst(size_t i) const
    {
        ASSERT(throw_insts_ != nullptr && i < throw_insts_->size());
        return throw_insts_->at(i);
    }

    void AppendThrowableInst(const Inst *inst);
    void ReplaceThrowableInst(const Inst *old_inst, const Inst *new_inst);
    void RemoveInput(unsigned index) override;

    bool IsAcc() const
    {
        return GetField<IsAccFlag>();
    }

    void SetIsAcc()
    {
        SetField<IsAccFlag>(true);
    }

protected:
    using IsAccFlag = LastField::NextFlag;
    using LastField = IsAccFlag;

private:
    size_t GetThrowableInstIndex(const Inst *inst)
    {
        ASSERT(throw_insts_ != nullptr);
        auto it = std::find(throw_insts_->begin(), throw_insts_->end(), inst);
        ASSERT(it != throw_insts_->end());
        return std::distance(throw_insts_->begin(), it);
    }

private:
    ArenaVector<const Inst *> *throw_insts_ {nullptr};
};

class TryInst : public FixedInputsInst0 {
public:
    DECLARE_INST(TryInst);
    using FixedInputsInst0::FixedInputsInst0;

    void AppendCatchTypeId(uint32_t id, uint32_t catch_edge_index);

    const ArenaVector<uint32_t> *GetCatchTypeIds() const
    {
        return catch_type_ids_;
    }

    const ArenaVector<uint32_t> *GetCatchEdgeIndexes() const
    {
        return catch_edge_indexes_;
    }

    size_t GetCatchTypeIdsCount() const
    {
        return (catch_type_ids_ == nullptr ? 0 : catch_type_ids_->size());
    }

    Inst *Clone(const Graph *targetGraph) const override;

    void SetTryEndBlock(BasicBlock *try_end_bb)
    {
        try_end_bb_ = try_end_bb;
    }

    BasicBlock *GetTryEndBlock() const
    {
        return try_end_bb_;
    }

private:
    ArenaVector<uint32_t> *catch_type_ids_ {nullptr};
    ArenaVector<uint32_t> *catch_edge_indexes_ {nullptr};
    BasicBlock *try_end_bb_ {nullptr};
};

TryInst *GetTryBeginInst(const BasicBlock *try_begin_bb);

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeInst : public DeoptimizeTypeMixin<FixedInputsInst1> {
public:
    DECLARE_INST(DeoptimizeInst);
    using Base = DeoptimizeTypeMixin<FixedInputsInst1>;
    using Base::Base;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToDeoptimize()->SetDeoptimizeType(GetDeoptimizeType());
        return clone;
    }

    void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeIfInst : public DeoptimizeTypeMixin<FixedInputsInst2> {
    DECLARE_INST(DeoptimizeInst);
    using Base = DeoptimizeTypeMixin<FixedInputsInst2>;

public:
    using Base::Base;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToDeoptimizeIf()->SetDeoptimizeType(GetDeoptimizeType());
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return GetInput(0).GetInst()->GetType();
            case 1:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeCompareInst : public InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst3>>> {
public:
    DECLARE_INST(DeoptimizeCompareInst);
    using Base = InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst3>>>;
    using Base::Base;

    explicit DeoptimizeCompareInst(Opcode opcode, const DeoptimizeIfInst *deoptIf, const CompareInst *compare)
        : Base(opcode, deoptIf->GetType(), deoptIf->GetPc())
    {
        SetDeoptimizeType(deoptIf->GetDeoptimizeType());
        SetOperandsType(compare->GetOperandsType());
        SetCc(compare->GetCc());
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst3::Clone(targetGraph);
        clone->CastToDeoptimizeCompare()->SetDeoptimizeType(GetDeoptimizeType());
        clone->CastToDeoptimizeCompare()->SetOperandsType(GetOperandsType());
        clone->CastToDeoptimizeCompare()->SetCc(GetCc());
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
            case 1:
                return GetInput(index).GetInst()->GetType();
            case 2U:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeCompareImmInst : public InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst2>>>,
                                 public ImmediateMixin {
public:
    DECLARE_INST(DeoptimizeCompareImmInst);
    using Base = InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst2>>>;
    using Base::Base;

    explicit DeoptimizeCompareImmInst(Opcode opcode, const DeoptimizeIfInst *deoptIf, const CompareInst *compare,
                                      uint64_t imm)
        : Base(opcode, deoptIf->GetType(), deoptIf->GetPc()), ImmediateMixin(imm)
    {
        SetDeoptimizeType(deoptIf->GetDeoptimizeType());
        SetOperandsType(compare->GetOperandsType());
        SetCc(compare->GetCc());
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst2::Clone(targetGraph);
        clone->CastToDeoptimizeCompareImm()->SetDeoptimizeType(GetDeoptimizeType());
        clone->CastToDeoptimizeCompareImm()->SetOperandsType(GetOperandsType());
        clone->CastToDeoptimizeCompareImm()->SetCc(GetCc());
        clone->CastToDeoptimizeCompareImm()->SetImm(GetImm());
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return GetInput(0).GetInst()->GetType();
            case 1:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    void DumpOpcode(std::ostream *out) const override;
    bool DumpInputs(std::ostream *out) const override;
};

class ThrowInst : public FixedInputsInst2 {
public:
    DECLARE_INST(ThrowInst);
    using Base = FixedInputsInst2;
    using Base::Base;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }
};

class BinaryOverflowInst : public IfInst {
public:
    DECLARE_INST(BinaryOverflowInst);
    using Base = IfInst;
    using Base::Base;

    BinaryOverflowInst(Opcode opcode, DataType::Type type, uint32_t pc, ConditionCode cc) : Base(opcode, type, pc, cc)
    {
        SetOperandsType(type);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    DataType::Type GetOperandsType() const override
    {
        return GetType();
    }
};

inline bool IsVolatileMemInst(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadObject:
            return inst->CastToLoadObject()->GetVolatile();
        case Opcode::StoreObject:
            return inst->CastToStoreObject()->GetVolatile();
        case Opcode::LoadStatic:
            return inst->CastToLoadStatic()->GetVolatile();
        case Opcode::StoreStatic:
            return inst->CastToStoreStatic()->GetVolatile();
        case Opcode::UnresolvedLoadObject:
        case Opcode::UnresolvedStoreObject:
        case Opcode::UnresolvedLoadStatic:
        case Opcode::UnresolvedStoreStatic:
            return true;
        default:
            return false;
    }
}

// Check if instruction is pseudo-user for mutli-output instruction
inline bool IsPseudoUserOfMultiOutput(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadPairPart:
            return true;
        default:
            return false;
    }
}

template <typename InstType, typename... Args>
InstType *Inst::New(ArenaAllocator *allocator, Args &&... args)
{
    static_assert(alignof(InstType) >= alignof(uintptr_t));
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (std::is_same_v<InstType, SpillFillInst>) {
        auto data = reinterpret_cast<uintptr_t>(allocator->Alloc(sizeof(InstType), DEFAULT_ALIGNMENT));
        ASSERT(data != 0);
        return new (reinterpret_cast<void *>(data)) InstType(allocator, std::forward<Args>(args)...);
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    } else if constexpr (InstType::INPUT_COUNT == 0) {
        auto data = reinterpret_cast<uintptr_t>(allocator->Alloc(sizeof(InstType), DEFAULT_ALIGNMENT));
        ASSERT(data != 0);
        return new (reinterpret_cast<void *>(data)) InstType(std::forward<Args>(args)...);
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    } else if constexpr (InstType::INPUT_COUNT == MAX_STATIC_INPUTS) {
        constexpr size_t OPERANDS_SIZE = sizeof(DynamicOperands);
        static_assert((OPERANDS_SIZE % alignof(InstType)) == 0);
        auto data = reinterpret_cast<uintptr_t>(allocator->Alloc(OPERANDS_SIZE + sizeof(InstType), DEFAULT_ALIGNMENT));
        ASSERT(data != 0);
        auto inst = new (reinterpret_cast<void *>(data + OPERANDS_SIZE)) InstType(std::forward<Args>(args)...);
        [[maybe_unused]] auto operands = new (reinterpret_cast<void *>(data)) DynamicOperands(allocator);
        static_cast<Inst *>(inst)->SetField<InputsCount>(InstType::INPUT_COUNT);
        return inst;
    } else {  // NOLINT(readability-misleading-indentation)
        constexpr size_t OPERANDS_SIZE = sizeof(Operands<InstType::INPUT_COUNT>);
        constexpr auto ALIGNMENT {GetLogAlignment(alignof(Operands<InstType::INPUT_COUNT>))};
        static_assert((OPERANDS_SIZE % alignof(InstType)) == 0);
        auto data = reinterpret_cast<uintptr_t>(allocator->Alloc(OPERANDS_SIZE + sizeof(InstType), ALIGNMENT));
        ASSERT(data != 0);
        auto inst = new (reinterpret_cast<void *>(data + OPERANDS_SIZE)) InstType(std::forward<Args>(args)...);
        auto operands = new (reinterpret_cast<void *>(data)) Operands<InstType::INPUT_COUNT>;
        static_cast<Inst *>(inst)->SetField<InputsCount>(InstType::INPUT_COUNT);
        unsigned idx = InstType::INPUT_COUNT - 1;
        for (auto &user : operands->users) {
            new (&user) User(true, idx--, InstType::INPUT_COUNT);
        }
        return inst;
    }
}

inline Inst *User::GetInput()
{
    return GetInst()->GetInput(GetIndex()).GetInst();
}

inline const Inst *User::GetInput() const
{
    return GetInst()->GetInput(GetIndex()).GetInst();
}

inline std::ostream &operator<<(std::ostream &os, const Inst &inst)
{
    inst.Dump(&os, false);
    return os;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, base, ...)                 \
    inline const base *Inst::CastTo##opcode() const \
    {                                               \
        ASSERT(GetOpcode() == Opcode::opcode);      \
        return static_cast<const base *>(this);     \
    }
OPCODE_LIST(INST_DEF)
#undef INST_DEF

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, base, ...)            \
    inline base *Inst::CastTo##opcode()        \
    {                                          \
        ASSERT(GetOpcode() == Opcode::opcode); \
        return static_cast<base *>(this);      \
    }
OPCODE_LIST(INST_DEF)
#undef INST_DEF
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_INST_H
