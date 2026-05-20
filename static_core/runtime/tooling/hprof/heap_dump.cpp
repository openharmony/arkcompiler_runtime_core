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
        if (strObject->GetLength() > 0 && !strObject->IsUtf16()) {
            return std::string(reinterpret_cast<const char *>(strObject->GetDataMUtf8()), strObject->GetUtf8Length());
        }
        return "";
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

void HeapDump::DumpObjectFields(ObjectHeader *object, std::vector<arkplatform::EdgeInfo> &edges,
                                const WeakEdgeChecker &checker)
{
    auto *cls = object->ClassAddr<Class>();
    auto addr = reinterpret_cast<uint64_t>(object);
    do {
        for (auto &field : cls->GetInstanceFields()) {
            if (field.GetTypeId() != panda_file::Type::TypeId::REFERENCE) {
                continue;
            }
            ObjectHeader *toObject = object->GetFieldObject(field.GetOffset());
            if (toObject == nullptr) {
                continue;
            }
            auto edgeType = IsWeakReferentEdge(object, field, checker) ? arkplatform::StaticEdgeType::WEAK
                                                                       : arkplatform::StaticEdgeType::PROPERTY;
            edges.push_back(arkplatform::EdgeInfo {edgeType, addr, reinterpret_cast<uint64_t>(toObject),
                                                   mem::GetFieldName(field), 0});
        }
        cls = cls->GetBase();
    } while (cls != nullptr);
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
        ObjectHeader *toObject = array->GetObject(offset);
        if (toObject != nullptr) {
            edges.push_back(arkplatform::EdgeInfo {arkplatform::StaticEdgeType::ELEMENT, addr,
                                                   reinterpret_cast<uint64_t>(toObject), "",
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
    for (auto &field : runtimeCls->GetStaticFields()) {
        if (field.GetTypeId() != panda_file::Type::TypeId::REFERENCE) {
            continue;
        }

        ObjectHeader *toObject = runtimeCls->GetFieldObject(field.GetOffset());
        if (toObject == nullptr) {
            continue;
        }

        auto edgeType = IsWeakReferentEdge(object, field, checker) ? arkplatform::StaticEdgeType::WEAK
                                                                   : arkplatform::StaticEdgeType::PROPERTY;
        edges.push_back(
            arkplatform::EdgeInfo {edgeType, addr, reinterpret_cast<uint64_t>(toObject), mem::GetFieldName(field), 0});
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
