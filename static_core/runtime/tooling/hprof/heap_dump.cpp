/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <string>

#include "runtime/tooling/hprof/heap_dump.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/object_helpers.h"

namespace ark::tooling::hprof {

bool HeapDump::IsWeakReferentEdge(ObjectHeader *object, const Field &field, const WeakEdgeChecker &checker)
{
    return checker && checker(object, field);
}

arkplatform::StaticNodeType HeapDump::MapToStaticNodeType(Class *cls)
{
    if (cls->IsArrayClass()) {
        return arkplatform::StaticNodeType::ARRAY;
    }
    if (cls->IsStringClass()) {
        return arkplatform::StaticNodeType::STRING;
    }
    if (cls->IsClassClass()) {
        return arkplatform::StaticNodeType::CLASS;
    }
    return arkplatform::StaticNodeType::OBJECT;
}

std::string HeapDump::GetNodeName(ObjectHeader *object)
{
    auto *cls = object->ClassAddr<Class>();
    if (cls->IsArrayClass()) {
        auto *array = coretypes::Array::Cast(object);
        return "Array[" + std::to_string(array->GetLength()) + "]";
    }
    if (cls->IsStringClass()) {
        auto *strObject = coretypes::String::Cast(object);
        size_t len = strObject->GetUtf8Length();
        if (len == 0) {
            return "";
        }
        std::string out(len, '\0');
        size_t copied = strObject->CopyDataRegionUtf8(reinterpret_cast<uint8_t *>(out.data()), 0, len, len);
        out.resize(copied);
        return out;
    }
    if (cls->IsClassClass()) {
        auto *runtimeCls = Class::FromClassObject(object);
        return std::string(runtimeCls->GetName());
    }
    return std::string(cls->GetName());
}

size_t HeapDump::GetObjectSize(ObjectHeader *object)
{
    auto *cls = object->ClassAddr<Class>();
    return object->ObjectSize<LangTypeT::LANG_TYPE_STATIC>(cls);
}

arkplatform::NodeInfo HeapDump::ObjectToNodeInfo(ObjectHeader *object)
{
    auto *cls = object->ClassAddr<Class>();
    return arkplatform::NodeInfo {GetNodeName(object), MapToStaticNodeType(cls), GetObjectSize(object), 0,
                                  reinterpret_cast<uint64_t>(object)};
}

void HeapDump::ForceFullGC(PandaVM *vm)
{
    ASSERT(vm != nullptr);
    auto *gc = vm->GetGC();
    if (gc == nullptr) {
        return;
    }
    ScopedManagedCodeThread sm(ManagedThread::GetCurrent());
    GCTask task(GCTaskCause::OOM_CAUSE);
    gc->WaitForGCInManaged(task);
}

std::vector<arkplatform::NodeInfo> HeapDump::GetAllEtsObjects(PandaVM *vm)
{
    std::vector<arkplatform::NodeInfo> objects;
    auto *heapManager = vm->GetHeapManager();
    auto visitor = [&objects](ObjectHeader *object) { objects.push_back(ObjectToNodeInfo(object)); };
    heapManager->IterateOverObjects(visitor);
    return objects;
}

void HeapDump::IterateAllObjects(PandaVM *vm, const std::function<void(uint64_t)> &callback)
{
    auto *heapManager = vm->GetHeapManager();
    auto visitor = [&callback](ObjectHeader *object) { callback(reinterpret_cast<uint64_t>(object)); };
    heapManager->IterateOverObjects(visitor);
}

static const Field *FindInstanceFieldByOffset(Class *cls, uint32_t offset)
{
    for (auto &field : cls->GetInstanceFields()) {
        if (field.GetOffset() == offset) {
            return &field;
        }
    }
    return nullptr;
}

static const Field *FindStaticFieldByOffset(Class *cls, uint32_t offset)
{
    for (auto &field : cls->GetStaticFields()) {
        if (field.GetOffset() == offset) {
            return &field;
        }
    }
    return nullptr;
}

void HeapDump::DumpObjectFields(ObjectHeader *object, std::vector<arkplatform::EdgeInfo> &edges,
                                const WeakEdgeChecker &checker)
{
    auto addr = reinterpret_cast<uint64_t>(object);
    for (auto *cls = object->ClassAddr<Class>(); cls != nullptr; cls = cls->GetBase()) {
        uint32_t refNum = cls->GetRefFieldsNum<false>();
        if (refNum == 0) {
            continue;
        }
        uint32_t offset = cls->GetRefFieldsOffset<false>();
        uint32_t refVolatileNum = cls->GetVolatileRefFieldsNum<false>();
        for (uint32_t i = 0; i < refNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            bool isVolatile = (i < refVolatileNum);
            ObjectHeader *targetObject =
                isVolatile ? object->GetFieldObject<true>(offset) : object->GetFieldObject<false>(offset);
            if (targetObject == nullptr) {
                continue;
            }
            const Field *field = FindInstanceFieldByOffset(cls, offset);
            auto edgeType = (field != nullptr && IsWeakReferentEdge(object, *field, checker))
                                ? arkplatform::StaticEdgeType::WEAK
                                : arkplatform::StaticEdgeType::PROPERTY;
            edges.push_back(arkplatform::EdgeInfo {edgeType, addr, reinterpret_cast<uint64_t>(targetObject),
                                                   field != nullptr ? mem::GetFieldName(*field) : std::string(), 0});
        }
    }
}

void HeapDump::DumpArrayElements(ObjectHeader *object, Class *cls, std::vector<arkplatform::EdgeInfo> &edges)
{
    if (!cls->IsObjectArrayClass()) {
        return;
    }
    auto addr = reinterpret_cast<uint64_t>(object);
    auto *array = coretypes::Array::Cast(object);
    for (ArraySizeT arrIndex = 0; arrIndex < array->GetLength(); ++arrIndex) {
        auto offset = arrIndex * cls->GetComponentSize();
        ObjectHeader *targetObject = array->GetObject(offset);
        if (targetObject != nullptr) {
            edges.push_back(arkplatform::EdgeInfo {arkplatform::StaticEdgeType::ELEMENT, addr,
                                                   reinterpret_cast<uint64_t>(targetObject), "",
                                                   static_cast<uint32_t>(arrIndex)});
        }
    }
}

void HeapDump::DumpClassStaticFields(ObjectHeader *object, std::vector<arkplatform::EdgeInfo> &edges,
                                     const WeakEdgeChecker &checker)
{
    auto *runtimeCls = Class::FromClassObject(object);
    if (runtimeCls == nullptr) {
        return;
    }
    if (!runtimeCls->IsInitializing() && !runtimeCls->IsInitialized()) {
        return;
    }
    auto addr = reinterpret_cast<uint64_t>(object);
    uint32_t refNum = runtimeCls->GetRefFieldsNum<true>();
    if (refNum == 0) {
        return;
    }
    uint32_t offset = runtimeCls->GetRefFieldsOffset<true>();
    uint32_t refVolatileNum = runtimeCls->GetVolatileRefFieldsNum<true>();
    for (uint32_t i = 0; i < refNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
        bool isVolatile = (i < refVolatileNum);
        ObjectHeader *targetObject =
            isVolatile ? runtimeCls->GetFieldObject<true>(offset) : runtimeCls->GetFieldObject<false>(offset);
        if (targetObject == nullptr) {
            continue;
        }
        const Field *field = FindStaticFieldByOffset(runtimeCls, offset);
        auto edgeType = (field != nullptr && IsWeakReferentEdge(object, *field, checker))
                            ? arkplatform::StaticEdgeType::WEAK
                            : arkplatform::StaticEdgeType::PROPERTY;
        edges.push_back(arkplatform::EdgeInfo {edgeType, addr, reinterpret_cast<uint64_t>(targetObject),
                                               field != nullptr ? mem::GetFieldName(*field) : std::string(), 0});
    }
}

void HeapDump::DumpReferences(uint64_t etsAddr, std::vector<arkplatform::EdgeInfo> &edges,
                              const WeakEdgeChecker &checker)
{
    auto *object = reinterpret_cast<ObjectHeader *>(etsAddr);
    if (object == nullptr) {
        return;
    }
    auto *cls = object->ClassAddr<Class>();
    if (cls == nullptr) {
        return;
    }

    if (cls->IsArrayClass()) {
        DumpArrayElements(object, cls, edges);
    } else if (cls->IsClassClass()) {
        DumpClassStaticFields(object, edges, checker);
    } else {
        DumpObjectFields(object, edges, checker);
    }
}

}  // namespace ark::tooling::hprof
