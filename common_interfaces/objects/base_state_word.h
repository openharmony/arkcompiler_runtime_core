/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H
#define COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H

#include <stddef.h>
#include <cstdint>

namespace common {
using StateWordType = uint64_t;
using MAddress = uint64_t;
class TypeInfo;

enum class LanguageType : uint64_t {
    DYNAMIC = 0,
    STATIC = 1,
};

class BaseStateWord {
public:
    static constexpr size_t PADDING_WIDTH = 60;
    static constexpr size_t FORWARD_WIDTH = 2;
    static constexpr size_t LANGUAGE_WIDTH = 2;

    BaseStateWord() = default;
    BaseStateWord(MAddress header) : header_(header) {};

    enum class ForwardState : uint64_t {
        NORMAL,
        FORWARDING,
        FORWARDED,
        TO_VERSION
    };

    inline void SetForwarding()
    {
        state_.forwardState_ = ForwardState::FORWARDING;
    }

    inline bool IsForwarding() const
    {
        return state_.forwardState_ == ForwardState::FORWARDING;
    }

    inline bool IsToVersion() const
    {
        return state_.forwardState_ == ForwardState::TO_VERSION;
    }

    bool TryLockBaseStateWord(const BaseStateWord current)
    {
        if (current.IsForwarding()) {
            return false;
        }
        BaseStateWord newState = BaseStateWord(current.GetHeader());
        newState.SetForwardState(ForwardState::FORWARDING);
        return CompareExchangeHeader(current.GetHeader(), newState.GetHeader());
    }

    void UnlockStateWord(const ForwardState forwardState)
    {
        do {
            BaseStateWord current = AtomicGetBaseStateWord();
            BaseStateWord newState = BaseStateWord(current.GetHeader());
            newState.SetForwardState(forwardState);
            if (CompareExchangeHeader(current.GetHeader(), newState.GetHeader())) {
                return;
            }
        } while (true);
    }

    void SetForwardState(ForwardState state)
    {
        state_.forwardState_ = state;
    }

    ForwardState GetForwardState() const
    {
        return state_.forwardState_;
    }

    common::StateWordType GetBaseClassAddress() const
    {
        return state_.padding_;
    }

    void SetFullBaseClassAddress(common::StateWordType address)
    {
        state_.padding_ = address;
    }
private:
    // Little endian
    struct State {
        common::StateWordType padding_     : PADDING_WIDTH;
        LanguageType language_     : LANGUAGE_WIDTH;
        ForwardState forwardState_ : FORWARD_WIDTH;
    };

    union {
        State state_;
        MAddress header_;
    };

    BaseStateWord AtomicGetBaseStateWord() const
    {
        return BaseStateWord(AtomicGetHeader());
    }

    MAddress AtomicGetHeader() const
    {
        return __atomic_load_n(&header_, __ATOMIC_ACQUIRE);
    }

    MAddress GetHeader() const { return header_; }

    bool CompareExchangeHeader(MAddress expected, MAddress newState)
    {
#if defined(__x86_64__)
        bool success =
            __atomic_compare_exchange_n(&header_, &expected, newState, true, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#else
        // due to "Spurious Failure" of compare_exchange_weak, compare_exchange_strong is chosen.
        bool success =
            __atomic_compare_exchange_n(&header_, &expected, newState, false, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
#endif
        return success;
    }

    inline void SetForwarded()
    {
        state_.forwardState_ = ForwardState::FORWARDED;
    }

    inline bool IsForwarded() const
    {
        return state_.forwardState_ == ForwardState::FORWARDED;
    }

    inline void SetLanguageType(LanguageType language)
    {
        state_.language_ = language;
    }

    inline bool IsStatic() const
    {
        return state_.language_ == LanguageType::STATIC;
    }

    inline bool IsDynamic() const
    {
        return state_.language_ == LanguageType::DYNAMIC;
    }

    friend class BaseObject;
};
static_assert(sizeof(BaseStateWord) == sizeof(uint64_t), "Excepts 8 bytes");
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H
