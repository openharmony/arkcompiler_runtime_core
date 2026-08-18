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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_DUMP_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_DUMP_H

#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"
#include "plugins/ets/runtime/tooling/hprof/session/object_id_map.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"
#include "plugins/ets/runtime/tooling/hprof/static_writer.h"
#include "profiler/heap_dump.h"
#include "libarkbase/macros.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/include/mem/panda_containers.h"
#include "libarkfile/file.h"
#include "libarkfile/include/type.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace ark {
class ObjectHeader;
class Class;
class Field;
class ManagedThread;
class PandaVM;
}  // namespace ark

namespace ark::coretypes {
class Array;
class TaggedValue;
}  // namespace ark::coretypes

namespace ark::tooling::hprof {

using common::dump::AbstractDumper;
using common::dump::DumpIdentity;
using common::dump::DumpRequest;
using common::dump::DumpResult;
using common::dump::DumpStatistics;

class StaticDumpTest;

/**
 * @brief Static-side heap dump implementation for hybrid binary dump format.
 *
 * Owns the complete static-side lifecycle, including runtime suspension,
 * descriptor acquisition, stream/writer construction, heap traversal, and
 * static record serialization.
 */
class StaticDump : public AbstractDumper {
    friend class StaticDumpTest;

public:
    using RootCallback = std::function<void(ark::ObjectHeader *)>;

    StaticDump(ark::PandaVM *pandaVm, StringIdPool *stringPool, ObjectIdMap *objectIdMap, const DumpRequest &request);

    ~StaticDump() override;

    NO_COPY_SEMANTIC(StaticDump);
    NO_MOVE_SEMANTIC(StaticDump);

    // -- AbstractDumper interface --

    /// @brief Force a full GC in the static runtime.
    void TriggerGC() override;
    void PrepareSession() override;
    bool AcquireOutput() override;
    int GetOutputFd() const override
    {
        return outputFd_;
    }
    DumpResult Dump() override;

    /**
     * @brief Return the OutputStream owned by StaticDump.
     * Used by HeapDumpCoordinator for CommonWriter operations.
     */
    common::dump::DumpOutput *GetOutput() override
    {
        return staticStream_;
    }

    // -- Phase implementations (called by Prepare/Execute/Finalize) --

    bool DumpRoot();
    bool DumpClass();
    bool DumpInstance();

    // -- Static utilities --

    /**
     * @brief Map panda_file::Type::TypeId to HPROF FieldType.
     *
     * Necessary because the two type systems are fundamentally different:
     *   - TypeId has 16 values with signed/unsigned distinction (I8 vs U8, etc.)
     *   - FieldType collapses signed/unsigned primitives into one category
     *     (both I8 and U8 -> BYTE) and retains TAGGED as a declared type.
     *   - TypeId has INVALID, VOID, NOVALUE with no FieldType equivalent -> UNKNOWN.
     *   The hybrid binary dump wire format requires FieldType values in field descriptors;
     *   using raw TypeId values would break the parser.
     */
    static FieldType MapFieldType(ark::panda_file::Type::TypeId typeId);

    /**
     * @brief Resolve the element FieldType for an array class.
     *
     * Primary path: MapFieldType(componentType->GetType().GetId()).
     * Fallback: DeriveElementTypeFromDescriptor(cls) when the component type's
     * TypeId is INVALID/VOID (ETS runtime may not properly initialize it).
     * When componentType is null, the descriptor distinguishes Any[] from a
     * normal reference array; an unrecognized descriptor falls back to OBJECT.
     */
    static FieldType ResolveArrayElementType(const ark::Class *cls);

    /**
     * @brief Derive FieldType from an array class descriptor string.
     *
     * When MapFieldType(componentType->GetType().GetId()) returns UNKNOWN
     * (because the component type's TypeId is INVALID/VOID), this function
     * parses the array class descriptor to determine the element type.
     * Descriptors follow the JVM/Panda convention:
     *   "[Z" -> BOOLEAN,  "[B" -> BYTE,  "[H" -> BYTE,
     *   "[C" -> CHAR,  "[S" -> SHORT,
     *   "[I" -> INT,  "[U" -> INT,
     *   "[J" -> LONG,  "[Q" -> LONG,
     *   "[F" -> FLOAT,  "[D" -> DOUBLE,
     *   "[A" -> TAGGED,  "[L...;" -> OBJECT,  "[[..." -> ARRAY
     */
    static FieldType DeriveElementTypeFromDescriptor(const ark::Class *cls);

