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
#ifndef PANDA_RUNTIME_ARCH_HELPERS_H_
#define PANDA_RUNTIME_ARCH_HELPERS_H_

#include "libpandabase/utils/arch.h"
#include "libpandabase/utils/bit_utils.h"
#include "libpandabase/utils/span.h"

namespace panda::arch {

template <Arch A>
struct ExtArchTraits;

#if !defined(PANDA_TARGET_ARM32_ABI_HARD)
template <>
struct ExtArchTraits<Arch::AARCH32> {
    using signed_word_type = int32_t;
    using unsigned_word_type = uint32_t;

    static constexpr size_t NUM_GP_ARG_REGS = 4;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 0;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = 0;
    static constexpr bool HARDFP = false;
};
#else   // !defined(PANDA_TARGET_ARM32_ABI_HARD)
template <>
struct ExtArchTraits<Arch::AARCH32> {
    using signed_word_type = int32_t;
    using unsigned_word_type = uint32_t;

    static constexpr size_t NUM_GP_ARG_REGS = 4;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 16; /* s0 - s15 */
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};
#endif  // !defined(PANDA_TARGET_ARM32_ABI_HARD)

template <>
struct ExtArchTraits<Arch::AARCH64> {
    using signed_word_type = int64_t;
    using unsigned_word_type = uint64_t;

    static constexpr size_t NUM_GP_ARG_REGS = 8;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 8;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};

template <>
struct ExtArchTraits<Arch::X86_64> {
    using signed_word_type = int64_t;
    using unsigned_word_type = uint64_t;

    static constexpr size_t NUM_GP_ARG_REGS = 6;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 8;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};

template <class T>
inline uint8_t *AlignPtr(uint8_t *ptr)
{
    return reinterpret_cast<uint8_t *>(RoundUp(reinterpret_cast<uintptr_t>(ptr), sizeof(T)));
}

template <class T>
inline const uint8_t *AlignPtr(const uint8_t *ptr)
{
    return reinterpret_cast<const uint8_t *>(RoundUp(reinterpret_cast<uintptr_t>(ptr), sizeof(T)));
}

template <typename T>
typename std::enable_if<sizeof(T) < sizeof(uint32_t), uint8_t *>::type WriteToMem(T v, uint8_t *mem)
{
    /*
     * When the type is less than 4 bytes
     * We write 4 bytes to stack with 0 in high bytes
     * To avoid of unspecified behavior
     */
    static_assert(!std::is_floating_point<T>::value);
    ASSERT(reinterpret_cast<std::uintptr_t>(mem) % sizeof(std::uintptr_t) == 0);

    *reinterpret_cast<uint32_t *>(mem) = 0;
    mem = AlignPtr<T>(mem);
    *reinterpret_cast<T *>(mem) = v;

    return mem;
}

template <typename T>
typename std::enable_if<(sizeof(T) >= sizeof(uint32_t)), uint8_t *>::type WriteToMem(T v, uint8_t *mem)
{
    ASSERT(reinterpret_cast<std::uintptr_t>(mem) % sizeof(std::uintptr_t) == 0);

    mem = AlignPtr<T>(mem);
    *reinterpret_cast<T *>(mem) = v;
    return mem;
}

template <Arch A>
class ArgCounter {
public:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, void> Count()
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        fpr_arg_size_ = RoundUp(fpr_arg_size_, NUM_BYTES);
        if (fpr_arg_size_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            fpr_arg_size_ += NUM_BYTES;
        } else {
            stack_size_ = RoundUp(stack_size_, NUM_BYTES);
            stack_size_ += NUM_BYTES;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), void> Count()
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), PTR_SIZE);
        gpr_arg_size_ = RoundUp(gpr_arg_size_, NUM_BYTES);
        if (gpr_arg_size_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            gpr_arg_size_ += NUM_BYTES;
        } else {
            stack_size_ = RoundUp(stack_size_, NUM_BYTES);
            stack_size_ += NUM_BYTES;
        }
    }

    size_t GetStackSize() const
    {
        return GetStackSpaceSize() / ArchTraits<A>::POINTER_SIZE;
    }

    size_t GetStackSpaceSize() const
    {
        return RoundUp(ExtArchTraits<A>::FP_ARG_NUM_BYTES + ExtArchTraits<A>::GP_ARG_NUM_BYTES + stack_size_,
                       2 * ArchTraits<A>::POINTER_SIZE);
    }

private:
    static constexpr size_t PTR_SIZE = ArchTraits<A>::POINTER_SIZE;
    size_t gpr_arg_size_ = 0;
    size_t fpr_arg_size_ = 0;
    size_t stack_size_ = 0;
};

