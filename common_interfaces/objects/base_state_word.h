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

<<<<<<< HEAD
#include <stddef.h>
#include <cstdint>

namespace common {
=======
#include <cstddef>
#include <cstdint>

namespace panda {
>>>>>>> OpenHarmony_feature_20250328
using StateWordType = uint64_t;
using MAddress = uint64_t;
class TypeInfo;

<<<<<<< HEAD
enum class LanguageType : uint64_t {
=======
enum class Language : uint64_t {
>>>>>>> OpenHarmony_feature_20250328
    DYNAMIC = 0,
    STATIC = 1,
};

<<<<<<< HEAD
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
=======
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
class BaseStateWord {
public:
#ifdef BASE_CLASS_32BITS
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
    // CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, google-explicit-constructor)
    BaseStateWord(MAddress header) : header_(header) {};

    enum class ForwardState : uint64_t { NORMAL, FORWARDING, FORWARDED, TO_VERSION };

    inline void SetForwarding()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        state_.forwardState_ = ForwardState::FORWARDING;
    }

    inline bool IsForwarding() const
    {
<<<<<<< HEAD
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        return state_.forwardState_ == ForwardState::FORWARDING;
    }

    inline bool IsToVersion() const
    {
<<<<<<< HEAD
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        return state_.forwardState_ == ForwardState::TO_VERSION;
    }

    bool TryLockBaseStateWord(const BaseStateWord current)
    {
        if (current.IsForwarding()) {
            return false;
        }
<<<<<<< HEAD
=======
        // NOLINTNEXTLINE(modernize-use-auto)
>>>>>>> OpenHarmony_feature_20250328
        BaseStateWord newState = BaseStateWord(current.GetHeader());
        newState.SetForwardState(ForwardState::FORWARDING);
        return CompareExchangeHeader(current.GetHeader(), newState.GetHeader());
    }

    void UnlockStateWord(const ForwardState forwardState)
    {
        do {
            BaseStateWord current = AtomicGetBaseStateWord();
<<<<<<< HEAD
=======
            // NOLINTNEXTLINE(modernize-use-auto)
>>>>>>> OpenHarmony_feature_20250328
            BaseStateWord newState = BaseStateWord(current.GetHeader());
            newState.SetForwardState(forwardState);
            if (CompareExchangeHeader(current.GetHeader(), newState.GetHeader())) {
                return;
            }
        } while (true);
    }

    void SetForwardState(ForwardState state)
    {
<<<<<<< HEAD
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        state_.forwardState_ = state;
    }

    ForwardState GetForwardState() const
    {
<<<<<<< HEAD
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
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return state_.forwardState_;
    }

    StateWordType GetBaseClassAddress() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return state_.bascClass_;
    }

    void SetFullBaseClassAddress(StateWordType address)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        state_.bascClass_ = address;
    }
private:
    // Little endian
    // NOLINTBEGIN(readability-identifier-naming)
    struct State {
        StateWordType bascClass_   : BASECLASS_WIDTH;
        StateWordType padding_     : PADDING_WIDTH;
        Language language_         : LANGUAGE_WIDTH;
>>>>>>> OpenHarmony_feature_20250328
        ForwardState forwardState_ : FORWARD_WIDTH;
    };

    union {
        State state_;
        MAddress header_;
    };
<<<<<<< HEAD
=======
    // NOLINTEND(readability-identifier-naming)
>>>>>>> OpenHarmony_feature_20250328

    BaseStateWord AtomicGetBaseStateWord() const
    {
        return BaseStateWord(AtomicGetHeader());
    }

    MAddress AtomicGetHeader() const
    {
<<<<<<< HEAD
        return __atomic_load_n(&header_, __ATOMIC_ACQUIRE);
    }

    MAddress GetHeader() const { return header_; }
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return __atomic_load_n(&header_, __ATOMIC_ACQUIRE);
    }

    MAddress GetHeader() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return header_;
    }
>>>>>>> OpenHarmony_feature_20250328

    bool CompareExchangeHeader(MAddress expected, MAddress newState)
    {
#if defined(__x86_64__)
        bool success =
<<<<<<< HEAD
=======
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
            __atomic_compare_exchange_n(&header_, &expected, newState, true, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#else
        // due to "Spurious Failure" of compare_exchange_weak, compare_exchange_strong is chosen.
        bool success =
<<<<<<< HEAD
=======
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
            __atomic_compare_exchange_n(&header_, &expected, newState, false, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
#endif
        return success;
    }

    inline void SetForwarded()
    {
<<<<<<< HEAD
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        state_.forwardState_ = ForwardState::FORWARDED;
    }

    inline bool IsForwarded() const
    {
<<<<<<< HEAD
        return state_.forwardState_ == ForwardState::FORWARDED;
    }

    inline void SetLanguageType(LanguageType language)
    {
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return state_.forwardState_ == ForwardState::FORWARDED;
    }

    inline void SetLanguage(Language language)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
>>>>>>> OpenHarmony_feature_20250328
        state_.language_ = language;
    }

    inline bool IsStatic() const
    {
<<<<<<< HEAD
        return state_.language_ == LanguageType::STATIC;
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return state_.language_ == Language::STATIC;
>>>>>>> OpenHarmony_feature_20250328
    }

    inline bool IsDynamic() const
    {
<<<<<<< HEAD
        return state_.language_ == LanguageType::DYNAMIC;
=======
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return state_.language_ == Language::DYNAMIC;
>>>>>>> OpenHarmony_feature_20250328
    }

    friend class BaseObject;
};
static_assert(sizeof(BaseStateWord) == sizeof(uint64_t), "Excepts 8 bytes");
<<<<<<< HEAD
}  // namespace common
=======
}  // namespace panda
>>>>>>> OpenHarmony_feature_20250328
#endif  // COMMON_INTERFACES_OBJECTS_BASE_STATE_WORD_H