    /**
     * @brief Parse array descriptor string to infer element type.
     * Takes the raw descriptor bytes so the parser is reachable without a Class.
     * @param desc Pointer to descriptor string (first byte should be '[' for array).
     * @return FieldType inferred from the element type character, or UNKNOWN if not an array.
     */
    static FieldType DeriveFromDescriptorChars(const uint8_t *desc);

    static uint32_t ComputeClassFlags(const ark::Class *cls);
    static std::unique_ptr<AbstractDumper> Create(ark::PandaVM *pandaVm, StringIdPool *stringPool,
                                                  ObjectIdMap *objectIdMap, const DumpRequest &request);
    static std::string ExtractFieldName(const ark::panda_file::File::StringData &nameData);

private:
    class RuntimeStateScope;

    /**
     * @brief Traverse the reachable heap and collect shared identifiers.
     * @return Object and class counts for the static heap summary.
     */
    DumpStatistics Prepare();
    bool CreateOutputWriter();
    bool Execute();
    bool Finalize();

    // -- Traversal helpers --

    void WalkRoots(const RootCallback &callback);

    // -- DumpClass record writers (split from DumpClass) --
    void WriteLoadClassRecord();
    void WriteClassDumpRecord();
    void WriteClassDumpItemFor(ark::Class *cls);

    /**
     * @brief Collect class metadata for one visited object during Prepare's BFS.
     *
     * For the object's class: register it in uniqueClasses_ (first object wins
     * as the representative for DumpClass), add its name + all field/method names
     * to the shared string pool. This must run during Prepare (before the pool
     * is frozen by Dump() so that Finalize's
     * GetStringId lookups resolve.
     */
    void CollectClassMetadata(ark::ObjectHeader *obj);

    /**
     * @brief Register one class and all metadata consumed by DumpClass.
     *
     * A class can be discovered either from an ordinary instance or from its
     * managed mirror. Keeping both paths here guarantees that a class with no
     * instances still contributes its fields and declared methods.
     */
    void RegisterClassMetadata(ark::Class *cls, ark::ObjectHeader *representative);

    // -- Data collection helpers (pre-collect data for writer WriteXXXItem methods) --

    /**
     * @brief Collect instance field descriptors for cls including inherited
     * fields, walking the base chain in the same order HeapDump::DumpObjectFields
     * traverses (subclass-own -> base-own -> ... -> root).
     *
     * The dumper's reachability BFS (DumpObjectFields) walks the base chain, so
     * inherited reference fields are followed and their referents get nodeIds.
     * The writer MUST emit values for the same set - otherwise objects
     * reachable only via an inherited reference field (e.g. Dog.species inherited
     * from Animal) are dumped as instances with no incoming edge and appear as
     * orphaned, root-unreachable nodes in the translated .heapsnapshot. Returns
     * pointers into the Class's own field spans; valid for the duration of the
     * dump.
     */
    ark::PandaVector<const ark::Field *> CollectInstanceFieldsChain(ark::Class *cls);

    /**
     * @brief Compute field descriptor data for a single field.
     * Returns a ClassFieldData struct ready to pass to WriteClassDumpItem.
     */
    ClassFieldData ComputeFieldData(const ark::Field &field);

    /**
     * @brief Compute field value data for a single instance field.
     * Returns a FieldValueData struct ready to pass to WriteInstanceDumpItem.
     */
    FieldValueData ComputeFieldValueData(ark::ObjectHeader *obj, const ark::Field &field);

    /**
     * @brief Compute the value of a single field whose storage is rooted at
     * `base` (an ObjectHeader* - for instance fields this is the object, for
     * static fields this is the class mirror object). Shared by instance and
     * static field-value collection so both paths use identical read logic.
     */
    FieldValueData ReadFieldValueAt(ark::ObjectHeader *base, const ark::Field &field);

    /**
     * @brief Read one static field value from the Class object itself. Class is
     * NOT an ObjectHeader subclass, so this uses Class::GetFieldPrimitive<T> /
     * Class::GetFieldObject (the same path HeapDump::DumpClassStaticFields uses)
     * rather than the ObjectHeader-based ReadFieldValueAt.
     */
    FieldValueData ReadStaticFieldValueAt(ark::Class *cls, const ark::Field &field);

    /** Convert one runtime TaggedValue into the self-describing wire value. */
    FieldValueData EncodeTaggedValue(ark::coretypes::TaggedValue value);