template <Arch A>
class ArgReader {
public:
    ArgReader(const Span<uint8_t> &gpr_args, const Span<uint8_t> &fpr_args, const uint8_t *stack_args)
        : gpr_args_(gpr_args), fpr_args_(fpr_args), stack_args_(stack_args)
    {
    }

    template <class T>
    ALWAYS_INLINE T Read()
    {
        return *ReadPtr<T>();
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        fp_arg_bytes_read_ = RoundUp(fp_arg_bytes_read_, READ_BYTES);
        if (fp_arg_bytes_read_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            const T *v = reinterpret_cast<const T *>(fpr_args_.data() + fp_arg_bytes_read_);
            fp_arg_bytes_read_ += READ_BYTES;
            return v;
        }
        stack_args_ = AlignPtr<T>(stack_args_);
        const T *v = reinterpret_cast<const T *>(stack_args_);
        stack_args_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = std::max(sizeof(T), PTR_SIZE);
        gp_arg_bytes_read_ = RoundUp(gp_arg_bytes_read_, READ_BYTES);
        if (gp_arg_bytes_read_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            const T *v = reinterpret_cast<const T *>(gpr_args_.data() + gp_arg_bytes_read_);
            gp_arg_bytes_read_ += READ_BYTES;
            return v;
        }
        stack_args_ = AlignPtr<T>(stack_args_);
        const T *v = reinterpret_cast<const T *>(stack_args_);
        stack_args_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

private:
    static constexpr size_t PTR_SIZE = ArchTraits<A>::POINTER_SIZE;
    const Span<uint8_t> &gpr_args_;
    const Span<uint8_t> &fpr_args_;
    const uint8_t *stack_args_;
    size_t gp_arg_bytes_read_ = 0;
    size_t fp_arg_bytes_read_ = 0;
};

template <Arch A, class T>
using ExtArchTraitsWorldType = std::conditional_t<std::is_signed_v<T>, typename ExtArchTraits<A>::signed_word_type,
                                                  typename ExtArchTraits<A>::unsigned_word_type>;

template <Arch A>
class ArgWriterBase {
public:
    ArgWriterBase(Span<uint8_t> *gpr_args, Span<uint8_t> *fpr_args, uint8_t *stack_args)
        : gpr_args_(gpr_args), fpr_args_(fpr_args), stack_args_(stack_args)
    {
    }
    ~ArgWriterBase() = default;

protected:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_integral_v<T> && sizeof(T) < ArchTraits<A>::POINTER_SIZE, void>
    RegisterValueWrite(T v)
    {
        *reinterpret_cast<ExtArchTraitsWorldType<A, T> *>(gpr_args_->data() + gp_arg_bytes_written_) = v;
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_integral_v<T> && sizeof(T) < ArchTraits<A>::POINTER_SIZE), void>
    RegisterValueWrite(T v)
    {
        *reinterpret_cast<T *>(gpr_args_->data() + gp_arg_bytes_written_) = v;
    }

    template <class T>
    void WriteNonFloatingPointValue(T v)
    {
        static_assert(!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP));

        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);
        gp_arg_bytes_written_ = RoundUp(gp_arg_bytes_written_, WRITE_BYTES);

        if (gp_arg_bytes_written_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            ArgWriterBase<A>::RegisterValueWrite(v);
            gp_arg_bytes_written_ += WRITE_BYTES;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            stack_args_ = WriteToMem(v, stack_args_) + WRITE_BYTES;
        }
    }

    NO_COPY_SEMANTIC(ArgWriterBase);
    NO_MOVE_SEMANTIC(ArgWriterBase);

    static constexpr size_t PTR_SIZE =
        ArchTraits<A>::POINTER_SIZE;   // NOLINT(misc-non-private-member-variables-in-classes)
    Span<uint8_t> *gpr_args_;          // NOLINT(misc-non-private-member-variables-in-classes)
    Span<uint8_t> *fpr_args_;          // NOLINT(misc-non-private-member-variables-in-classes)
    uint8_t *stack_args_;              // NOLINT(misc-non-private-member-variables-in-classes)
    size_t gp_arg_bytes_written_ = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t fp_arg_bytes_written_ = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
};

template <Arch A>
class ArgWriter : private ArgWriterBase<A> {
public:
    using ArgWriterBase<A>::gpr_args_;
    using ArgWriterBase<A>::fpr_args_;
    using ArgWriterBase<A>::stack_args_;
    using ArgWriterBase<A>::gp_arg_bytes_written_;
    using ArgWriterBase<A>::fp_arg_bytes_written_;
    using ArgWriterBase<A>::PTR_SIZE;

