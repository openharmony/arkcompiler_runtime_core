/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/tooling/hprof/static_dump.h"

#include <limits>
#include <new>
#include <string_view>
#include <vector>
#include <unistd.h>

#if defined(ENABLE_DUMP_IN_FAULTLOG)
#include "faultloggerd_client.h"
#endif

#include "runtime/include/object_header.h"
#include "runtime/include/class.h"
#include "runtime/include/field.h"
#include "runtime/include/method.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/language_config.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/tooling/hprof/heap_dump.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"

#include "plugins/ets/runtime/tooling/hprof/session/common_writer.h"
#include "plugins/ets/runtime/tooling/hprof/heap_dump_coordinator.h"
#include "plugins/ets/runtime/tooling/hprof/session/object_id_map.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"

namespace ark::tooling::hprof {
namespace {

using ReferenceVisitor = std::function<void(ark::ObjectHeader *, bool)>;

bool IsWeakReferentField(ark::ObjectHeader *object, const ark::Field &field)
{
    if (object == nullptr) {
        return false;
    }
    auto *etsClass = ark::ets::EtsObject::FromCoreType(object)->GetClass();
    return etsClass != nullptr && etsClass->IsWeakReference() &&
           field.GetOffset() == ark::ets::EtsWeakReference::GetReferentOffset();
}

void VisitTaggedReference(ark::coretypes::TaggedValue tagged, const ReferenceVisitor &visitor, bool forceWeak = false)
{
    if (!tagged.IsHeapObject()) {
        return;
    }
    auto *referent = tagged.IsWeak() ? tagged.GetWeakReferent() : tagged.GetHeapObject();
    if (referent != nullptr) {
        visitor(referent, forceWeak || tagged.IsWeak());
    }
}

void VisitArrayReferences(ark::ObjectHeader *object, ark::Class *cls, const ReferenceVisitor &visitor)
{
    auto *array = ark::coretypes::Array::Cast(object);
    if (StaticDump::ResolveArrayElementType(cls) == FieldType::TAGGED) {
        for (ark::ArraySizeT index = 0; index < array->GetLength(); ++index) {
            VisitTaggedReference(
                ark::coretypes::TaggedValue(array->Get<ark::coretypes::TaggedType, false, true>(index)), visitor);
        }
        return;
    }
    if (!cls->IsObjectArrayClass()) {
        return;
    }
    for (ark::ArraySizeT index = 0; index < array->GetLength(); ++index) {
        auto *referent = array->Get<ark::ObjectHeader *>(index);
        if (referent != nullptr) {
            visitor(referent, false);
        }
    }
}

void VisitObjectFields(ark::ObjectHeader *object, ark::Class *cls, const ReferenceVisitor &visitor)
{
    do {
        for (auto &field : cls->GetInstanceFields()) {
            if (field.GetTypeId() == ark::panda_file::Type::TypeId::TAGGED) {
                VisitTaggedReference(
                    ark::coretypes::TaggedValue(object->GetFieldPrimitive<ark::coretypes::TaggedType>(field)), visitor,
                    IsWeakReferentField(object, field));
                continue;
            }
            if (field.GetTypeId() != ark::panda_file::Type::TypeId::REFERENCE) {
                continue;
            }
            auto *referent = object->GetFieldObject(field);
            if (referent != nullptr) {
                visitor(referent, IsWeakReferentField(object, field));
            }
        }
        cls = cls->GetBase();
    } while (cls != nullptr);
}

void VisitStaticFields(ark::ObjectHeader *object, const ReferenceVisitor &visitor)
{
    auto *cls = ark::Class::FromClassObject(object);
    if (cls == nullptr || (!cls->IsInitializing() && !cls->IsInitialized())) {
        return;
    }
    for (auto &field : cls->GetStaticFields()) {
        if (field.GetTypeId() == ark::panda_file::Type::TypeId::TAGGED) {
            VisitTaggedReference(ark::coretypes::TaggedValue(cls->GetFieldPrimitive<ark::coretypes::TaggedType>(field)),
                                 visitor);
            continue;
        }
        if (field.GetTypeId() != ark::panda_file::Type::TypeId::REFERENCE) {
            continue;
        }
        auto *referent = cls->GetFieldObject(field);
        if (referent != nullptr) {
            visitor(referent, false);
        }
    }
}

void VisitObjectReferences(ark::ObjectHeader *object, const ReferenceVisitor &visitor)
{
    auto *cls = object->ClassAddr<ark::Class>();
    if (cls == nullptr) {
        return;
    }
    if (cls->IsArrayClass()) {
        VisitArrayReferences(object, cls, visitor);
    } else if (cls->IsClassClass()) {
        VisitStaticFields(object, visitor);
    } else {
        VisitObjectFields(object, cls, visitor);
    }
}

}  // namespace

// ============================================================================
// Constructor / Destructor
// ============================================================================

StaticDump::StaticDump(ark::PandaVM *pandaVm, StringIdPool *stringPool, ObjectIdMap *objectIdMap,
                       const DumpRequest &request)
    : pandaVm_(pandaVm),
      stringPool_(stringPool),
      objectIdMap_(objectIdMap),
      identity_(request.identity),
      outputPath_(request.output.staticPath)
{
}

StaticDump::~StaticDump()
{
    runtimeStateScope_.reset();
    if (writer_ != nullptr) {
        writer_->EndRecord();  // flush any pending record data
        delete writer_;
        writer_ = nullptr;
    }
    if (staticStream_ != nullptr) {
        staticStream_->Close();
        if (outputFd_ >= 0) {
            LOG(INFO, RUNTIME) << "[HybDump][Sta] Output fd closed: fd=" << outputFd_;
            outputFd_ = -1;
        }
        delete staticStream_;
        staticStream_ = nullptr;
    } else if (outputFd_ >= 0) {
        close(outputFd_);
        LOG(INFO, RUNTIME) << "[HybDump][Sta] Output fd closed: fd=" << outputFd_;
        outputFd_ = -1;
    }
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Dumper destroyed";
}

// ============================================================================
// AbstractDumper interface implementation
// ============================================================================

DumpStatistics StaticDump::Prepare()
{
    DumpStatistics result;

    ASSERT(pandaVm_ != nullptr);
    ASSERT(pandaVm_->GetHeapManager() != nullptr);

    LOG(INFO, RUNTIME) << "[HybDump][Sta] Heap prepare begin";

    // GC and thread suspension are coordinated before Prepare. No GC or
    // runtime-state transitions belong in this phase.

    // Per-round liveness: mark ALL previously-known entries dead so survivors
    // from a prior dump are re-traversed (and dead ones pruned). See the
    // ObjectIdMap lifecycle (see object_id_map.h) - PrepareRound, NOT Reset,
    // so nodeIds stay stable across dumps.
    objectIdMap_->PrepareRound();

    // BFS worklist of objects whose edges still need expanding. MarkLive is
    // the visited-check: it returns true only on first-visit-this-round, so
    // each object is pushed exactly once and survivors are correctly revisited
    // rather than skipped as "already in the map".
    ark::PandaVector<ark::ObjectHeader *> worklist;
    // Seed from the same root set the GC's InitialMark would enumerate.
    WalkRoots([this, &worklist](ark::ObjectHeader *obj) {
        if (objectIdMap_->MarkLive<Language::STATIC>(reinterpret_cast<uintptr_t>(obj))) {
            worklist.push_back(obj);
        }
    });

    while (!worklist.empty()) {
        auto *obj = worklist.back();
        worklist.pop_back();

        // Class metadata (uniqueClasses_, string pool) - same responsibilities
        // as the old full-heap walk, now applied per reachable object.
        CollectClassMetadata(obj);

        // String objects' internal storage (Char[]/Char buffers) is captured
        // by TAG_STATIC_STRING_DUMP's content, not as graph edges - the
        // translator's STRING node carries only the value (no field edges).
        // Skip expanding String fields here so internal storage objects are
        // not dumped as root-unreachable orphans. References held by user
        // objects still reach their target Strings via the parent's field.
        auto *cls = obj->ClassAddr<ark::Class>();
        if (cls != nullptr && cls->IsStringClass()) {
            continue;
        }

        // Strong instance/static fields and array elements define snapshot
        // reachability. Tagged weak values and ETS WeakRef referents do not
        // keep their targets alive.
        VisitObjectReferences(obj, [this, &worklist](ark::ObjectHeader *referent, bool weak) {
            if (!weak && objectIdMap_->MarkLive<Language::STATIC>(reinterpret_cast<uintptr_t>(referent))) {
                worklist.push_back(referent);
            }
        });
    }

    // Erase entries still dead this round (unreachable). After this the map
    // contains exactly the live, root-reachable set.
    objectIdMap_->PruneDead();

    result.objectCount = static_cast<uint32_t>(objectIdMap_->Count());
    result.classCount = static_cast<uint32_t>(uniqueClasses_.size());

    LOG(INFO, RUNTIME) << "[HybDump][Sta] Heap prepare end: objects=" << result.objectCount
                       << ", classes=" << result.classCount;

    return result;
}

void StaticDump::CollectClassMetadata(ark::ObjectHeader *obj)
{
    auto *cls = obj->ClassAddr<ark::Class>();
    if (cls == nullptr) {
        return;
    }

    RegisterClassMetadata(cls, obj);

    // A class mirror object (instance of the metaclass std.core.Class) is the
    // GC root for the underlying class. Classes with no instances (e.g.
    // SnapshotFixtures, a static holder) are never visited via an instance, so
    // they would be absent from uniqueClasses_ and their CLASS_DUMP + static
    // field edges would never be emitted - orphaning the objects their static
    // fields hold. When visiting a mirror, also register the underlying class
    // so DumpClass emits its static field edges and connects those objects.
    if (cls->IsClassClass()) {
        auto *underlying = ark::Class::FromClassObject(obj);
        if (underlying != nullptr) {
            RegisterClassMetadata(underlying, obj);
        }
    }
}

void StaticDump::RegisterClassMetadata(ark::Class *cls, ark::ObjectHeader *representative)
{
    ASSERT(cls != nullptr);
    bool inserted = uniqueClasses_.try_emplace(cls, representative).second;
    if (!inserted) {
        return;
    }

    stringPool_->AddString(cls->GetName());

    // Own field names (GetFields = own static + instance fields).
    for (auto &field : cls->GetFields()) {
        stringPool_->AddString(ExtractFieldName(field.GetName()));
    }
    // Inherited instance field names. The writer (CollectInstanceFieldsChain)
    // walks the base chain and emits descriptors + values for inherited
    // instance fields, so their names must be AddString'd here (before Freeze)
    // or ComputeFieldData's GetStringId returns INVALID and the edge ends up
    // with an empty name. GetFields() above is own-only, so walk the base
    // chain explicitly for inherited instance fields.
    for (ark::Class *base = cls->GetBase(); base != nullptr; base = base->GetBase()) {
        for (auto &field : base->GetInstanceFields()) {
            stringPool_->AddString(ExtractFieldName(field.GetName()));
        }
    }

    // Method names must be AddString'd here, before Dump() freezes the pool.
    // After Freeze, AddString returns
    // INVALID_STRING_ID and GetStringId lookups in Finalize/DumpClass would
    // fail. GetMethods() returns the class's own declared methods only.
    for (auto &method : cls->GetMethods()) {
        stringPool_->AddString(ExtractFieldName(method.GetName()));
    }
}

bool StaticDump::Execute()
{
    if (writer_ == nullptr) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Dump failed: output is not open";
        return false;
    }
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Dump begin";
    bool result = DumpRoot() && DumpInstance();
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Dump end: success=" << (result ? "true" : "false");
    return result;
}

