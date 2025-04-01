/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "runtime/include/class_linker.h"
#include "runtime/bridge/bridge.h"
#include "runtime/cha.h"
#include "runtime/class_initializer.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/field.h"
#include "runtime/include/itable_builder.h"
#include "runtime/include/method.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "libpandabase/macros.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/utils/bit_utils.h"
#include "libpandabase/utils/span.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/field_data_accessor-inl.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/modifiers.h"
#include "libpandafile/panda_cache.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "runtime/include/tooling/debug_inf.h"
#include "trace/trace.h"

namespace ark {

using Type = panda_file::Type;
using SourceLang = panda_file::SourceLang;

void ClassLinker::AddPandaFile(std::unique_ptr<const panda_file::File> &&pf, ClassLinkerContext *context)
{
    ASSERT(pf != nullptr);

    const panda_file::File *file = pf.get();

    SCOPED_TRACE_STREAM << __FUNCTION__ << " " << file->GetFilename();

    {
        os::memory::LockHolder lock {pandaFilesLock_};
        pandaFiles_.push_back({context, std::forward<std::unique_ptr<const panda_file::File>>(pf)});
    }

    if (context == nullptr || context->IsBootContext()) {
        os::memory::LockHolder lock {bootPandaFilesLock_};
        bootPandaFiles_.push_back(file);
    }

    if (Runtime::GetCurrent()->IsInitialized()) {
        // LoadModule for initial boot files is called in runtime
        Runtime::GetCurrent()->GetNotificationManager()->LoadModuleEvent(file->GetFilename());
    }

    tooling::DebugInf::AddCodeMetaInfo(file);
}

void ClassLinker::FreeClassData(Class *classPtr)
{
    Span<Field> fields = classPtr->GetFields();
    if (fields.Size() > 0) {
        allocator_->Free(fields.begin());
        classPtr->SetFields(Span<Field>(), 0);
    }
    Span<Method> methods = classPtr->GetMethods();
    size_t n = methods.Size() + classPtr->GetNumCopiedMethods();
    if (n > 0) {
        mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
        for (auto &method : methods) {
            // We create Profiling data in method class via InternalAllocator.
            // Therefore, we should delete it via InternalAllocator too.
            allocator->Free(method.GetProfilingData());
        }

        // Free copied method's ProfileData if it is necessary.
        for (auto &method : classPtr->GetCopiedMethods()) {
            auto id = method.GetFileId();
            auto pf = method.GetPandaFile();

            // There is a possibility that ProfilingData can be borrowed from the original method. Example:
            // 1. create a base interface class with a default method,
            // 2. call StartProfiling(),
            // 3. create a derived class and do not override the default method from the base interface,
            // 4. call StartProfiling() for the derived class.
            // Currently this example is impossible (the default method cannot be called from the interface) because
            // there is no such a language where #2 step will be executed.
            // Need more investigations to eliminate this extra check.
            Span<Method> originalMethods = method.GetClass()->GetMethods();
            auto it = std::find_if(originalMethods.begin(), originalMethods.end(),
                                   [id, pf](const auto &m) { return m.GetFileId() == id && m.GetPandaFile() == pf; });

            // Free method's ProfileData if it was set after copying of the original method.
            auto ptr = method.GetProfilingData();
            if (it == originalMethods.end() || it->GetProfilingDataWithoutCheck() != ptr) {
                allocator->Free(ptr);
            }
        }

        allocator_->Free(methods.begin());
        classPtr->SetMethods(Span<Method>(), 0, 0);
    }
    bool hasOwnItable = !classPtr->IsArrayClass();
    auto itable = classPtr->GetITable().Get();
    if (hasOwnItable && !itable.Empty()) {
        for (size_t i = 0; i < itable.Size(); i++) {
            Span<Method *> imethods = itable[i].GetMethods();
            if (!imethods.Empty()) {
                allocator_->Free(imethods.begin());
            }
        }
        allocator_->Free(itable.begin());
        classPtr->SetITable(ITable());
    }
    Span<Class *> interfaces = classPtr->GetInterfaces();
    if (!interfaces.Empty()) {
        allocator_->Free(interfaces.begin());
        classPtr->SetInterfaces(Span<Class *>());
    }
}

void ClassLinker::FreeClass(Class *classPtr)
{
    FreeClassData(classPtr);
    GetExtension(classPtr->GetSourceLang())->FreeClass(classPtr);
}

ClassLinker::~ClassLinker()
{
    for (auto &copiedName : copiedNames_) {
        allocator_->Free(reinterpret_cast<void *>(const_cast<uint8_t *>(copiedName)));
    }
}

ClassLinker::ClassLinker(mem::InternalAllocatorPtr allocator,
                         std::vector<std::unique_ptr<ClassLinkerExtension>> &&extensions)
    : allocator_(allocator), aotManager_(MakePandaUnique<AotManager>()), copiedNames_(allocator->Adapter())
{
    for (auto &ext : extensions) {
        extensions_[ark::panda_file::GetLangArrIndex(ext->GetLanguage())] = std::move(ext);
    }
}

void ClassLinker::ResetExtension(panda_file::SourceLang lang)
{
    extensions_[ark::panda_file::GetLangArrIndex(lang)] =
        Runtime::GetCurrent()->GetLanguageContext(lang).CreateClassLinkerExtension();
}

template <class T, class... Args>
static T *InitializeMemory(T *mem, Args... args)
{
    return new (mem) T(std::forward<Args>(args)...);
}

bool ClassLinker::Initialize(bool compressedStringEnabled)
{
    if (isInitialized_) {
        return true;
    }

    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        if (!ext->Initialize(this, compressedStringEnabled)) {
            return false;
        }
    }

    isInitialized_ = true;

    return true;
}

bool ClassLinker::InitializeRoots(ManagedThread *thread)
{
    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        if (!ext->InitializeRoots(thread)) {
            return false;
        }
    }

    return true;
}

using ClassEntry = std::pair<panda_file::File::EntityId, const panda_file::File *>;
using PandaFiles = PandaVector<const panda_file::File *>;

static ClassEntry FindClassInPandaFiles(const uint8_t *descriptor, const PandaFiles &pandaFiles)
{
    for (auto *pf : pandaFiles) {
        auto classId = pf->GetClassId(descriptor);
        if (classId.IsValid() && !pf->IsExternal(classId)) {
            return {classId, pf};
        }
    }

    return {};
}

Class *ClassLinker::FindLoadedClass(const uint8_t *descriptor, ClassLinkerContext *context)
{
    ASSERT(context != nullptr);
    return context->FindClass(descriptor);
}