    // NOLINTNEXTLINE(readability-non-const-parameter)
    ArgWriter(Span<uint8_t> *gpr_args, Span<uint8_t> *fpr_args, uint8_t *stack_args)
        : ArgWriterBase<A>(gpr_args, fpr_args, stack_args)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, void> Write(T v)
    {
        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);

        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        if (fp_arg_bytes_written_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            *reinterpret_cast<T *>(fpr_args_->data() + fp_arg_bytes_written_) = v;
            fp_arg_bytes_written_ += NUM_BYTES;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            stack_args_ = WriteToMem(v, stack_args_) + WRITE_BYTES;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), void> Write(T v)
    {
        ArgWriterBase<A>::WriteNonFloatingPointValue(v);
    }

    NO_COPY_SEMANTIC(ArgWriter);
    NO_MOVE_SEMANTIC(ArgWriter);
};

// This class is required due to specific calling conventions in AARCH32
template <>
class ArgWriter<Arch::AARCH32> : private ArgWriterBase<Arch::AARCH32> {
public:
    using ArgWriterBase<Arch::AARCH32>::gpr_args_;
    using ArgWriterBase<Arch::AARCH32>::fpr_args_;
    using ArgWriterBase<Arch::AARCH32>::stack_args_;
    using ArgWriterBase<Arch::AARCH32>::gp_arg_bytes_written_;
    using ArgWriterBase<Arch::AARCH32>::fp_arg_bytes_written_;
    using ArgWriterBase<Arch::AARCH32>::PTR_SIZE;

    // NOLINTNEXTLINE(readability-non-const-parameter)
    ArgWriter(Span<uint8_t> *gpr_args, Span<uint8_t> *fpr_args, uint8_t *stack_args)
        : ArgWriterBase<Arch::AARCH32>(gpr_args, fpr_args, stack_args)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<Arch::AARCH32>::HARDFP, void>
    Write(T v)
    {
        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);

        if (fp_arg_bytes_written_ < ExtArchTraits<Arch::AARCH32>::FP_ARG_NUM_BYTES &&
            (std::is_same_v<T, float> ||
             (fp_arg_bytes_written_ < ExtArchTraits<Arch::AARCH32>::FP_ARG_NUM_BYTES - sizeof(float))) &&
            !is_float_arm_stack_has_been_written_) {
            RegisterFloatingPointValueWriteArm32(v);
            return;
        }

        is_float_arm_stack_has_been_written_ = true;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack_args_ = WriteToMem(v, stack_args_) + WRITE_BYTES;
    }

    template <class T>
    ALWAYS_INLINE
        typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<Arch::AARCH32>::HARDFP), void>
        Write(T v)
    {
        ArgWriterBase<Arch::AARCH32>::WriteNonFloatingPointValue(v);
    }

    NO_COPY_SEMANTIC(ArgWriter);
    NO_MOVE_SEMANTIC(ArgWriter);

private:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<(std::is_same_v<T, float>), void> RegisterFloatingPointValueWriteArm32(T v)
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<Arch::AARCH32>::FPR_SIZE);
        if (half_empty_register_offset_ == 0) {
            half_empty_register_offset_ = fp_arg_bytes_written_ + sizeof(float);
            *reinterpret_cast<T *>(fpr_args_->data() + fp_arg_bytes_written_) = v;
            fp_arg_bytes_written_ += NUM_BYTES;
        } else {
            *reinterpret_cast<T *>(fpr_args_->data() + half_empty_register_offset_) = v;
            if (half_empty_register_offset_ == fp_arg_bytes_written_) {
                fp_arg_bytes_written_ += NUM_BYTES;
            }
            half_empty_register_offset_ = 0;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_same_v<T, float>), void> RegisterFloatingPointValueWriteArm32(T v)
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<Arch::AARCH32>::FPR_SIZE);
        fp_arg_bytes_written_ = RoundUp(fp_arg_bytes_written_, sizeof(T));
        *reinterpret_cast<T *>(fpr_args_->data() + fp_arg_bytes_written_) = v;
        fp_arg_bytes_written_ += NUM_BYTES;
    }

    size_t half_empty_register_offset_ = 0;
    bool is_float_arm_stack_has_been_written_ = false;
};

}  // namespace panda::arch

#endif  // PANDA_RUNTIME_ARCH_HELPERS_H_