bool StaticDump::Finalize()
{
    if (writer_ == nullptr) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Finalize failed: output is not open";
        return false;
    }
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Finalize begin";
    bool result = DumpClass();
    writer_->EndRecord();
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Finalize end: success=" << (result ? "true" : "false");
    return result;
}

// ============================================================================
// Phase implementations - batched record groups
// ============================================================================

bool StaticDump::DumpRoot()
{
    writer_->BeginRecord(TAG_ROOT_RECORD);
    WalkRoots([this](ark::ObjectHeader *obj) {
        uint32_t objectId = objectIdMap_->Find(reinterpret_cast<uintptr_t>(obj));
        writer_->WriteRootItem(objectId);  // WriteU8 + WriteU32 + FinishItem
    });
    writer_->EndRecord();
    return true;
}

bool StaticDump::DumpClass()
{
    WriteLoadClassRecord();
    WriteClassDumpRecord();
    return true;
}

// LOAD_CLASS group - all load class items batched into one record.
// classObjectId must match what GetInstanceIds writes for each instance
// of this class (the class mirror object), so the reader can join
// INSTANCE_DUMP.classNodeId -> classMap_[classObjectId].
void StaticDump::WriteLoadClassRecord()
{
    writer_->BeginRecord(TAG_LOAD_CLASS);
    for (auto &[cls, obj] : uniqueClasses_) {
        if (cls == nullptr) {
            continue;
        }
        uint32_t classObjectId = GetClassObjectId(cls);
        uint32_t classSerial = classSerialNumber_++;
        std::string className = cls->GetName();
        StringId classNameId = stringPool_->GetStringId(className);
        uint32_t classFlags = ComputeClassFlags(cls);

        writer_->WriteLoadClassItem(classSerial, classObjectId, classNameId, classFlags);
    }
    writer_->EndRecord();
}

