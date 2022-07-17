/*
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

#include <utility>

#include "type_system.h"
#include "type_sort.h"
#include "type_image.h"
#include "type_params.h"
#include "type_tags.h"
#include "type_systems.h"

#include "verification/util/enum_array.h"
#include "verification/util/hash.h"

#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"

#include "macros.h"

namespace panda::verifier {

class FullTypeSystem {
public:
    explicit FullTypeSystem(TypeSystemKind kind, ThreadNum threadnum)
        : sort_ {"Bot", "Top"},
          type_image_ {sort_},
          bot_sort_ {sort_["Bot"]},
          top_sort_ {sort_["Top"]},
          type_system_ {bot_sort_, top_sort_, threadnum, kind}
    {
    }
    NO_COPY_SEMANTIC(FullTypeSystem);
    // can't use the default move constructor due to an internal pointer in type_image_
    FullTypeSystem(FullTypeSystem &&other)
        : sort_ {std::move(other.sort_)},
          type_image_ {sort_},
          bot_sort_ {std::exchange(other.bot_sort_, 0)},
          top_sort_ {std::exchange(other.top_sort_, 1)},
          type_system_ {std::move(other.type_system_)}
    {
    }
    NO_MOVE_OPERATOR(FullTypeSystem);
    ~FullTypeSystem() = default;
    SortIdx GetSort(const PandaString &name)
    {
        return sort_[name];
    }
    const PandaString &ImageOfType(const Type &type)
    {
        return type_image_[type];
    }
    PandaString ImageOfTypeParams(const TypeParams &type_params)
    {
        return type_image_.ImageOfTypeParams(type_params);
    }
    TypeSystem &GetTypeSystem()
    {
        return type_system_;
    }

private:
    SortNames sort_;
    TypeImage type_image_;
    SortIdx bot_sort_;
    SortIdx top_sort_;
    TypeSystem type_system_;
};

struct TypeSystems::Impl {
    explicit Impl(size_t numThreads) : type_systems {}
    {
        for (auto kind : {TypeSystemKind::PANDA, TypeSystemKind::JAVA}) {
            type_systems[kind].reserve(numThreads);
            for (ThreadNum threadNum = 0; threadNum < numThreads; threadNum++) {
                type_systems[kind].emplace_back(kind, threadNum);
            }
        }
    }

    FullTypeSystem &GetFullTypeSystem(TypeSystem *tsys)
    {
        return type_systems[tsys->GetKind()][tsys->GetThreadNum()];
    }

    FullTypeSystem &GetFullTypeSystem(TypeSystemKind kind, ThreadNum threadNum)
    {
        return type_systems[kind][threadNum];
    }

private:
    EnumArray<PandaVector<FullTypeSystem>, TypeSystemKind, TypeSystemKind::PANDA, TypeSystemKind::JAVA> type_systems;
};

void TypeSystems::Initialize(size_t numThreads)
{
    if (impl != nullptr) {
        return;
    }
    impl = new (mem::AllocatorAdapter<TypeSystems::Impl>().allocate(1)) Impl {numThreads};
    ASSERT(impl != nullptr);
}

void TypeSystems::Destroy()
{
    if (impl == nullptr) {
        return;
    }
    impl->~Impl();
    mem::AllocatorAdapter<TypeSystems::Impl>().deallocate(impl, 1);
    impl = nullptr;
}

const PandaString &TypeSystems::ImageOfType(const Type &type)
{
    ASSERT(impl != nullptr);
    return impl->GetFullTypeSystem(&type.GetTypeSystem()).ImageOfType(type);
}

PandaString TypeSystems::ImageOfTypeParams(const TypeParams &type)
{
    ASSERT(impl != nullptr);
    return impl->GetFullTypeSystem(&type.GetTypeSystem()).ImageOfTypeParams(type);
}

SortIdx TypeSystems::GetSort(TypeSystemKind kind, ThreadNum threadnum, const PandaString &name)
{
    ASSERT(impl != nullptr);
    return impl->GetFullTypeSystem(kind, threadnum).GetSort(name);
}

TypeSystem &TypeSystems::Get(TypeSystemKind kind, ThreadNum threadnum)
{
    ASSERT(impl != nullptr);
    return impl->GetFullTypeSystem(kind, threadnum).GetTypeSystem();
}

}  // namespace panda::verifier