template <class ClassDataAccessorT>
static size_t GetClassSize(ClassDataAccessorT dataAccessor, size_t vtableSize, size_t imtSize, size_t *outNumSfields)
{
    size_t num8bitSfields = 0;
    size_t num16bitSfields = 0;
    size_t num32bitSfields = 0;
    size_t num64bitSfields = 0;
    size_t numRefSfields = 0;
    size_t numTaggedSfields = 0;

    size_t numSfields = 0;

    dataAccessor.template EnumerateStaticFieldTypes([&num8bitSfields, &num16bitSfields, &num32bitSfields,
                                                     &num64bitSfields, &numRefSfields, &numTaggedSfields,
                                                     &numSfields](Type fieldType) {
        ++numSfields;

        switch (fieldType.GetId()) {
            case Type::TypeId::U1:
            case Type::TypeId::I8:
            case Type::TypeId::U8:
                ++num8bitSfields;
                break;
            case Type::TypeId::I16:
            case Type::TypeId::U16:
                ++num16bitSfields;
                break;
            case Type::TypeId::I32:
            case Type::TypeId::U32:
            case Type::TypeId::F32:
                ++num32bitSfields;
                break;
            case Type::TypeId::I64:
            case Type::TypeId::U64:
            case Type::TypeId::F64:
                ++num64bitSfields;
                break;
            case Type::TypeId::REFERENCE:
                ++numRefSfields;
                break;
            case Type::TypeId::TAGGED:
                ++numTaggedSfields;
                break;
            default:
                UNREACHABLE();
                break;
        }
    });

    *outNumSfields = numSfields;

    return Class::ComputeClassSize(vtableSize, imtSize, num8bitSfields, num16bitSfields, num32bitSfields,
                                   num64bitSfields, numRefSfields, numTaggedSfields);
}

class ClassDataAccessorWrapper {
public:
    explicit ClassDataAccessorWrapper(panda_file::ClassDataAccessor *dataAccessor = nullptr)
        : dataAccessor_(dataAccessor)
    {
    }

    template <class Callback>
    void EnumerateStaticFieldTypes(const Callback &cb) const
    {
        dataAccessor_->EnumerateFields([cb](panda_file::FieldDataAccessor &fda) {
            if (!fda.IsStatic()) {
                return;
            }

            cb(Type::GetTypeFromFieldEncoding(fda.GetType()));
        });
    }

    ~ClassDataAccessorWrapper() = default;

    DEFAULT_COPY_SEMANTIC(ClassDataAccessorWrapper);
    DEFAULT_MOVE_SEMANTIC(ClassDataAccessorWrapper);

private:
    panda_file::ClassDataAccessor *dataAccessor_;
};

void ClassLinker::FreeITableAndInterfaces(ITable itable, Span<Class *> &interfaces)
{
    auto table = itable.Get();
    if (!table.Empty()) {
        for (size_t i = 0; i < table.Size(); i++) {
            Span<Method *> imethods = table[i].GetMethods();
            if (!imethods.Empty()) {
                allocator_->Free(imethods.begin());
            }
            table[i].SetInterface(nullptr);
        }
        allocator_->Free(table.begin());
    }
    if (!interfaces.Empty()) {
        allocator_->Free(interfaces.begin());
    }
}

bool ClassLinker::SetupClassInfo(ClassLinker::ClassInfo &info, panda_file::ClassDataAccessor *dataAccessor, Class *base,
                                 Span<Class *> interfaces, ClassLinkerContext *context,
                                 ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(dataAccessor);

    info.vtableBuilder = ctx.CreateVTableBuilder(errorHandler);
    info.itableBuilder = ctx.CreateITableBuilder(errorHandler);
    info.imtableBuilder = ctx.CreateIMTableBuilder();

    if (!info.itableBuilder->Build(this, base, interfaces, dataAccessor->IsInterface())) {
        return false;
    }
    if (!info.vtableBuilder->Build(dataAccessor, base, info.itableBuilder->GetITable(), context)) {
        FreeITableAndInterfaces(info.itableBuilder->GetITable(), interfaces);
        return false;
    }
    info.imtableBuilder->Build(dataAccessor, info.itableBuilder->GetITable());

    ClassDataAccessorWrapper dataAccessorWrapper(dataAccessor);
    info.size = GetClassSize(dataAccessorWrapper, info.vtableBuilder->GetVTableSize(),
                             info.imtableBuilder->GetIMTSize(), &info.numSfields);
    return true;
}

class ClassDataAccessor {
public:
    explicit ClassDataAccessor(Span<Field> fields) : fields_(fields) {}

    template <class Callback>
    void EnumerateStaticFieldTypes(const Callback &cb) const
    {
        for (const auto &field : fields_) {
            if (!field.IsStatic()) {
                continue;
            }

            cb(field.GetType());
        }
    }

    ~ClassDataAccessor() = default;

    DEFAULT_COPY_SEMANTIC(ClassDataAccessor);
    DEFAULT_MOVE_SEMANTIC(ClassDataAccessor);

private:
    Span<Field> fields_;
};

bool ClassLinker::SetupClassInfo(ClassLinker::ClassInfo &info, Span<Method> methods, Span<Field> fields, Class *base,
                                 Span<Class *> interfaces, bool isInterface, ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*base);

    info.vtableBuilder = ctx.CreateVTableBuilder(errorHandler);
    info.itableBuilder = ctx.CreateITableBuilder(errorHandler);
    info.imtableBuilder = ctx.CreateIMTableBuilder();

    if (!info.itableBuilder->Build(this, base, interfaces, isInterface)) {
        return false;
    }
    if (!info.vtableBuilder->Build(methods, base, info.itableBuilder->GetITable(), isInterface)) {
        FreeITableAndInterfaces(info.itableBuilder->GetITable(), interfaces);
        return false;
    }
    info.imtableBuilder->Build(info.itableBuilder->GetITable(), isInterface);

    ClassDataAccessor dataAccessor(fields);
    info.size = GetClassSize(dataAccessor, info.vtableBuilder->GetVTableSize(), info.imtableBuilder->GetIMTSize(),
                             &info.numSfields);
    return true;
}

static void LoadMethod(Method *method, panda_file::MethodDataAccessor *methodDataAccessor, Class *klass,
                       const LanguageContext &ctx, const ClassLinkerExtension *ext)
{
    const auto &pf = methodDataAccessor->GetPandaFile();
    panda_file::ProtoDataAccessor pda(pf, methodDataAccessor->GetProtoId());

    uint32_t accessFlags = methodDataAccessor->GetAccessFlags();

    auto *methodName = pf.GetStringData(methodDataAccessor->GetNameId()).data;
    if (utf::IsEqual(methodName, ctx.GetCtorName()) || utf::IsEqual(methodName, ctx.GetCctorName())) {
        accessFlags |= ACC_CONSTRUCTOR;
    }

    auto codeId = methodDataAccessor->GetCodeId();
    size_t numArgs = (methodDataAccessor->IsStatic()) ? pda.GetNumArgs() : (pda.GetNumArgs() + 1);

    if (!codeId.has_value()) {
        InitializeMemory(method, klass, &pf, methodDataAccessor->GetMethodId(), panda_file::File::EntityId(0),
                         accessFlags, numArgs, reinterpret_cast<const uint16_t *>(pda.GetShorty().Data()));

        if (methodDataAccessor->IsNative()) {
            method->SetCompiledEntryPoint(ext->GetNativeEntryPointFor(method));
        } else {
            method->SetInterpreterEntryPoint();
        }
    } else {
        InitializeMemory(method, klass, &pf, methodDataAccessor->GetMethodId(), codeId.value(), accessFlags, numArgs,
                         reinterpret_cast<const uint16_t *>(pda.GetShorty().Data()));
        method->SetCompiledEntryPoint(GetCompiledCodeToInterpreterBridge(method));
    }
}