// CLASS_DUMP group - all class dump items batched into one record.
void StaticDump::WriteClassDumpRecord()
{
    writer_->BeginRecord(TAG_STATIC_CLASS_DUMP);
    for (auto &[cls, obj] : uniqueClasses_) {
        if (cls == nullptr) {
            continue;
        }
        WriteClassDumpItemFor(cls);
    }
    writer_->EndRecord();
}

// Pre-collect field descriptor data, static field values (parallel to the
// static descriptors, same order) and declared method-name ids. Static values
// and method ids may be empty for classes without a mirror object / with no
// declared methods.
void StaticDump::WriteClassDumpItemFor(ark::Class *cls)
{
    uint32_t classObjectId = GetClassObjectId(cls);

    auto *superClass = cls->GetBase();
    uint32_t superClassId = 0;
    if (superClass != nullptr) {
        superClassId = GetClassObjectId(superClass);
    }

    auto staticFields = cls->GetStaticFields();
    auto instanceFieldChain = CollectInstanceFieldsChain(cls);
    auto staticCount = static_cast<uint16_t>(staticFields.Size());
    auto instanceCount = static_cast<uint16_t>(instanceFieldChain.size());

    ark::PandaVector<ClassFieldData> staticFieldData;
    for (size_t i = 0; i < staticFields.Size(); i++) {
        staticFieldData.push_back(ComputeFieldData(staticFields[i]));
    }
    ark::PandaVector<ClassFieldData> instanceFieldData;
    instanceFieldData.reserve(instanceFieldChain.size());
    for (const auto *field : instanceFieldChain) {
        instanceFieldData.push_back(ComputeFieldData(*field));
    }

    auto staticValues = ComputeStaticFieldValues(cls);
    auto methodIds = ComputeMethodData(cls);
    auto staticValueCount = static_cast<uint16_t>(staticValues.size());
    auto methodCount = static_cast<uint16_t>(methodIds.size());

    writer_->WriteClassDumpItem(classObjectId, superClassId, cls->GetObjectSize(), staticFieldData.data(), staticCount,
                                instanceFieldData.data(), instanceCount, staticValues.data(), staticValueCount,
                                methodIds.data(), methodCount);
}

bool StaticDump::DumpInstance()
{
    ASSERT(pandaVm_ != nullptr);
    // Iterate the live (root-reachable) set captured during Prepare's BFS.
    // After PrepareRound -> BFS -> PruneDead, ObjectIdMap holds exactly the
    // reachable objects, so no second full-heap walk is needed.
    //
    // INSTANCE_DUMP - all normal (non-array, non-string) instances in one batch
    writer_->BeginRecord(TAG_STATIC_INSTANCE_DUMP);
    objectIdMap_->ForEachLive([this](uintptr_t addr, [[maybe_unused]] uint32_t nodeId) {
        auto *obj = reinterpret_cast<ark::ObjectHeader *>(addr);
        auto *cls = obj->ClassAddr<ark::Class>();
        if (cls != nullptr && !cls->IsArrayClass() && !cls->IsStringClass()) {
            WriteNormalInstance(obj, cls);
        }
    });
    writer_->EndRecord();

    // STRING_DUMP - string objects carry their UTF-8 content so the translator
    // can name each node by its value. Excluded from INSTANCE_DUMP above so the
    // string's raw character buffer (not a named instance field) is not lost.
    writer_->BeginRecord(TAG_STATIC_STRING_DUMP);
    objectIdMap_->ForEachLive([this](uintptr_t addr, [[maybe_unused]] uint32_t nodeId) {
        auto *obj = reinterpret_cast<ark::ObjectHeader *>(addr);
        auto *cls = obj->ClassAddr<ark::Class>();
        if (cls != nullptr && cls->IsStringClass()) {
            WriteStringInstance(obj, cls);
        }
    });
    writer_->EndRecord();

    // ARRAY_DUMP - all array instances in a separate batch
    writer_->BeginRecord(TAG_STATIC_ARRAY_DUMP);
    objectIdMap_->ForEachLive([this](uintptr_t addr, [[maybe_unused]] uint32_t nodeId) {
        auto *obj = reinterpret_cast<ark::ObjectHeader *>(addr);
        auto *cls = obj->ClassAddr<ark::Class>();
        if (cls != nullptr && cls->IsArrayClass()) {
            WriteArrayInstance(obj, cls);
        }
    });
    writer_->EndRecord();

    return true;
}

