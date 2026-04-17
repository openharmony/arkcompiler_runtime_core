/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H
#define COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H

#include <cstddef>
#include <cstdint>
namespace common_vm {
using StateWordType = uint64_t;
using MAddress = uint64_t;
class TypeInfo;

enum class LanguageType : uint64_t {
    DYNAMIC = 0,
    STATIC = 1,
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

class BaseStateWord {
public:
#ifdef PANDA_32_BIT_MANAGED_POINTER
    static constexpr size_t BASECLASS_WIDTH = 32;
    static constexpr size_t PADDING_WIDTH = 28;
#else
    static constexpr size_t BASECLASS_WIDTH = 48;
    static constexpr size_t PADDING_WIDTH = 12;
#endif
    static constexpr size_t FORWARD_WIDTH = 2;
    static constexpr size_t LANGUAGE_WIDTH = 2;
    static constexpr uint64_t LOW_32_BITS_MASK = 0xFFFFFFFF;
    static constexpr uint64_t HIGH_32_BITS_MASK = ~LOW_32_BITS_MASK;

    BaseStateWord() = default;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit BaseStateWord(MAddress h) : header(h) {};

    enum class ForwardState : uint64_t { NORMAL, FORWARDING, FORWARDED, TO_VERSION };

    inline void SetForwarding()
    {
        state.forwardState = ForwardState::FORWARDING;
    }

    inline bool IsForwarding() const
    {
        return state.forwardState == ForwardState::FORWARDING;
    }

    inline bool IsToVersion() const
    {
        return state.forwardState == ForwardState::TO_VERSION;
    }

    bool TryLockBaseStateWord(const BaseStateWord current)
    {
        if (current.IsForwarding()) {
            return false;
        }
        auto newState = BaseStateWord(current.GetHeader());
        newState.SetForwardState(ForwardState::FORWARDING);
        return CompareExchangeHeader(current.GetHeader(), newState.GetHeader());
    }

    void UnlockStateWord(const ForwardState forwardState)
    {
        do {
            BaseStateWord current = AtomicGetBaseStateWord();
            auto newState = BaseStateWord(current.GetHeader());
            newState.SetForwardState(forwardState);
            if (CompareExchangeHeader(current.GetHeader(), newState.GetHeader())) {
                return;
            }
        } while (true);
    }

    void SetForwardState(ForwardState s)
    {
        state.forwardState = s;
    }

    ForwardState GetForwardState() const
    {
        return state.forwardState;
    }

    common_vm::StateWordType GetBaseClassAddress() const
    {
        return state.bascClass;
    }

    void SetFullBaseClassAddress(common_vm::StateWordType address)
    {
        state.bascClass = address;
    }

private:
    // Little endian
    struct State {
        StateWordType bascClass : BASECLASS_WIDTH;
        StateWordType padding : PADDING_WIDTH;
        LanguageType language : LANGUAGE_WIDTH;
        ForwardState forwardState : FORWARD_WIDTH;
    };

    union {
        State state;
        MAddress header;
    };

    BaseStateWord AtomicGetBaseStateWord() const
    {
        return BaseStateWord(AtomicGetHeader());
    }

    MAddress AtomicGetHeader() const
    {
        // Atomic with acquire order reason: data race with header with dependecies on reads after the load
        return __atomic_load_n(&header, __ATOMIC_ACQUIRE);
    }

    MAddress GetHeader() const
    {
        return header;
    }

    bool CompareExchangeHeader(MAddress expected, MAddress newState)
    {
#if defined(__x86_64__)
        // Atomic with acq_rel/acquire order reason: data race with header with acquire-release semantics on success
        // and acquire on failure
        bool success =
            __atomic_compare_exchange_n(&header, &expected, newState, true, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#else
        // Atomic with seq_cst/acquire order reason: data race with header with sequentially consistent order on
        // success and acquire on failure, due to "Spurious Failure" of compare_exchange_weak,
        // compare_exchange_strong is chosen.
        bool success =
            __atomic_compare_exchange_n(&header, &expected, newState, false, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
#endif
        return success;
    }

    inline void SetForwarded()
    {
        state.forwardState = ForwardState::FORWARDED;
    }

    inline bool IsForwarded() const
    {
        return state.forwardState == ForwardState::FORWARDED;
    }

    inline void SetLanguageType(LanguageType language)
    {
        state.language = language;
    }

    inline bool IsStatic() const
    {
        return state.language == LanguageType::STATIC;
    }

    inline bool IsDynamic() const
    {
        return state.language == LanguageType::DYNAMIC;
    }

    friend class BaseObject;
};
static_assert(sizeof(BaseStateWord) == sizeof(uint64_t), "Excepts 8 bytes");

// NOLINTEND(cppcoreguidelines-pro-type-union-access)

}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H