static void MaybeLinkMethodToAotCode(Method *method, const compiler::AotClass &aotClass, size_t methodIndex)
{
    ASSERT(aotClass.IsValid());
    if (method->IsIntrinsic()) {
        return;
    }
    auto entry = aotClass.FindMethodCodeEntry(methodIndex);
    if (entry != nullptr) {
        method->SetCompiledEntryPoint(entry);
        LOG(INFO, AOT) << "Found AOT entrypoint ["
                       << reinterpret_cast<const void *>(aotClass.FindMethodCodeSpan(methodIndex).data()) << ":"
                       << reinterpret_cast<const void *>(aotClass.FindMethodCodeSpan(methodIndex).end())
                       << "] for method: " << method->GetFullName();

        EVENT_AOT_ENTRYPOINT_FOUND(method->GetFullName());
        ASSERT(aotClass.FindMethodHeader(methodIndex)->methodId == method->GetFileId().GetOffset());
    }
}

static void SetupCopiedMethods(Span<Method> methods, Span<const CopiedMethod> copiedMethods)
{
    size_t const numMethods = methods.size() - copiedMethods.size();

    for (size_t i = 0; i < copiedMethods.size(); i++) {
        Method *method = &methods[numMethods + i];
        InitializeMemory(method, copiedMethods[i].GetMethod());
        method->SetIsDefaultInterfaceMethod();
        switch (copiedMethods[i].GetStatus()) {
            case CopiedMethod::Status::ORDINARY:
                break;
            case CopiedMethod::Status::ABSTRACT:
                method->SetCompiledEntryPoint(GetAbstractMethodStub());
                break;
            case CopiedMethod::Status::CONFLICT:
                method->SetCompiledEntryPoint(GetDefaultConflictMethodStub());
                break;
        }
    }
}

bool ClassLinker::LoadMethods(Class *klass, ClassInfo *classInfo, panda_file::ClassDataAccessor *dataAccessor,
                              [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    uint32_t numMethods = dataAccessor->GetMethodsNumber();

    uint32_t numVmethods = klass->GetNumVirtualMethods();
    uint32_t numSmethods = numMethods - numVmethods;

    auto copiedMethods = classInfo->vtableBuilder->GetCopiedMethods();
    uint32_t totalNumMethods = numMethods + copiedMethods.size();
    if (totalNumMethods == 0) {
        return true;
    }

    Span<Method> methods {allocator_->AllocArray<Method>(totalNumMethods), totalNumMethods};

    size_t smethodIdx = numVmethods;
    size_t vmethodIdx = 0;

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);

    auto aotPfile = aotManager_->FindPandaFile(klass->GetPandaFile()->GetFullFileName());
    if (aotPfile != nullptr) {
        EVENT_AOT_LOADED_FOR_CLASS(PandaString(aotPfile->GetFileName()), PandaString(klass->GetName()));
    }

    compiler::AotClass aotClass =
        (aotPfile != nullptr) ? aotPfile->GetClass(klass->GetFileId().GetOffset()) : compiler::AotClass::Invalid();

    size_t methodIndex = 0;
    dataAccessor->EnumerateMethods([klass, &smethodIdx, &vmethodIdx, &methods, aotClass, ctx, ext,
                                    &methodIndex](panda_file::MethodDataAccessor &methodDataAccessor) {
        Method *method = methodDataAccessor.IsStatic() ? &methods[smethodIdx++] : &methods[vmethodIdx++];
        LoadMethod(method, &methodDataAccessor, klass, ctx, ext);
        if (aotClass.IsValid()) {
            MaybeLinkMethodToAotCode(method, aotClass, methodIndex);
        }
        // Instead of checking if the method is abstract before every virtual call
        // the special stub throwing AbstractMethodError is registered as compiled entry point.
        if (method->IsAbstract()) {
            method->SetCompiledEntryPoint(GetAbstractMethodStub());
        }
        methodIndex++;
    });

    SetupCopiedMethods(methods, copiedMethods);
    klass->SetMethods(methods, numVmethods, numSmethods);
    return true;
}