// ============================================================================
// Traversal helpers
// ============================================================================

void StaticDump::WalkRoots(const RootCallback &callback)
{
    ASSERT(pandaVm_ != nullptr);
    ark::GCRootVisitor gcVisitor = [&callback](const ark::mem::GCRoot &gcRoot) {
        auto *obj = gcRoot.GetObjectHeader();
        if (obj != nullptr) {
            callback(obj);
        }
    };
    ark::mem::RootManager<ark::EtsLanguageConfig> rootManager(pandaVm_);
    rootManager.VisitNonHeapRoots(gcVisitor, ark::mem::VisitGCRootFlags::ACCESS_ROOT_ALL);

    // The string table is visited separately by every GC implementation and
    // is intentionally not part of RootManager::VisitNonHeapRoots.
    pandaVm_->VisitStringTable(gcVisitor, ark::mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
}

// ============================================================================
// Data collection helpers (pre-collect data for writer WriteXXXItem methods)
// ============================================================================

ark::PandaVector<const ark::Field *> StaticDump::CollectInstanceFieldsChain(ark::Class *cls)
{
    ark::PandaVector<const ark::Field *> fields;
    // Walk the base chain exactly like HeapDump::DumpObjectFields: own instance
    // fields of the most-derived class first, then its base, and so on to the
    // root. This is the set of reference fields the BFS follows for
    // reachability, so the writer must emit values for the same set - otherwise
    // objects reachable only via an inherited reference field become orphans.
    auto *c = cls;
    while (c != nullptr) {
        auto own = c->GetInstanceFields();
        for (size_t i = 0; i < own.Size(); i++) {
            fields.push_back(&own[i]);
        }
        c = c->GetBase();
    }
    return fields;
}

ClassFieldData StaticDump::ComputeFieldData(const ark::Field &field)
{
    auto nameData = field.GetName();
    std::string fieldName = ExtractFieldName(nameData);
    StringId nameId = stringPool_->GetStringId(fieldName);

    FieldType ft = MapFieldType(field.GetTypeId());
    uint16_t flags = 0;
    if (field.IsStatic()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_STATIC);
    }
    if (field.IsFinal()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_FINAL);
    }
    if (field.IsVolatile()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_VOLATILE);
    }
    if (field.IsPublic()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_PUBLIC);
    }
    if (field.IsPrivate()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_PRIVATE);
    }
    if (field.IsProtected()) {
        flags |= static_cast<uint16_t>(FieldFlags::IS_PROTECTED);
    }

    return ClassFieldData {nameId, static_cast<uint8_t>(ft), field.GetOffset(), flags};
}

FieldValueData StaticDump::ReadFieldValueAt(ark::ObjectHeader *base, const ark::Field &field)
{
    if (field.GetTypeId() == ark::panda_file::Type::TypeId::TAGGED) {
        auto rawValue = base->GetFieldPrimitive<ark::coretypes::TaggedType>(field);
        return EncodeTaggedValue(ark::coretypes::TaggedValue(rawValue));
    }

    auto ft = MapFieldType(field.GetTypeId());

    auto typeByte = static_cast<uint8_t>(ft);
    uint64_t value = 0;

    switch (ft) {
        case FieldType::BOOLEAN:
        case FieldType::BYTE:
            value = base->GetFieldPrimitive<uint8_t>(field);
            break;
        case FieldType::CHAR:
        case FieldType::SHORT:
            value = base->GetFieldPrimitive<uint16_t>(field);
            break;
        case FieldType::INT:
        case FieldType::FLOAT:
            value = base->GetFieldPrimitive<uint32_t>(field);
            break;
        case FieldType::LONG:
        case FieldType::DOUBLE:
            value = base->GetFieldPrimitive<uint64_t>(field);
            break;
        case FieldType::OBJECT:
        case FieldType::ARRAY: {
            auto *refObj = base->GetFieldObject(field);
            value = (refObj != nullptr) ? objectIdMap_->Find(reinterpret_cast<uintptr_t>(refObj)) : 0;
            if (IsWeakReferentField(base, field)) {
                typeByte = static_cast<uint8_t>(FieldType::WEAK_OBJECT);
            }
            break;
        }
        case FieldType::TAGGED:
        case FieldType::WEAK_OBJECT:
        case FieldType::UNKNOWN:
            break;
    }

    return FieldValueData {typeByte, value};
}

FieldValueData StaticDump::ComputeFieldValueData(ark::ObjectHeader *obj, const ark::Field &field)
{
    return ReadFieldValueAt(obj, field);
}

