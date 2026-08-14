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

#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>

#include "runtime/tooling/hprof/heap_dump.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/object_helpers.h"

namespace ark::tooling::hprof {

namespace {

template <class T>
std::string FormatIntegerPrimitive(T value)
{
    return "Int:" + std::to_string(value);
}

template <class T>
std::string FormatFloatingPrimitive(T value)
{
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
    return "Double:" + stream.str();
}

struct PrimitiveFieldEdgeContext {
    std::vector<arkplatform::EdgeInfo> &edges;
    uint64_t addr;
    bool isSimplify;
    bool captureNumericValue;
};

template <class Owner>
bool FillPrimitiveFieldInfo(Owner *owner, const Field &field, arkplatform::EdgeInfo &edge, bool isSimplify,
                            bool captureNumericValue)
{
    using TypeId = panda_file::Type::TypeId;
    if (isSimplify) {
        return false;
    }
    if (field.GetTypeId() == TypeId::U1) {
        edge.primitiveType = arkplatform::StaticPrimitiveType::BOOLEAN;
        edge.primitiveValue = owner->template GetFieldPrimitive<bool>(field) ? "Boolean:true" : "Boolean:false";
        return true;
    }
    if (!captureNumericValue) {
        return false;
    }
    switch (field.GetTypeId()) {
        case TypeId::I8:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<int8_t>(field));
            break;
        case TypeId::U8:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<uint8_t>(field));
            break;
        case TypeId::I16:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<int16_t>(field));
            break;
        case TypeId::U16:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<uint16_t>(field));
            break;
        case TypeId::I32:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<int32_t>(field));
            break;
        case TypeId::U32:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<uint32_t>(field));
            break;
        case TypeId::I64:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<int64_t>(field));
            break;
        case TypeId::U64:
            edge.primitiveValue = FormatIntegerPrimitive(owner->template GetFieldPrimitive<uint64_t>(field));
            break;
        case TypeId::F32:
            edge.primitiveValue = FormatFloatingPrimitive(owner->template GetFieldPrimitive<float>(field));
            break;
        case TypeId::F64:
            edge.primitiveValue = FormatFloatingPrimitive(owner->template GetFieldPrimitive<double>(field));
            break;
        default:
            return false;
    }
    edge.primitiveType = arkplatform::StaticPrimitiveType::NUMBER;
    return true;
}

template <class Owner>
void AddPrimitiveFieldEdge(Owner *owner, const Field &field, const PrimitiveFieldEdgeContext &context)
{
    arkplatform::EdgeInfo edge {arkplatform::StaticEdgeType::PROPERTY, context.addr, 0, "", 0};
    if (!FillPrimitiveFieldInfo(owner, field, edge, context.isSimplify, context.captureNumericValue)) {
        return;
    }
    edge.name = mem::GetFieldName(field);
    context.edges.push_back(std::move(edge));
}

bool FillPrimitiveArrayInfo(coretypes::Array *array, panda_file::Type::TypeId typeId, size_t offset,
                            arkplatform::EdgeInfo &edge)
{
    using TypeId = panda_file::Type::TypeId;
    if (typeId == TypeId::U1) {
        edge.primitiveType = arkplatform::StaticPrimitiveType::BOOLEAN;
        edge.primitiveValue = array->GetPrimitive<bool>(offset) ? "Boolean:true" : "Boolean:false";
        return true;
    }
    switch (typeId) {
        case TypeId::I8:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<int8_t>(offset));
            break;
        case TypeId::U8:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<uint8_t>(offset));
            break;
        case TypeId::I16:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<int16_t>(offset));
            break;
        case TypeId::U16:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<uint16_t>(offset));
            break;
        case TypeId::I32:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<int32_t>(offset));
            break;
        case TypeId::U32:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<uint32_t>(offset));
            break;
        case TypeId::I64:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<int64_t>(offset));
            break;
        case TypeId::U64:
            edge.primitiveValue = FormatIntegerPrimitive(array->GetPrimitive<uint64_t>(offset));
            break;
        case TypeId::F32:
            edge.primitiveValue = FormatFloatingPrimitive(array->GetPrimitive<float>(offset));
            break;
        case TypeId::F64:
            edge.primitiveValue = FormatFloatingPrimitive(array->GetPrimitive<double>(offset));
            break;
        default:
            return false;
    }
    edge.primitiveType = arkplatform::StaticPrimitiveType::NUMBER;
    return true;
}

}  // namespace

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
                                const WeakEdgeChecker &checker, bool isSimplify, bool captureNumericValue)
{
    auto addr = reinterpret_cast<uint64_t>(object);
    const PrimitiveFieldEdgeContext primitiveContext {edges, addr, isSimplify, captureNumericValue};
    for (auto *cls = object->ClassAddr<Class>(); cls != nullptr; cls = cls->GetBase()) {
        for (auto &field : cls->GetInstanceFields()) {
            if (field.GetTypeId() != panda_file::Type::TypeId::REFERENCE) {
                AddPrimitiveFieldEdge(object, field, primitiveContext);
            }
        }

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

void HeapDump::DumpArrayElements(ObjectHeader *object, Class *cls, std::vector<arkplatform::EdgeInfo> &edges,
                                 bool isSimplify, bool captureNumericValue)
{
    auto addr = reinterpret_cast<uint64_t>(object);
    auto *array = coretypes::Array::Cast(object);
    if (!cls->IsObjectArrayClass()) {
        auto typeId = cls->GetComponentType()->GetType().GetId();
        if (isSimplify || (!captureNumericValue && typeId != panda_file::Type::TypeId::U1)) {
            return;
        }
        auto arrayLength = array->GetLength();
        auto componentSize = cls->GetComponentSize();
        edges.reserve(edges.size() + arrayLength);
        for (ArraySizeT arrIndex = 0; arrIndex < arrayLength; ++arrIndex) {
            auto offset = arrIndex * componentSize;
            arkplatform::EdgeInfo edge {arkplatform::StaticEdgeType::ELEMENT, addr, 0, "",
                                        static_cast<uint32_t>(arrIndex)};
            if (FillPrimitiveArrayInfo(array, typeId, offset, edge)) {
                edges.push_back(std::move(edge));
            }
        }
        return;
    }
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
                                     const WeakEdgeChecker &checker, bool isSimplify, bool captureNumericValue)
{
    auto *runtimeCls = Class::FromClassObject(object);
    if (runtimeCls == nullptr) {
        return;
    }
    if (!runtimeCls->IsInitializing() && !runtimeCls->IsInitialized()) {
        return;
    }
    auto addr = reinterpret_cast<uint64_t>(object);
    const PrimitiveFieldEdgeContext primitiveContext {edges, addr, isSimplify, captureNumericValue};
    for (auto &field : runtimeCls->GetStaticFields()) {
        if (field.GetTypeId() != panda_file::Type::TypeId::REFERENCE) {
            AddPrimitiveFieldEdge(runtimeCls, field, primitiveContext);
        }
    }

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
                              const WeakEdgeChecker &checker, bool isSimplify, bool captureNumericValue)
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
        DumpArrayElements(object, cls, edges, isSimplify, captureNumericValue);
    } else if (cls->IsClassClass()) {
        DumpClassStaticFields(object, edges, checker, isSimplify, captureNumericValue);
    } else {
        DumpObjectFields(object, edges, checker, isSimplify, captureNumericValue);
    }
}

}  // namespace ark::tooling::hprof