bool ClassLinker::LoadFields(Class *klass, panda_file::ClassDataAccessor *dataAccessor,
                             [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    uint32_t numFields = dataAccessor->GetFieldsNumber();
    if (numFields == 0) {
        return true;
    }

    uint32_t numSfields = klass->GetNumStaticFields();

    Span<Field> fields {allocator_->AllocArray<Field>(numFields), numFields};

    size_t sfieldsIdx = 0;
    size_t ifieldsIdx = numSfields;
    dataAccessor->EnumerateFields(
        [klass, &sfieldsIdx, &ifieldsIdx, &fields](panda_file::FieldDataAccessor &fieldDataAccessor) {
            Field *field = fieldDataAccessor.IsStatic() ? &fields[sfieldsIdx++] : &fields[ifieldsIdx++];
            InitializeMemory(field, klass, fieldDataAccessor.GetFieldId(), fieldDataAccessor.GetAccessFlags(),
                             panda_file::Type::GetTypeFromFieldEncoding(fieldDataAccessor.GetType()));
        });

    klass->SetFields(fields, numSfields);

    return true;
}

template <bool REVERSE_LAYOUT = false>
static void LayoutFieldsWithoutAlignment(size_t size, size_t *offset, size_t *space, PandaVector<Field *> *fields)
{
    auto lastProceededElement = fields->end();
    // Iterating from beginning to end and erasing elements from the beginning of a vector
    // is required for correct field layout between language class representation in C++ code
    // and generated by methods in class linker.
    // (e.g. class String should have length field before hash field, not vice versa)
    for (auto i = fields->begin(); i != fields->end(); i++) {
        if (!(space == nullptr || *space >= size)) {
            lastProceededElement = i;
            break;
        }
        Field *field = *i;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (REVERSE_LAYOUT) {
            *offset -= size;
            field->SetOffset(*offset);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            field->SetOffset(*offset);
            *offset += size;
        }
        if (space != nullptr) {
            *space -= size;
        }
    }
    fields->erase(fields->begin(), lastProceededElement);
}

static uint32_t LayoutReferenceFields(size_t size, size_t *offset, const PandaVector<Field *> &fields)
{
    uint32_t volatileFieldsNum = 0;
    // layout volatile fields firstly
    for (auto *field : fields) {
        if (field->IsVolatile()) {
            volatileFieldsNum++;
            field->SetOffset(*offset);
            *offset += size;
        }
    }
    for (auto *field : fields) {
        if (!field->IsVolatile()) {
            field->SetOffset(*offset);
            *offset += size;
        }
    }
    return volatileFieldsNum;
}

constexpr size_t SIZE_64 = sizeof(uint64_t);
constexpr size_t SIZE_32 = sizeof(uint32_t);
constexpr size_t SIZE_16 = sizeof(uint16_t);
constexpr size_t SIZE_8 = sizeof(uint8_t);

// CC-OFFNXT(G.FUN.01) solid logic
static size_t LayoutFieldsInBaseClassPadding(Class *klass, PandaVector<Field *> *taggedFields,
                                             PandaVector<Field *> *fields64, PandaVector<Field *> *fields32,
                                             PandaVector<Field *> *fields16, PandaVector<Field *> *fields8,
                                             PandaVector<Field *> *refFields, bool isStatic)
{
    size_t offset;

    if (isStatic) {
        offset = klass->GetStaticFieldsOffset();
    } else {
        offset = (klass->GetBase() != nullptr) ? klass->GetBase()->GetObjectSize() : ObjectHeader::ObjectHeaderSize();
    }

    size_t alignOffset = offset;
    if (!refFields->empty()) {
        alignOffset = AlignUp(offset, ClassHelper::OBJECT_POINTER_SIZE);
    } else if (!(fields64->empty()) || !(taggedFields->empty())) {
        alignOffset = AlignUp(offset, SIZE_64);
    } else if (!fields32->empty()) {
        alignOffset = AlignUp(offset, SIZE_32);
    } else if (!fields16->empty()) {
        alignOffset = AlignUp(offset, SIZE_16);
    }
    if (alignOffset != offset) {
        size_t endOffset = alignOffset;
        size_t padding = endOffset - offset;
        // try to put short fields of child class at end of free space of base class
        LayoutFieldsWithoutAlignment<true>(SIZE_32, &endOffset, &padding, fields32);
        LayoutFieldsWithoutAlignment<true>(SIZE_16, &endOffset, &padding, fields16);
        LayoutFieldsWithoutAlignment<true>(SIZE_8, &endOffset, &padding, fields8);
    }
    return alignOffset;
}

// CC-OFFNXT(G.FUN.01) solid logic
static size_t LayoutFields(Class *klass, PandaVector<Field *> *taggedFields, PandaVector<Field *> *fields64,
                           PandaVector<Field *> *fields32, PandaVector<Field *> *fields16,
                           PandaVector<Field *> *fields8, PandaVector<Field *> *refFields, bool isStatic)
{
    size_t offset =
        LayoutFieldsInBaseClassPadding(klass, taggedFields, fields64, fields32, fields16, fields8, refFields, isStatic);
    if (!refFields->empty()) {
        offset = AlignUp(offset, ClassHelper::OBJECT_POINTER_SIZE);
        klass->SetRefFieldsNum(refFields->size(), isStatic);
        klass->SetRefFieldsOffset(offset, isStatic);
        auto volatileNum = LayoutReferenceFields(ClassHelper::OBJECT_POINTER_SIZE, &offset, *refFields);
        klass->SetVolatileRefFieldsNum(volatileNum, isStatic);
    }

    static_assert(coretypes::TaggedValue::TaggedTypeSize() == SIZE_64,
                  "Please fix alignment of the fields of type \"TaggedValue\"");
    if (!IsAligned<SIZE_64>(offset) && (!fields64->empty() || !taggedFields->empty())) {
        size_t padding = AlignUp(offset, SIZE_64) - offset;

        LayoutFieldsWithoutAlignment(SIZE_32, &offset, &padding, fields32);
        LayoutFieldsWithoutAlignment(SIZE_16, &offset, &padding, fields16);
        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(coretypes::TaggedValue::TaggedTypeSize(), &offset, nullptr, taggedFields);
    LayoutFieldsWithoutAlignment(SIZE_64, &offset, nullptr, fields64);

    if (!IsAligned<SIZE_32>(offset) && !fields32->empty()) {
        size_t padding = AlignUp(offset, SIZE_32) - offset;

        LayoutFieldsWithoutAlignment(SIZE_16, &offset, &padding, fields16);
        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(SIZE_32, &offset, nullptr, fields32);

    if (!IsAligned<SIZE_16>(offset) && !fields16->empty()) {
        size_t padding = AlignUp(offset, SIZE_16) - offset;

        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(SIZE_16, &offset, nullptr, fields16);

    LayoutFieldsWithoutAlignment(SIZE_8, &offset, nullptr, fields8);

    return offset;
}

static inline bool PushBackFieldIfNonPrimitiveType(Field &field, PandaVector<Field *> &refFields)
{
    auto type = field.GetType();
    if (!type.IsPrimitive()) {
        refFields.push_back(&field);
        return true;
    }

    return false;
}

/* static */
// CC-OFFNXT(huge_method) solid logic
bool ClassLinker::LayoutFields(Class *klass, Span<Field> fields, bool isStatic,
                               [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    // These containers must be optimized
    PandaVector<Field *> taggedFields;
    PandaVector<Field *> fields64;
    PandaVector<Field *> fields32;
    PandaVector<Field *> fields16;
    PandaVector<Field *> fields8;
    PandaVector<Field *> refFields;
    taggedFields.reserve(fields.size());
    fields64.reserve(fields.size());
    fields32.reserve(fields.size());
    fields16.reserve(fields.size());
    fields8.reserve(fields.size());
    refFields.reserve(fields.size());

    for (auto &field : fields) {
        if (PushBackFieldIfNonPrimitiveType(field, refFields)) {
            continue;
        }

        switch (field.GetType().GetId()) {
            case Type::TypeId::U1:
            case Type::TypeId::I8:
            case Type::TypeId::U8:
                fields8.push_back(&field);
                break;
            case Type::TypeId::I16:
            case Type::TypeId::U16:
                fields16.push_back(&field);
                break;
            case Type::TypeId::I32:
            case Type::TypeId::U32:
            case Type::TypeId::F32:
                fields32.push_back(&field);
                break;
            case Type::TypeId::I64:
            case Type::TypeId::U64:
            case Type::TypeId::F64:
                fields64.push_back(&field);
                break;
            case Type::TypeId::TAGGED:
                taggedFields.push_back(&field);
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    size_t size =
        ark::LayoutFields(klass, &taggedFields, &fields64, &fields32, &fields16, &fields8, &refFields, isStatic);

    if (!isStatic && !klass->IsVariableSize()) {
        klass->SetObjectSize(size);
    }

    return true;
}

bool ClassLinker::LinkMethods(Class *klass, ClassInfo *classInfo,
                              [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    classInfo->vtableBuilder->UpdateClass(klass);
    if (!classInfo->itableBuilder->Resolve(klass)) {
        return false;
    }
    classInfo->itableBuilder->UpdateClass(klass);
    classInfo->imtableBuilder->UpdateClass(klass);
    return true;
}

bool ClassLinker::LinkFields(Class *klass, ClassLinkerErrorHandler *errorHandler)
{
    if (!LayoutFields(klass, klass->GetStaticFields(), true, errorHandler)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot layout static fields of class '" << klass->GetName() << "'";
        return false;
    }

    if (!LayoutFields(klass, klass->GetInstanceFields(), false, errorHandler)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot layout instance fields of class '" << klass->GetName() << "'";
        return false;
    }

    return true;
}

Class *ClassLinker::LoadBaseClass(panda_file::ClassDataAccessor *cda, const LanguageContext &ctx,
                                  ClassLinkerContext *context, ClassLinkerErrorHandler *errorHandler)
{
    auto baseClassId = cda->GetSuperClassId();
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);
    if (baseClassId.GetOffset() == 0) {
        return ext->GetClassRoot(ClassRoot::OBJECT);
    }

    auto &pf = cda->GetPandaFile();
    auto *baseClass = ext->GetClass(pf, baseClassId, context, errorHandler);
    if (baseClass == nullptr) {
        LOG(INFO, CLASS_LINKER) << "Cannot find base class '" << utf::Mutf8AsCString(pf.GetStringData(baseClassId).data)
                                << "' of class '" << utf::Mutf8AsCString(pf.GetStringData(cda->GetClassId()).data)
                                << "' in ctx " << context;
        return nullptr;
    }

    return baseClass;
}

std::optional<Span<Class *>> ClassLinker::LoadInterfaces(panda_file::ClassDataAccessor *cda,
                                                         ClassLinkerContext *context,
                                                         ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(context != nullptr);
    size_t ifacesNum = cda->GetIfacesNumber();
    if (ifacesNum == 0) {
        return Span<Class *>(nullptr, ifacesNum);
    }

    Span<Class *> ifaces {allocator_->AllocArray<Class *>(ifacesNum), ifacesNum};

    for (size_t i = 0; i < ifacesNum; i++) {
        auto id = cda->GetInterfaceId(i);
        auto &pf = cda->GetPandaFile();
        auto *iface = GetClass(pf, id, context, errorHandler);
        if (iface == nullptr) {
            LOG(INFO, CLASS_LINKER) << "Cannot find interface '" << utf::Mutf8AsCString(pf.GetStringData(id).data)
                                    << "' of class '" << utf::Mutf8AsCString(pf.GetStringData(cda->GetClassId()).data)
                                    << "' in ctx " << context;
            ASSERT(!ifaces.Empty());
            allocator_->Free(ifaces.begin());
            return {};
        }

        ifaces[i] = iface;
    }

    return ifaces;
}

using ClassLoadingSet = std::unordered_set<uint64_t>;

// This class is required to clear static unordered_set on return
class ClassScopeStaticSetAutoCleaner {
public:
    ClassScopeStaticSetAutoCleaner() = default;
    explicit ClassScopeStaticSetAutoCleaner(ClassLoadingSet *setPtr, ClassLoadingSet **tlSetPtr)
        : setPtr_(setPtr), tlSetPtr_(tlSetPtr)
    {
    }
    ~ClassScopeStaticSetAutoCleaner()
    {
        setPtr_->clear();
        if (tlSetPtr_ != nullptr) {
            *tlSetPtr_ = nullptr;
        }
    }

    NO_COPY_SEMANTIC(ClassScopeStaticSetAutoCleaner);
    NO_MOVE_SEMANTIC(ClassScopeStaticSetAutoCleaner);

private:
    ClassLoadingSet *setPtr_ {nullptr};
    ClassLoadingSet **tlSetPtr_ {nullptr};
};

static uint64_t GetClassUniqueHash(uint32_t pandaFileHash, uint32_t classId)
{
    const uint8_t bitsToShuffle = 32;
    return (static_cast<uint64_t>(pandaFileHash) << bitsToShuffle) | static_cast<uint64_t>(classId);
}

Class *ClassLinker::LoadClass(panda_file::ClassDataAccessor *classDataAccessor, const uint8_t *descriptor,
                              Class *baseClass, Span<Class *> interfaces, ClassLinkerContext *context,
                              ClassLinkerExtension *ext, ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(context != nullptr);
    ClassInfo classInfo {};
    if (!SetupClassInfo(classInfo, classDataAccessor, baseClass, interfaces, context, errorHandler)) {
        return nullptr;
    }

    auto *klass = ext->CreateClass(descriptor, classInfo.vtableBuilder->GetVTableSize(),
                                   classInfo.imtableBuilder->GetIMTSize(), classInfo.size);

    if (UNLIKELY(klass == nullptr)) {
        return nullptr;
    }

    klass->SetLoadContext(context);
    klass->SetBase(baseClass);
    klass->SetInterfaces(interfaces);
    klass->SetFileId(classDataAccessor->GetClassId());
    klass->SetPandaFile(&classDataAccessor->GetPandaFile());
    klass->SetAccessFlags(classDataAccessor->GetAccessFlags());

    auto &pf = classDataAccessor->GetPandaFile();
    auto classId = classDataAccessor->GetClassId();
    klass->SetClassIndex(pf.GetClassIndex(classId));
    klass->SetMethodIndex(pf.GetMethodIndex(classId));
    klass->SetFieldIndex(pf.GetFieldIndex(classId));

    klass->SetNumVirtualMethods(classInfo.vtableBuilder->GetNumVirtualMethods());
    klass->SetNumCopiedMethods(classInfo.vtableBuilder->GetCopiedMethods().size());
    klass->SetNumStaticFields(classInfo.numSfields);

    auto const onFail = [this, descriptor, klass](std::string_view msg) {
        FreeClass(klass);
        LOG(ERROR, CLASS_LINKER) << msg << " '" << descriptor << "'";
        return nullptr;
    };
    if (!LoadMethods(klass, &classInfo, classDataAccessor, errorHandler)) {
        return onFail("Cannot load methods of class");
    }
    if (!LoadFields(klass, classDataAccessor, errorHandler)) {
        return onFail("Cannot load fields of class");
    }
    if (!LinkMethods(klass, &classInfo, errorHandler)) {
        return onFail("Cannot link methods of class");
    }
    if (!LinkFields(klass, errorHandler)) {
        return onFail("Cannot link fields of class");
    }
    return klass;
}

Class *ClassLinker::LoadClass(const panda_file::File *pf, const uint8_t *descriptor, panda_file::SourceLang lang)
{
    panda_file::File::EntityId classId = pf->GetClassId(descriptor);
    if (!classId.IsValid() || pf->IsExternal(classId)) {
        return nullptr;
    }
    ClassLinkerContext *context = GetExtension(lang)->GetBootContext();
    return LoadClass(pf, classId, descriptor, context, nullptr);
}

static void OnError(ClassLinkerErrorHandler *errorHandler, ClassLinker::Error error, const PandaString &msg)
{
    if (errorHandler != nullptr) {
        errorHandler->OnError(error, msg);
    }
}

static bool TryInsertClassLoading(panda_file::File::EntityId &classId, const panda_file::File *pf,
                                  panda_file::ClassDataAccessor &classDataAccessor, ClassLoadingSet *threadLocalSet,
                                  ClassLinkerErrorHandler *errorHandler)
{
    uint32_t classIdInt = classId.GetOffset();
    uint32_t pandaFileHash = pf->GetFilenameHash();
    if (!threadLocalSet->insert(GetClassUniqueHash(pandaFileHash, classIdInt)).second) {
        const PandaString &className = utf::Mutf8AsCString(pf->GetStringData(classDataAccessor.GetClassId()).data);
        PandaString msg = "Class or interface \"" + className + "\" is its own superclass or superinterface";
        OnError(errorHandler, ClassLinker::Error::CLASS_CIRCULARITY, msg);
        return false;
    }

    return true;
}

static bool IsContextCanBeLoaded(ClassLinkerContext *context, panda_file::ClassDataAccessor &classDataAccessor,
                                 const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&classDataAccessor);
    if (ctx.GetLanguage() != context->GetSourceLang()) {
        LanguageContext currentCtx = Runtime::GetCurrent()->GetLanguageContext(context->GetSourceLang());
        PandaStringStream ss;
        ss << "Cannot load " << ctx << " class " << descriptor << " into " << currentCtx << " context";
        LOG(ERROR, CLASS_LINKER) << ss.str();
        OnError(errorHandler, ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
        return false;
    }

    return true;
}

static void HandleNoExtensionError(LanguageContext &ctx, const uint8_t *descriptor,
                                   ClassLinkerErrorHandler *errorHandler)
{
    PandaStringStream ss;
    ss << "Cannot load class '" << descriptor << "' as class linker hasn't " << ctx << " language extension";
    LOG(ERROR, CLASS_LINKER) << ss.str();
    OnError(errorHandler, ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
Class *ClassLinker::LoadClass(const panda_file::File *pf, panda_file::File::EntityId classId, const uint8_t *descriptor,
                              ClassLinkerContext *context, ClassLinkerErrorHandler *errorHandler,
                              bool addToRuntime /* = true */)
{
    ASSERT(!pf->IsExternal(classId));
    ASSERT(context != nullptr);
    panda_file::ClassDataAccessor classDataAccessor(*pf, classId);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&classDataAccessor);
    if (!IsContextCanBeLoaded(context, classDataAccessor, descriptor, errorHandler)) {
        return nullptr;
    }

    if (!HasExtension(ctx)) {
        HandleNoExtensionError(ctx, descriptor, errorHandler);
        return nullptr;
    }

    // This set is used to find out if the class is its own superclass
    ClassLoadingSet loadingSet;
    static thread_local ClassLoadingSet *threadLocalSet = nullptr;
    ClassLoadingSet **threadLocalSetPtr = nullptr;
    if (threadLocalSet == nullptr) {
        threadLocalSet = &loadingSet;
        threadLocalSetPtr = &threadLocalSet;
    }
    ClassScopeStaticSetAutoCleaner classSetAutoCleanerOnReturn(threadLocalSet, threadLocalSetPtr);

    auto *ext = GetExtension(ctx);
    Class *baseClass = nullptr;
    bool needLoadBase = IsInitialized() || !utf::IsEqual(ctx.GetObjectClassDescriptor(), descriptor);
    if (needLoadBase) {
        if (!TryInsertClassLoading(classId, pf, classDataAccessor, threadLocalSet, errorHandler)) {
            return nullptr;
        }

        baseClass = LoadBaseClass(&classDataAccessor, ctx, context, errorHandler);
        if (baseClass == nullptr) {
            LOG(INFO, CLASS_LINKER) << "Cannot load base class of class '" << descriptor << "'";
            return nullptr;
        }
    }

    auto res = LoadInterfaces(&classDataAccessor, context, errorHandler);
    if (!res) {
        LOG(INFO, CLASS_LINKER) << "Cannot load interfaces of class '" << descriptor << "'";
        return nullptr;
    }

    auto *klass = LoadClass(&classDataAccessor, descriptor, baseClass, res.value(), context, ext, errorHandler);
    if (klass == nullptr) {
        return nullptr;
    }

    Runtime::GetCurrent()->GetCha()->Update(klass);

    if (LIKELY(ext->CanInitializeClasses())) {
        if (!ext->InitializeClass(klass)) {
            LOG(ERROR, CLASS_LINKER) << "Language specific initialization for class '" << descriptor << "' failed";
            FreeClass(klass);
            return nullptr;
        }
        klass->SetState(Class::State::LOADED);
    }

    if (LIKELY(addToRuntime)) {
        Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(klass);

        auto *otherKlass = context->InsertClass(klass);
        if (otherKlass != nullptr) {
            // Someone has created the class in the other thread (increase the critical section?)
            FreeClass(klass);
            return otherKlass;
        }

        RemoveCreatedClassInExtension(klass);
        Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(klass);
    }
    return klass;
}

static const uint8_t *CopyMutf8String(mem::InternalAllocatorPtr allocator, const uint8_t *descriptor)
{
    size_t size = utf::Mutf8Size(descriptor) + 1;  // + 1 - null terminate
    auto *ptr = allocator->AllocArray<uint8_t>(size);
    memcpy_s(ptr, size, descriptor, size);
    return ptr;
}

bool ClassLinker::LinkEntitiesAndInitClass(Class *klass, ClassInfo *classInfo, ClassLinkerExtension *ext,
                                           const uint8_t *descriptor)
{
    if (!LinkMethods(klass, classInfo, ext->GetErrorHandler())) {
        LOG(ERROR, CLASS_LINKER) << "Cannot link class methods '" << descriptor << "'";
        return false;
    }

    if (!LinkFields(klass, ext->GetErrorHandler())) {
        LOG(ERROR, CLASS_LINKER) << "Cannot link class fields '" << descriptor << "'";
        return false;
    }

    if (!ext->InitializeClass(klass)) {
        LOG(ERROR, CLASS_LINKER) << "Language specific initialization for class '" << descriptor << "' failed";
        FreeClass(klass);
        return false;
    }

    return true;
}

Class *ClassLinker::BuildClass(const uint8_t *descriptor, bool needCopyDescriptor, uint32_t accessFlags,
                               Span<Method> methods, Span<Field> fields, Class *baseClass, Span<Class *> interfaces,
                               ClassLinkerContext *context, bool isInterface)
{
    ASSERT(context != nullptr);
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *ext = GetExtension(baseClass->GetSourceLang());
    ASSERT(ext != nullptr);

    ClassInfo classInfo {};
    if (!SetupClassInfo(classInfo, methods, fields, baseClass, interfaces, isInterface, nullptr)) {
        return nullptr;
    }

    // Need to protect ArenaAllocator and loaded_classes_
    auto *klass = ext->CreateClass(descriptor, classInfo.vtableBuilder->GetVTableSize(),
                                   classInfo.imtableBuilder->GetIMTSize(), classInfo.size);

    if (UNLIKELY(klass == nullptr)) {
        return nullptr;
    }

    klass->SetLoadContext(context);
    klass->SetBase(baseClass);
    klass->SetInterfaces(interfaces);
    klass->SetAccessFlags(accessFlags);

    klass->SetNumVirtualMethods(classInfo.vtableBuilder->GetNumVirtualMethods());
    klass->SetNumCopiedMethods(classInfo.vtableBuilder->GetCopiedMethods().size());
    klass->SetNumStaticFields(classInfo.numSfields);

    ASSERT(klass->GetNumCopiedMethods() == 0);

    size_t numSmethods = methods.size() - klass->GetNumVirtualMethods();
    klass->SetMethods(methods, klass->GetNumVirtualMethods(), numSmethods);
    klass->SetFields(fields, klass->GetNumStaticFields());

    for (auto &method : methods) {
        method.SetClass(klass);
    }

    for (auto &field : fields) {
        field.SetClass(klass);
    }

    if (!LinkEntitiesAndInitClass(klass, &classInfo, ext, descriptor)) {
        return nullptr;
    }

    klass->SetState(Class::State::LOADED);

    Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(klass);

    auto *otherKlass = context->InsertClass(klass);
    if (otherKlass != nullptr) {
        // Someone has created the class in the other thread (increase the critical section?)
        FreeClass(klass);
        return otherKlass;
    }

    RemoveCreatedClassInExtension(klass);
    Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(klass);

    return klass;
}

Class *ClassLinker::CreateArrayClass(ClassLinkerExtension *ext, const uint8_t *descriptor, bool needCopyDescriptor,
                                     Class *componentClass)
{
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *arrayClass = ext->CreateClass(descriptor, ext->GetArrayClassVTableSize(), ext->GetArrayClassIMTSize(),
                                        ext->GetArrayClassSize());

    if (UNLIKELY(arrayClass == nullptr)) {
        return nullptr;
    }

    arrayClass->SetLoadContext(componentClass->GetLoadContext());

    if (UNLIKELY(!ext->InitializeArrayClass(arrayClass, componentClass))) {
        return nullptr;
    }

    return arrayClass;
}

Class *ClassLinker::LoadArrayClass(const uint8_t *descriptor, bool needCopyDescriptor, ClassLinkerContext *context,
                                   ClassLinkerErrorHandler *errorHandler)
{
    Span<const uint8_t> sp(descriptor, 1);

    Class *componentClass = GetClass(sp.cend(), needCopyDescriptor, context, errorHandler);

    if (componentClass == nullptr) {
        return nullptr;
    }

    if (UNLIKELY(componentClass->GetType().GetId() == panda_file::Type::TypeId::VOID)) {
        OnError(errorHandler, Error::NO_CLASS_DEF, "Try to create array with void component type");
        return nullptr;
    }

    auto *ext = GetExtension(componentClass->GetSourceLang());
    ASSERT(ext != nullptr);

    auto *componentClassContext = componentClass->GetLoadContext();
    ASSERT(componentClassContext != nullptr);
    if (componentClassContext != context) {
        auto *loadedClass = FindLoadedClass(descriptor, componentClassContext);
        if (loadedClass != nullptr) {
            return loadedClass;
        }
    }

    auto *arrayClass = CreateArrayClass(ext, descriptor, needCopyDescriptor, componentClass);

    if (UNLIKELY(arrayClass == nullptr)) {
        return nullptr;
    }

    Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(arrayClass);

    auto *otherKlass = componentClassContext->InsertClass(arrayClass);
    if (otherKlass != nullptr) {
        FreeClass(arrayClass);
        return otherKlass;
    }

    RemoveCreatedClassInExtension(arrayClass);
    Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(arrayClass);

    return arrayClass;
}

static PandaString PandaFilesToString(const PandaVector<const panda_file::File *> &pandaFiles)
{
    PandaStringStream ss;
    ss << "[";

    size_t n = pandaFiles.size();
    for (size_t i = 0; i < n; i++) {
        ss << pandaFiles[i]->GetFilename();

        if (i < n - 1) {
            ss << ", ";
        }
    }

    ss << "]";
    return ss.str();
}

Class *ClassLinker::GetClass(const uint8_t *descriptor, bool needCopyDescriptor, ClassLinkerContext *context,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(context != nullptr);
    ASSERT(!MTManagedThread::ThreadIsMTManagedThread(Thread::GetCurrent()) ||
           !PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    Class *cls = FindLoadedClass(descriptor, context);
    if (cls != nullptr) {
        return cls;
    }

    if (ClassHelper::IsArrayDescriptor(descriptor)) {
        return LoadArrayClass(descriptor, needCopyDescriptor, context, errorHandler);
    }

    if (context->IsBootContext()) {
        panda_file::File::EntityId classId;
        const panda_file::File *pandaFile {nullptr};
        {
            {
                os::memory::LockHolder lock {bootPandaFilesLock_};
                std::tie(classId, pandaFile) = FindClassInPandaFiles(descriptor, bootPandaFiles_);
            }

            if (!classId.IsValid()) {
                PandaStringStream ss;
                {
                    // can't make a wider scope for lock here - will get recursion
                    os::memory::LockHolder lock {bootPandaFilesLock_};
                    ss << "Cannot find class " << descriptor
                       << " in boot panda files: " << PandaFilesToString(bootPandaFiles_);
                }
                OnError(errorHandler, Error::CLASS_NOT_FOUND, ss.str());
                return nullptr;
            }
        }

        return LoadClass(pandaFile, classId, pandaFile->GetStringData(classId).data, context, errorHandler);
    }

    return context->LoadClass(descriptor, needCopyDescriptor, errorHandler);
}

Class *ClassLinker::GetClass(const panda_file::File &pf, panda_file::File::EntityId id, ClassLinkerContext *context,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(context != nullptr);
    ASSERT(!MTManagedThread::ThreadIsMTManagedThread(Thread::GetCurrent()) ||
           !PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    Class *cls = pf.GetPandaCache()->GetClassFromCache(id);
    if (cls != nullptr) {
        return cls;
    }
    const uint8_t *descriptor = pf.GetStringData(id).data;

    cls = FindLoadedClass(descriptor, context);
    if (cls != nullptr) {
        pf.GetPandaCache()->SetClassCache(id, cls);
        return cls;
    }

    if (ClassHelper::IsArrayDescriptor(descriptor)) {
        cls = LoadArrayClass(descriptor, false, context, errorHandler);
        if (LIKELY(cls != nullptr)) {
            pf.GetPandaCache()->SetClassCache(id, cls);
        }
        return cls;
    }

    if (context->IsBootContext()) {
        const panda_file::File *pfPtr = nullptr;
        panda_file::File::EntityId extId;
        {
            os::memory::LockHolder lock {bootPandaFilesLock_};
            std::tie(extId, pfPtr) = FindClassInPandaFiles(descriptor, bootPandaFiles_);
        }

        if (!extId.IsValid()) {
            PandaStringStream ss;
            {
                // can't make a wider scope for lock here - will get recursion
                os::memory::LockHolder lock {bootPandaFilesLock_};
                ss << "Cannot find class " << descriptor
                   << " in boot panda files: " << PandaFilesToString(bootPandaFiles_);
            }
            OnError(errorHandler, Error::CLASS_NOT_FOUND, ss.str());
            return nullptr;
        }

        cls = LoadClass(pfPtr, extId, descriptor, context, errorHandler);
        if (LIKELY(cls != nullptr)) {
            pf.GetPandaCache()->SetClassCache(id, cls);
        }
        return cls;
    }

    return context->LoadClass(descriptor, false, errorHandler);
}

Method *ClassLinker::GetMethod(const panda_file::File &pf, panda_file::File::EntityId id,
                               ClassLinkerContext *context /* = nullptr */,
                               ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Method *method = pf.GetPandaCache()->GetMethodFromCache(id);
    if (method != nullptr) {
        return method;
    }
    panda_file::MethodDataAccessor methodDataAccessor(pf, id);

    auto classId = methodDataAccessor.GetClassId();
    if (context == nullptr) {
        panda_file::ClassDataAccessor classDataAccessor(pf, classId);
        auto lang = classDataAccessor.GetSourceLang();
        if (!lang) {
            LOG(INFO, CLASS_LINKER) << "Cannot resolve language context for class_id " << classId << " in file "
                                    << pf.GetFilename();
            return nullptr;
        }
        auto *extension = GetExtension(lang.value());
        context = extension->GetBootContext();
    }

    Class *klass = GetClass(pf, classId, context, errorHandler);

    if (klass == nullptr) {
        auto className = pf.GetStringData(classId).data;
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }
    method = GetMethod(klass, methodDataAccessor, errorHandler);
    if (LIKELY(method != nullptr)) {
        pf.GetPandaCache()->SetMethodCache(id, method);
    }
    return method;
}

Method *ClassLinker::GetMethod(const Method &caller, panda_file::File::EntityId id,
                               ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    auto *pf = caller.GetPandaFile();
    Method *method = pf->GetPandaCache()->GetMethodFromCache(id);
    if (method != nullptr) {
        return method;
    }

    panda_file::MethodDataAccessor methodDataAccessor(*pf, id);
    auto classId = methodDataAccessor.GetClassId();

    auto *context = caller.GetClass()->GetLoadContext();
    auto *ext = GetExtension(caller.GetClass()->GetSourceLang());
    Class *klass = ext->GetClass(*pf, classId, context, errorHandler);

    if (klass == nullptr) {
        auto className = pf->GetStringData(classId).data;
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }

    method = GetMethod(klass, methodDataAccessor, (errorHandler == nullptr) ? ext->GetErrorHandler() : errorHandler);
    if (LIKELY(method != nullptr)) {
        pf->GetPandaCache()->SetMethodCache(id, method);
    }
    return method;
}

Method *ClassLinker::GetMethod(const Class *klass, const panda_file::MethodDataAccessor &methodDataAccessor,
                               ClassLinkerErrorHandler *errorHandler)
{
    Method *method;
    auto id = methodDataAccessor.GetMethodId();
    const auto &pf = methodDataAccessor.GetPandaFile();

    bool isStatic = methodDataAccessor.IsStatic();
    if (!methodDataAccessor.IsExternal() && klass->GetPandaFile() == &pf) {
        if (klass->IsInterface()) {
            method = isStatic ? klass->GetStaticInterfaceMethod(id) : klass->GetVirtualInterfaceMethod(id);
        } else {
            method = isStatic ? klass->GetStaticClassMethod(id) : klass->GetVirtualClassMethod(id);
        }

        if (method == nullptr) {
            Method::Proto proto(pf, methodDataAccessor.GetProtoId());
            PandaStringStream ss;
            ss << "Cannot find method '" << methodDataAccessor.GetName().data << " " << proto.GetSignature(true)
               << "' in class '" << klass->GetName() << "'";
            OnError(errorHandler, Error::METHOD_NOT_FOUND, ss.str());
            return nullptr;
        }

        return method;
    }

    auto name = methodDataAccessor.GetName();
    Method::Proto proto(pf, methodDataAccessor.GetProtoId());
    if (klass->IsInterface()) {
        method = isStatic ? klass->GetStaticInterfaceMethodByName(name, proto)
                          : klass->GetVirtualInterfaceMethodByName(name, proto);
    } else {
        method =
            isStatic ? klass->GetStaticClassMethodByName(name, proto) : klass->GetVirtualClassMethodByName(name, proto);
        if (method == nullptr && klass->IsAbstract()) {
            method = klass->GetInterfaceMethod(name, proto);
        }
    }

    if (method == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find method '" << methodDataAccessor.GetName().data << " " << proto.GetSignature(true)
           << "' in class '" << klass->GetName() << "'";
        OnError(errorHandler, Error::METHOD_NOT_FOUND, ss.str());
        return nullptr;
    }

    LOG_IF(method->IsStatic() != methodDataAccessor.IsStatic(), FATAL, CLASS_LINKER)
        << "Expected ACC_STATIC for method " << name.data << " in class " << klass->GetName()
        << " does not match loaded value";

    return method;
}

Field *ClassLinker::GetFieldById(Class *klass, const panda_file::FieldDataAccessor &fieldDataAccessor,
                                 ClassLinkerErrorHandler *errorHandler, bool isStatic)
{
    auto &pf = fieldDataAccessor.GetPandaFile();
    auto id = fieldDataAccessor.GetFieldId();

    Field *field = isStatic ? klass->FindStaticFieldById(id) : klass->FindInstanceFieldById(id);

    if (field == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find field '" << pf.GetStringData(fieldDataAccessor.GetNameId()).data << "' in class '"
           << klass->GetName() << "'";
        OnError(errorHandler, Error::FIELD_NOT_FOUND, ss.str());
        return nullptr;
    }

    pf.GetPandaCache()->SetFieldCache(id, field);
    return field;
}

Field *ClassLinker::GetFieldBySignature(Class *klass, const panda_file::FieldDataAccessor &fieldDataAccessor,
                                        ClassLinkerErrorHandler *errorHandler, bool isStatic)
{
    auto &pf = fieldDataAccessor.GetPandaFile();
    auto id = fieldDataAccessor.GetFieldId();
    auto fieldName = pf.GetStringData(fieldDataAccessor.GetNameId());
    auto fieldType = panda_file::Type::GetTypeFromFieldEncoding(fieldDataAccessor.GetType());
    auto filter = [&fieldDataAccessor, &fieldType, &fieldName, &id, &pf](const Field &fld) {
        if (fieldType == fld.GetType() && fieldName == fld.GetName()) {
            if (!fieldType.IsReference()) {
                return true;
            }

            // compare field class type
            if (&pf == fld.GetPandaFile() && id == fld.GetFileId()) {
                return true;
            }
            auto typeId = panda_file::FieldDataAccessor::GetTypeId(*fld.GetPandaFile(), fld.GetFileId());
            if (pf.GetStringData(panda_file::File::EntityId(fieldDataAccessor.GetType())) ==
                fld.GetPandaFile()->GetStringData(typeId)) {
                return true;
            }
        }
        return false;
    };
    Field *field = isStatic ? klass->FindStaticField(filter) : klass->FindInstanceField(filter);

    if (field == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find field '" << fieldName.data << "' in class '" << klass->GetName() << "'";
        OnError(errorHandler, Error::FIELD_NOT_FOUND, ss.str());
        return nullptr;
    }

    pf.GetPandaCache()->SetFieldCache(id, field);
    return field;
}

Field *ClassLinker::GetField(const panda_file::File &pf, panda_file::File::EntityId id, bool isStatic,
                             ClassLinkerContext *context /* = nullptr */,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Field *field = pf.GetPandaCache()->GetFieldFromCache(id);
    if (field != nullptr) {
        return field;
    }
    panda_file::FieldDataAccessor fieldDataAccessor(pf, id);

    Class *klass = GetClass(pf, fieldDataAccessor.GetClassId(), context, errorHandler);

    if (klass == nullptr) {
        auto className = pf.GetStringData(fieldDataAccessor.GetClassId()).data;
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }

    if (!fieldDataAccessor.IsExternal() && klass->GetPandaFile() == &pf) {
        field = GetFieldById(klass, fieldDataAccessor, errorHandler, isStatic);
    } else {
        field = GetFieldBySignature(klass, fieldDataAccessor, errorHandler, isStatic);
    }
    return field;
}

bool ClassLinker::InitializeClass(ManagedThread *thread, Class *klass)
{
    ASSERT(klass != nullptr);
    if (klass->IsInitialized()) {
        return true;
    }

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    return ctx.InitializeClass(this, thread, klass);
}

size_t ClassLinker::NumLoadedClasses()
{
    size_t sum = 0;

    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        sum += ext->NumLoadedClasses();
    }

    return sum;
}

void ClassLinker::VisitLoadedClasses(size_t flag)
{
    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }
        ext->VisitLoadedClasses(flag);
    }
}

Field *ClassLinker::GetField(const Method &caller, panda_file::File::EntityId id, bool isStatic,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Field *field = caller.GetPandaFile()->GetPandaCache()->GetFieldFromCache(id);
    if (field != nullptr) {
        return field;
    }
    auto *ext = GetExtension(caller.GetClass()->GetSourceLang());
    field = GetField(*caller.GetPandaFile(), id, isStatic, caller.GetClass()->GetLoadContext(),
                     (errorHandler == nullptr) ? ext->GetErrorHandler() : errorHandler);
    if (LIKELY(field != nullptr)) {
        caller.GetPandaFile()->GetPandaCache()->SetFieldCache(id, field);
    }
    return field;
}

void ClassLinker::RemoveCreatedClassInExtension(Class *klass)
{
    if (klass == nullptr) {
        return;
    }
    auto ext = GetExtension(klass->GetSourceLang());
    if (ext != nullptr) {
        ext->OnClassPrepared(klass);
    }
}

}  // namespace ark