FieldValueData StaticDump::ReadStaticFieldValueAt(ark::Class *cls, const ark::Field &field)
{
    if (field.GetTypeId() == ark::panda_file::Type::TypeId::TAGGED) {
        auto rawValue = cls->GetFieldPrimitive<ark::coretypes::TaggedType>(field);
        return EncodeTaggedValue(ark::coretypes::TaggedValue(rawValue));
    }

    auto ft = MapFieldType(field.GetTypeId());
    auto typeByte = static_cast<uint8_t>(ft);
    uint64_t value = 0;

    switch (ft) {
        case FieldType::BOOLEAN:
        case FieldType::BYTE:
            value = cls->GetFieldPrimitive<uint8_t>(field);
            break;
        case FieldType::CHAR:
        case FieldType::SHORT:
            value = cls->GetFieldPrimitive<uint16_t>(field);
            break;
        case FieldType::INT:
        case FieldType::FLOAT:
            value = cls->GetFieldPrimitive<uint32_t>(field);
            break;
        case FieldType::LONG:
        case FieldType::DOUBLE:
            value = cls->GetFieldPrimitive<uint64_t>(field);
            break;
        case FieldType::OBJECT:
        case FieldType::ARRAY: {
            auto *refObj = cls->GetFieldObject(field);
            value = (refObj != nullptr) ? objectIdMap_->Find(reinterpret_cast<uintptr_t>(refObj)) : 0;
            break;
        }
        case FieldType::TAGGED:
        case FieldType::WEAK_OBJECT:
        case FieldType::UNKNOWN:
            break;
    }
    return FieldValueData {typeByte, value};
}

FieldValueData StaticDump::EncodeTaggedValue(ark::coretypes::TaggedValue value)
{
    if (value.IsHeapObject()) {
        auto *object = value.IsWeak() ? value.GetWeakReferent() : value.GetHeapObject();
        auto nodeId = objectIdMap_->Find(reinterpret_cast<uintptr_t>(object));
        auto type = value.IsWeak() ? FieldType::WEAK_OBJECT : FieldType::OBJECT;
        return {static_cast<uint8_t>(type), nodeId};
    }
    if (value.IsInt()) {
        return {static_cast<uint8_t>(FieldType::INT), static_cast<uint32_t>(value.GetInt())};
    }
    if (value.IsDouble()) {
        return {static_cast<uint8_t>(FieldType::DOUBLE),
                ark::coretypes::ReinterpretDoubleToTaggedType(value.GetDouble())};
    }
    if (value.IsBoolean()) {
        return {static_cast<uint8_t>(FieldType::BOOLEAN), value.IsTrue() ? 1U : 0U};
    }
    return {static_cast<uint8_t>(FieldType::TAGGED), value.GetRawData()};
}

ark::PandaVector<FieldValueData> StaticDump::ComputeStaticFieldValues(ark::Class *cls)
{
    ark::PandaVector<FieldValueData> values;
    if (cls == nullptr) {
        return values;
    }
    // Static field values live in the Class object itself (Class is a managed
    // object, but NOT an ObjectHeader subclass - it has its own GetFieldPrimitive
    // / GetFieldObject). This mirrors HeapDump::DumpClassStaticFields, which
    // reads via Class::FromClassObject(object)->GetFieldObject(offset). Reading
    // from cls->GetManagedObject() instead yields garbage (the mirror does not
    // hold the static storage). Skip classes that are not initialized.
    if (!cls->IsInitializing() && !cls->IsInitialized()) {
        return values;
    }
    auto staticFields = cls->GetStaticFields();
    values.reserve(staticFields.Size());
    for (size_t i = 0; i < staticFields.Size(); i++) {
        values.push_back(ReadStaticFieldValueAt(cls, staticFields[i]));
    }
    return values;
}

ark::PandaVector<uint32_t> StaticDump::ComputeMethodData(ark::Class *cls)
{
    ark::PandaVector<uint32_t> methodIds;
    if (cls == nullptr) {
        return methodIds;
    }
    // GetMethods() returns the class's own declared methods (vmethods + smethods,
    // NOT copied/inherited), matching the "own fields" semantics already used by
    // GetStaticFields()/GetInstanceFields(). Method-name strings were AddString'd
    // during Prepare(), so GetStringId resolves here (after Freeze).
    auto methods = cls->GetMethods();
    methodIds.reserve(methods.Size());
    for (size_t i = 0; i < methods.Size(); i++) {
        std::string methodName = ExtractFieldName(methods[i].GetName());
        methodIds.push_back(stringPool_->GetStringId(methodName));
    }
    return methodIds;
}

// ============================================================================
// Per-object write helpers (called from DumpInstance ForEachLive callbacks)
// ============================================================================

void StaticDump::WriteNormalInstance(ark::ObjectHeader *obj, ark::Class *cls)
{
    auto [objectId, classObjectId] = GetInstanceIds(obj, cls);
    // Emit values for own + inherited instance fields (CollectInstanceFieldsChain
    // walks the base chain), matching the BFS reference set. The CLASS_DUMP
    // instance descriptors are written in the same order, so the translator can
    // pair each value with its descriptor.
    auto instanceFieldChain = CollectInstanceFieldsChain(cls);
    auto fieldCount = static_cast<uint16_t>(instanceFieldChain.size());

    // Pre-collect field value data
    ark::PandaVector<FieldValueData> fieldValues;
    fieldValues.reserve(instanceFieldChain.size());
    for (const auto *field : instanceFieldChain) {
        fieldValues.push_back(ComputeFieldValueData(obj, *field));
    }

    auto objectSize = static_cast<uint32_t>(HeapDump::GetObjectSize(obj));
    writer_->WriteInstanceDumpItem(objectId, classObjectId, objectSize, fieldValues.data(), fieldCount);
}

void StaticDump::WriteStringInstance(ark::ObjectHeader *obj, ark::Class *cls)
{
    auto [objectId, classObjectId] = GetInstanceIds(obj, cls);
    auto *strObject = ark::coretypes::String::Cast(obj);
    size_t utf8Length = strObject->GetUtf8Length();
    std::string content(utf8Length, '\0');
    if (utf8Length > 0) {
        size_t copied = strObject->CopyDataRegionUtf8(reinterpret_cast<uint8_t *>(content.data()), 0,
                                                      strObject->GetLength(), utf8Length);
        if (copied != utf8Length) {
            LOG(WARNING, RUNTIME) << "[HybDump][Sta] UTF-8 conversion incomplete: expected=" << utf8Length
                                  << ", copied=" << copied;
            content.resize(copied);
        }
    }
    auto objectSize = static_cast<uint32_t>(HeapDump::GetObjectSize(obj));
    writer_->WriteStringDumpItem(objectId, classObjectId, objectSize, reinterpret_cast<const uint8_t *>(content.data()),
                                 static_cast<uint32_t>(content.size()));
}