    /**
     * @brief Collect static field values for a class by reading each static
     * field's slot from the Class object (cls->GetFieldPrimitive/GetFieldObject).
     * Returns one FieldValueData per static field, in GetStaticFields() order.
     * Skips classes that are not initialized (static storage not ready).
     */
    ark::PandaVector<FieldValueData> ComputeStaticFieldValues(ark::Class *cls);

    /**
     * @brief Collect declared method-name string-pool ids for a class.
     * Iterates cls->GetMethods() (own declared methods only, no copied/
     * inherited), resolves each method name via ExtractFieldName +
     * stringPool_->GetStringId. Method-name strings must have been AddString'd
     * during Prepare() (before Freeze); GetStringId returns INVALID_STRING_ID
     * for strings not pre-registered.
     */
    ark::PandaVector<uint32_t> ComputeMethodData(ark::Class *cls);

    // -- Per-object write helpers (called from DumpInstance WalkHeap callbacks) --

    /// @brief Write one INSTANCE_DUMP item for a non-array object.
    void WriteNormalInstance(ark::ObjectHeader *obj, ark::Class *cls);

    /**
     * @brief Write one STATIC_STRING_DUMP item for a string object, embedding
     * the UTF-8 content so the translator can name the node by its value.
     */
    void WriteStringInstance(ark::ObjectHeader *obj, ark::Class *cls);

    /**
     * @brief Write one ARRAY_DUMP item for an array object.
     * Handles element type resolution, zero-length arrays, overflow guard,
     * null data check, and 32-bit managed pointer expansion.
     */
    void WriteArrayInstance(ark::ObjectHeader *obj, ark::Class *cls);

    void WriteEmptyArrayInstance(uint32_t objectId, uint32_t classObjectId, uint32_t objectSize, FieldType elementType);
    void WriteReferenceArrayInstance(const ark::coretypes::Array *array, uint32_t objectId, uint32_t classObjectId,
                                     uint32_t objectSize, uint32_t arrayLength, FieldType elementType);
    void WriteTaggedArrayInstance(const ark::coretypes::Array *array, uint32_t objectId, uint32_t classObjectId,
                                  uint32_t objectSize, uint32_t arrayLength);

    // -- Helpers --

    /**
     * @brief Resolve the classObjectId (nodeId) for a class.
     *
     * Uses the class's managed mirror object (cls->GetManagedObject()) as the
     * class object - this matches HPROF semantics where the "class object" is
     * the Class mirror instance, and is the same object referenced by
     * GetInstanceIds for each instance's classObjectId. Falling back to the
     * representative instance (uniqueClasses_[cls]) only when the class has no
     * managed mirror object.
     *
     * DumpClass and GetInstanceIds both resolve classObjectId through this
     * helper; see the invariant note at the call site in static_dump.cpp.
     *
     * @return classObjectId (0 if the class has neither a mirror object nor a
     *         known representative instance).
     */
    uint32_t GetClassObjectId(ark::Class *cls);

    /**
     * @brief Get object ID and class object ID for an instance record.
     * Common pattern extracted from WriteNormalInstance and WriteArrayInstance.
     * @return (objectId, classObjectId) pair. classObjectId=0 if class has no
     *         managed object and no representative instance.
     */
    std::pair<uint32_t, uint32_t> GetInstanceIds(ark::ObjectHeader *obj, ark::Class *cls);

    // -- Data members --

    ark::PandaVM *pandaVm_;
    StaticWriter *writer_ = nullptr;        // owned by StaticDump (created from OutputStream)
    OutputStream *staticStream_ = nullptr;  // owned by StaticDump (created from fd in factory)
    int outputFd_ = -1;                     // owned directly until transferred to staticStream_
    StringIdPool *stringPool_;
    ObjectIdMap *objectIdMap_;
    DumpIdentity identity_ {};
    std::string outputPath_;
    std::unique_ptr<RuntimeStateScope> runtimeStateScope_;
    uint32_t classSerialNumber_ = 0;
    // Unique classes collected during Prepare - reused by DumpClass to avoid a redundant WalkHeap
    // traversal. Maps Class* -> representative ObjectHeader*.
    ark::PandaUnorderedMap<ark::Class *, ark::ObjectHeader *> uniqueClasses_;
};

// MapFieldType and ExtractFieldName are declared here but defined in
// static_dump.cpp (moved out of the header to satisfy G.FUD.06 -
// inline functions should be <=10 lines).

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_DUMP_H
