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

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_TRAITS_H
#define COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_TRAITS_H

#include "common_interfaces/objects/base_object.h"
#include <type_traits>
#include <vector>

namespace common_vm::objects_traits {

template <typename U>
constexpr bool IS_HEAP_OBJECT_V = std::is_base_of_v<BaseObject, std::remove_pointer_t<U>>;

// WriteBarrier: void (void*, size_t, U)
template <typename F>
constexpr bool IS_WRITE_BARRIER_CALLABLE_V = std::is_invocable_r_v<void, F, void *, size_t, BaseObject *>;

// ReadBarrier: U (void*, size_t)
template <typename F, typename U>
constexpr bool IS_READ_BARRIER_CALLABLE_V = IS_HEAP_OBJECT_V<U> &&std::is_invocable_v<F, void *, size_t>
    &&std::is_convertible_v<std::invoke_result_t<F, void *, size_t>, U>;

// Allocator: U (size_t, CommonType)
template <typename F, typename U>
constexpr bool IS_ALLOCATE_CALLABLE_V = IS_HEAP_OBJECT_V<U> &&std::is_invocable_r_v<U, F, size_t, ObjectType>;

// ---- enable_if_is_* traits ----
template <typename F>
using EnableIfIsWriteBarrier = std::enable_if_t<IS_WRITE_BARRIER_CALLABLE_V<F>, int>;

template <typename F, typename U>
using EnableIfIsReadBarrier = std::enable_if_t<IS_READ_BARRIER_CALLABLE_V<F, U>, int>;

template <typename F, typename U>
using EnableIfIsAllocate = std::enable_if_t<IS_ALLOCATE_CALLABLE_V<F, U>, int>;

template <typename Container, typename T>
struct IsStdVectorOf : std::false_type {
};

template <typename Alloc, typename T>
struct IsStdVectorOf<std::vector<T, Alloc>, T> : std::true_type {
};

template <typename Container, typename T>
constexpr bool IS_STD_VECTOR_OF_V = IsStdVectorOf<Container, T>::value;

template <typename Vec>
using GetAllocatorTypeT = typename std::decay_t<Vec>::allocator_type;

template <typename Alloc, typename U>
using RebindAllocT = typename std::allocator_traits<Alloc>::template rebind_alloc<U>;

template <typename Vec, typename NewT>
using VectorWithSameAllocT = std::vector<NewT, RebindAllocT<GetAllocatorTypeT<Vec>, NewT>>;

}  // namespace common_vm::objects_traits

#endif  // COMMON_INTERFACES_OBJECTS_TRAITS_H