void StaticDump::WriteArrayInstance(ark::ObjectHeader *obj, ark::Class *cls)
{
    auto [objectId, classObjectId] = GetInstanceIds(obj, cls);
    auto *arr = reinterpret_cast<const ark::coretypes::Array *>(obj);
    uint32_t arrayLength = arr->GetLength();
    FieldType elementType = ResolveArrayElementType(cls);
    uint32_t elementSize = cls->GetComponentSize();
    auto objectSize = static_cast<uint32_t>(HeapDump::GetObjectSize(obj));

    if (arrayLength == 0) {
        WriteEmptyArrayInstance(objectId, classObjectId, objectSize, elementType);
        return;
    }

    if (elementSize > std::numeric_limits<uint32_t>::max() / arrayLength) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Array size overflow: length=" << arrayLength
                              << ", element_size=" << elementSize;
        return;
    }

    if (elementType == FieldType::OBJECT || elementType == FieldType::ARRAY) {
        WriteReferenceArrayInstance(arr, objectId, classObjectId, objectSize, arrayLength, elementType);
        return;
    }

    if (elementType == FieldType::TAGGED) {
        WriteTaggedArrayInstance(arr, objectId, classObjectId, objectSize, arrayLength);
        return;
    }

    auto *dataPtr = reinterpret_cast<const uint8_t *>(arr->GetData());
    if (dataPtr == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Array data missing: length=" << arrayLength;
        return;
    }
    uint32_t arrayDataSize = arrayLength * elementSize;
    writer_->WriteArrayDumpItem(objectId, classObjectId, objectSize, arrayLength, static_cast<uint8_t>(elementType),
                                dataPtr, arrayDataSize);
}

void StaticDump::WriteEmptyArrayInstance(uint32_t objectId, uint32_t classObjectId, uint32_t objectSize,
                                         FieldType elementType)
{
    if (elementType == FieldType::TAGGED) {
        writer_->WriteTaggedArrayDumpItem(objectId, classObjectId, objectSize, nullptr, 0);
        return;
    }
    writer_->WriteArrayDumpItem(objectId, classObjectId, objectSize, 0, static_cast<uint8_t>(elementType), nullptr, 0);
}

void StaticDump::WriteReferenceArrayInstance(const ark::coretypes::Array *array, uint32_t objectId,
                                             uint32_t classObjectId, uint32_t objectSize, uint32_t arrayLength,
                                             FieldType elementType)
{
    ark::PandaVector<uint32_t> nodeIds;
    nodeIds.reserve(arrayLength);
    for (uint32_t index = 0; index < arrayLength; ++index) {
        auto *object = array->Get<ark::ObjectHeader *>(index);
        auto nodeId = object == nullptr ? 0U : objectIdMap_->Find(reinterpret_cast<uintptr_t>(object));
        nodeIds.push_back(nodeId);
    }
    writer_->WriteArrayDumpItem(objectId, classObjectId, objectSize, arrayLength, static_cast<uint8_t>(elementType),
                                reinterpret_cast<const uint8_t *>(nodeIds.data()), nodeIds.size() * sizeof(uint32_t));
}

void StaticDump::WriteTaggedArrayInstance(const ark::coretypes::Array *array, uint32_t objectId, uint32_t classObjectId,
                                          uint32_t objectSize, uint32_t arrayLength)
{
    ark::PandaVector<FieldValueData> elements;
    elements.reserve(arrayLength);
    for (uint32_t index = 0; index < arrayLength; ++index) {
        auto rawValue = array->Get<ark::coretypes::TaggedType, false, true>(index);
        elements.push_back(EncodeTaggedValue(ark::coretypes::TaggedValue(rawValue)));
    }
    writer_->WriteTaggedArrayDumpItem(objectId, classObjectId, objectSize, elements.data(), arrayLength);
}

// ============================================================================
// Helpers
// ============================================================================

uint32_t StaticDump::GetClassObjectId(ark::Class *cls)
{
    auto *mirror = cls->GetManagedObject();
    if (mirror != nullptr) {
        return objectIdMap_->Find(reinterpret_cast<uintptr_t>(mirror));
    }
    // Fallback: a class without a managed mirror object (e.g. some primitive /
    // array classes) - use its representative instance so that LOAD_CLASS and
    // INSTANCE_DUMP still agree on the classObjectId.
    auto it = uniqueClasses_.find(cls);
    if (it != uniqueClasses_.end() && it->second != nullptr) {
        return objectIdMap_->Find(reinterpret_cast<uintptr_t>(it->second));
    }
    return 0;
}

std::pair<uint32_t, uint32_t> StaticDump::GetInstanceIds(ark::ObjectHeader *obj, ark::Class *cls)
{
    uint32_t objectId = objectIdMap_->Find(reinterpret_cast<uintptr_t>(obj));
    uint32_t classObjectId = GetClassObjectId(cls);
    return {objectId, classObjectId};
}

void StaticDump::TriggerGC()
{
    // Same as stsInterface_->EtsForceFullGC(): HeapDump::ForceFullGC(vm).
    ASSERT(pandaVm_ != nullptr);
    HeapDump::ForceFullGC(pandaVm_);
}

