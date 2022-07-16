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

#ifndef PANDA_VERIFIER_UTIL_STRUCT_FIELD_H__
#define PANDA_VERIFIER_UTIL_STRUCT_FIELD_H__

#include "macros.h"

#include <functional>
// included for nullopt_t
#include <optional>

namespace panda::verifier {

template <typename T>
struct OptionalRef;

template <typename T>
struct OptionalConstRef {
public:
    OptionalConstRef() : value {nullptr} {}
    // These are intentionally implicit for consistency with std::optional
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(std::nullopt_t) : value {nullptr} {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(const T &reference) : value {const_cast<T *>(&reference)} {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(std::reference_wrapper<const T> reference) : OptionalConstRef(reference.get()) {}
    explicit OptionalConstRef(const T *ptr) : value {const_cast<T *>(ptr)} {}

    bool HasRef() const
    {
        return value != nullptr;
    }

    const T &Get() const
    {
        ASSERT(HasRef());
        return *value;
    }

    const T &operator*() const
    {
        return Get();
    }

    const T *AsPointer() const
    {
        return &Get();
    }

    const T *operator->() const
    {
        return AsPointer();
    }

    std::reference_wrapper<const T> AsWrapper() const
    {
        return Get();
    }

    /**
     * @brief The unsafe direction of const_cast, though without UB. Try to avoid using.
     */
    OptionalRef<T> Unconst() const
    {
        return OptionalRef<T> {value};
    }

    DEFAULT_COPY_SEMANTIC(OptionalConstRef);
    DEFAULT_MOVE_SEMANTIC(OptionalConstRef);

protected:
    explicit OptionalConstRef(T *ptr) : value {ptr} {}
    // not const to let OptionalRef inherit this without unsafe behavior
    T *value;
};

/**
 * @brief Morally equivalent to std::optional<std::reference_wrapper<T>>, but nicer to use and takes less memory
 *
 * @tparam T The referenced type
 */
template <typename T>
struct OptionalRef : public OptionalConstRef<T> {
public:
    using OptionalConstRef<T>::OptionalConstRef;
    using OptionalConstRef<T>::Get;
    using OptionalConstRef<T>::AsPointer;
    using OptionalConstRef<T>::AsWrapper;
    using OptionalConstRef<T>::operator->;
    using OptionalConstRef<T>::operator*;
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalRef(T &reference) : OptionalConstRef<T>(&reference) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalRef(std::reference_wrapper<T> reference) : OptionalRef(reference.get()) {}

    T &Get()
    {
        ASSERT(OptionalConstRef<T>::HasRef());
        return *OptionalConstRef<T>::value;
    }

    T &operator*()
    {
        return Get();
    }

    T *AsPointer()
    {
        return &Get();
    }

    T *operator->()
    {
        return AsPointer();
    }

    std::reference_wrapper<T> AsWrapper()
    {
        return Get();
    }

    DEFAULT_COPY_SEMANTIC(OptionalRef);
    DEFAULT_MOVE_SEMANTIC(OptionalRef);

private:
    // makes the inherited protected constructor public
    friend struct OptionalConstRef<T>;
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_UTIL_STRUCT_FIELD_H__