class StaticDump::RuntimeStateScope final {
public:
    explicit RuntimeStateScope(ark::PandaVM *pandaVm) : ownerPid_(getpid())
    {
        auto *thread = ark::ManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        if (!thread->IsManagedCode()) {
            thread->ManagedCodeBegin();
            enteredManagedCode_ = true;
        }

        auto *rendezvous = pandaVm->GetRendezvous();
        ASSERT(rendezvous != nullptr);
        ASSERT(rendezvous->GetMutatorLock()->HasLock());
        suspendScope_ = std::make_unique<ark::ScopedSuspendAllThreadsRunning>(rendezvous);
    }

    ~RuntimeStateScope()
    {
        if (getpid() != ownerPid_) {
            // The child inherited guards whose destructors would attempt to
            // resume parent runtime threads. The child exits after dumping,
            // so deliberately abandon these copied guards without running
            // their restore operations.
            (void)suspendScope_.release();
            return;
        }
        suspendScope_.reset();
        if (enteredManagedCode_) {
            auto *thread = ark::ManagedThread::GetCurrent();
            ASSERT(thread != nullptr);
            thread->ManagedCodeEnd();
        }
    }

    NO_COPY_SEMANTIC(RuntimeStateScope);
    NO_MOVE_SEMANTIC(RuntimeStateScope);

private:
    pid_t ownerPid_;
    bool enteredManagedCode_ {false};
    std::unique_ptr<ark::ScopedSuspendAllThreadsRunning> suspendScope_;
};

void StaticDump::PrepareSession()
{
    ASSERT(pandaVm_ != nullptr);
    if (runtimeStateScope_ == nullptr) {
        LOG(INFO, RUNTIME) << "[HybDump][Sta] Session prepare begin";
        runtimeStateScope_ = std::make_unique<RuntimeStateScope>(pandaVm_);
        LOG(INFO, RUNTIME) << "[HybDump][Sta] Session prepare end";
    }
}

bool StaticDump::AcquireOutput()
{
    if (staticStream_ != nullptr || outputFd_ >= 0) {
        return true;
    }
    if (!outputPath_.empty()) {
        staticStream_ = new (std::nothrow) OutputStream(outputPath_);
        if (staticStream_ == nullptr || !staticStream_->Good()) {
            delete staticStream_;
            staticStream_ = nullptr;
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output file open failed";
            return false;
        }
        return true;
    }
#if defined(ENABLE_DUMP_IN_FAULTLOG)
    if (!identity_.IsValid()) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output fd acquire failed: invalid dump identity";
        return false;
    }
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Output fd acquire begin";
    FaultLoggerdRequest fdRequest = {};
    fdRequest.type = static_cast<int32_t>(FaultLoggerType::STATIC_JS_RAW_SNAPSHOT);
    fdRequest.pid = identity_.GetPid();
    // The static runtime owns one process-wide heap, so its output is never
    // associated with an individual thread.
    fdRequest.tid = DumpIdentity::UNSPECIFIED_ID;
    fdRequest.time = identity_.GetTimestampMillis();
    int fd = RequestFileDescriptorEx(&fdRequest);
    if (fd < 0) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output fd acquire failed: faultlogger request failed";
        return false;
    }
    outputFd_ = fd;
    LOG(INFO, RUNTIME) << "[HybDump][Sta] Output fd acquired: fd=" << fd;
    return true;
#else
    return staticStream_ != nullptr;
#endif
}

bool StaticDump::CreateOutputWriter()
{
    if (writer_ != nullptr) {
        return true;
    }
    if (staticStream_ == nullptr) {
        if (outputFd_ < 0) {
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output writer creation failed: fd unavailable";
            return false;
        }
        staticStream_ = new (std::nothrow) OutputStream(outputFd_, OutputStream::DEFAULT_BUFFER_SIZE, true);
        if (staticStream_ == nullptr) {
            return false;
        }
        outputFd_ = -1;
    }
    writer_ = new (std::nothrow) StaticWriter(staticStream_);
    return writer_ != nullptr;
}

DumpResult StaticDump::Dump()
{
    DumpStatistics statistics = Prepare();
    if (!AcquireOutput() || !CreateOutputWriter()) {
        return {statistics, false};
    }

    stringPool_->Freeze();
    objectIdMap_->Freeze();
    {
        CommonWriter commonWriter(staticStream_);
        commonWriter.WriteFileHeader(Language::STATIC, statistics.objectCount, statistics.classCount);
        commonWriter.WriteStringPool(stringPool_);
    }
    bool executeSuccess = Execute();
    bool finalizeSuccess = Finalize();
    return {statistics, executeSuccess && finalizeSuccess};
}

uint32_t StaticDump::ComputeClassFlags(const ark::Class *cls)
{
    uint32_t flags = 0;
    if (cls->IsArrayClass()) {
        flags |= static_cast<uint32_t>(ClassFlags::IS_ARRAY);
    }
    if (cls->IsInterface()) {
        flags |= static_cast<uint32_t>(ClassFlags::IS_INTERFACE);
    }
    if (cls->IsAbstract()) {
        flags |= static_cast<uint32_t>(ClassFlags::IS_ABSTRACT);
    }
    if (cls->IsFinal()) {
        flags |= static_cast<uint32_t>(ClassFlags::IS_FINAL);
    }
    if (cls->IsPrimitive()) {
        flags |= static_cast<uint32_t>(ClassFlags::IS_PRIMITIVE);
    }
    return flags;
}

FieldType StaticDump::ResolveArrayElementType(const ark::Class *cls)
{
    auto *componentType = cls->GetComponentType();
    if (componentType == nullptr) {
        auto elementType = DeriveElementTypeFromDescriptor(cls);
        return elementType == FieldType::UNKNOWN ? FieldType::OBJECT : elementType;
    }

    FieldType elementType = MapFieldType(componentType->GetType().GetId());
    // Fallback: when TypeId maps to UNKNOWN (INVALID/VOID), parse the
    // array class descriptor.  ETS runtime may not properly initialize
    // the componentType's type_ field for some array classes.
    if (elementType == FieldType::UNKNOWN) {
        elementType = DeriveElementTypeFromDescriptor(cls);
    }
    return elementType;
}

FieldType StaticDump::DeriveElementTypeFromDescriptor(const ark::Class *cls)
{
    return DeriveFromDescriptorChars(cls->GetDescriptor());
}

// Parse the raw descriptor bytes to infer the element type (no Class needed).
FieldType StaticDump::DeriveFromDescriptorChars(const uint8_t *desc)
{
    constexpr size_t ARRAY_DESCRIPTOR_MIN_SIZE = 2U;
    constexpr size_t ELEMENT_TYPE_INDEX = 1U;

    // Array class descriptors: "[<type_char>" for primitive, "[L...;" for reference, "[[..." for nested
    if (desc == nullptr) {
        return FieldType::UNKNOWN;
    }
    std::string_view descriptor(utf::Mutf8AsCString(desc));
    if (descriptor.size() < ARRAY_DESCRIPTOR_MIN_SIZE || descriptor.front() != '[') {
        return FieldType::UNKNOWN;
    }
    switch (descriptor[ELEMENT_TYPE_INDEX]) {
        case 'Z':
            return FieldType::BOOLEAN;  // boolean (U1)
        case 'B':
            return FieldType::BYTE;  // byte (I8)
        case 'H':
            return FieldType::BYTE;  // unsigned byte (U8) -> BYTE (signed/unsigned collapse)
        case 'C':
            return FieldType::CHAR;  // char (U16)
        case 'S':
            return FieldType::SHORT;  // short (I16)
        case 'I':
            return FieldType::INT;  // int (I32)
        case 'U':
            return FieldType::INT;  // unsigned int (U32) -> INT (signed/unsigned collapse)
        case 'J':
            return FieldType::LONG;  // long (I64)
        case 'Q':
            return FieldType::LONG;  // unsigned long (U64) -> LONG (signed/unsigned collapse)
        case 'F':
            return FieldType::FLOAT;  // float (F32)
        case 'D':
            return FieldType::DOUBLE;  // double (F64)
        case 'A':
            return FieldType::TAGGED;  // tagged/dynamic value
        case 'L':
            return FieldType::OBJECT;  // reference type
        case '[':
            return FieldType::ARRAY;  // nested array (e.g., int[][])
        default:
            return FieldType::UNKNOWN;
    }
}

// ============================================================================
// Participant creation
// ============================================================================

std::unique_ptr<AbstractDumper> StaticDump::Create(ark::PandaVM *pandaVm, StringIdPool *stringPool,
                                                   ObjectIdMap *objectIdMap, const DumpRequest &request)
{
#if defined(PANDA_TARGET_ARM32)
    LOG(ERROR, RUNTIME) << "[HybDump][Sta] Dumper creation failed: ARM32 is not supported";
    return nullptr;
#endif
    if (pandaVm == nullptr || stringPool == nullptr || objectIdMap == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Dumper creation failed: invalid context";
        return nullptr;
    }
    auto *dumper = new (std::nothrow) StaticDump(pandaVm, stringPool, objectIdMap, request);
    if (dumper == nullptr) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Dumper creation failed: allocation failed";
    } else {
        LOG(INFO, RUNTIME) << "[HybDump][Sta] Dumper created";
    }
    return std::unique_ptr<AbstractDumper>(dumper);
}

// --- Field type mapping and name extraction (moved from header) ---
// Definitions moved from the header (see static_dump.h note on G.FUD.06).

FieldType StaticDump::MapFieldType(ark::panda_file::Type::TypeId typeId)
{
    using Tid = ark::panda_file::Type::TypeId;
    switch (typeId) {
        case Tid::U1:  // boolean
            return FieldType::BOOLEAN;
        case Tid::I8:  // signed byte
        case Tid::U8:  // unsigned byte
            return FieldType::BYTE;
        case Tid::I16:  // signed short
            return FieldType::SHORT;
        case Tid::U16:  // UTF-16 code unit
            return FieldType::CHAR;
        case Tid::I32:  // signed int
        case Tid::U32:  // unsigned int
            return FieldType::INT;
        case Tid::I64:  // signed long
        case Tid::U64:  // unsigned long
            return FieldType::LONG;
        case Tid::F32:  // float
            return FieldType::FLOAT;
        case Tid::F64:  // double
            return FieldType::DOUBLE;
        case Tid::TAGGED:
            return FieldType::TAGGED;
        case Tid::REFERENCE:  // object reference
            return FieldType::OBJECT;
        default:  // INVALID, VOID, NOVALUE
            return FieldType::UNKNOWN;
    }
}

std::string StaticDump::ExtractFieldName(const ark::panda_file::File::StringData &nameData)
{
    if (nameData.data == nullptr) {
        return {};
    }
    if (nameData.isAscii) {
        return std::string(reinterpret_cast<const char *>(nameData.data));
    }
    if (nameData.utf16Length == 0) {
        return {};
    }

    // This utility is also used before the runtime allocator is initialized,
    // so it must use the standard allocator rather than PandaVector.
    std::vector<uint16_t> utf16(nameData.utf16Length);
    ark::utf::ConvertMUtf8ToUtf16(nameData.data, ark::utf::Mutf8Size(nameData.data), utf16.data());
    size_t utf8Length = ark::utf::Utf16ToUtf8Size(utf16.data(), nameData.utf16Length, false) - 1;
    std::string result(utf8Length, '\0');
    if (utf8Length > 0) {
        size_t converted = ark::utf::ConvertRegionUtf16ToUtf8(utf16.data(), reinterpret_cast<uint8_t *>(result.data()),
                                                              utf16.size(), utf8Length, 0, false);
        result.resize(converted);
    }
    return result;
}

}  // namespace ark::tooling::hprof
