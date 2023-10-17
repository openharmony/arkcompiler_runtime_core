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

#include "runtime/handle_scope-inl.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/monitor_object_lock.h"
#include "runtime/monitor.h"
#include "libpandafile/panda_cache.h"

#include "runtime/hotreload/hotreload.h"

namespace panda::hotreload {

static Error GetHotreloadErrorByFlag(uint32_t flag)
{
    if (flag == ChangesFlags::F_NO_STRUCT_CHANGES) {
        return Error::NONE;
    }
    if ((flag & ChangesFlags::F_ACCESS_FLAGS) != 0U) {
        return Error::CLASS_MODIFIERS;
    }
    if ((flag & ChangesFlags::F_FIELDS_TYPE) != 0U || (flag & ChangesFlags::F_FIELDS_AMOUNT) != 0U) {
        return Error::FIELD_CHANGED;
    }
    if ((flag & ChangesFlags::F_INHERITANCE) != 0U || (flag & ChangesFlags::F_INTERFACES) != 0U) {
        return Error::HIERARCHY_CHANGED;
    }
    if ((flag & ChangesFlags::F_METHOD_SIGN) != 0U) {
        return Error::METHOD_SIGN;
    }
    if ((flag & ChangesFlags::F_METHOD_ADDED) != 0U) {
        return Error::METHOD_ADDED;
    }
    if ((flag & ChangesFlags::F_METHOD_DELETED) != 0U) {
        return Error::METHOD_DELETED;
    }

    return Error::UNSUPPORTED_CHANGES;
}

// ---------------------------------------------------------
// ------------------ Hotreload Class API ------------------
// ---------------------------------------------------------

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromFile(const char *location)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFile(location);
    const panda_file::File *ptr = pf.get();
    panda_files_.push_back(std::move(pf));
    return ptr;
}

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromMemory(const void *buffer, size_t buff_size)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFileFromMemory(buffer, buff_size);
    auto ptr = pf.get();
    panda_files_.push_back(std::move(pf));
    return ptr;
}

Error ArkHotreloadBase::ProcessHotreload()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    Error err = ValidateClassesHotreloadPossibility();
    if (err != Error::NONE) {
        return err;
    }

    for (auto &h_cls : classes_) {
        auto changes_type = RecognizeHotreloadType(&h_cls);
        if (changes_type == Type::STRUCTURAL) {
            LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_
                                  << " has structural changes. Structural changes is unsafe for hotreload.";
            return GetHotreloadErrorByFlag(h_cls.f_changes);
        }
    }

    /*
     * On this point all classes were verified and hotreload recognized like possible
     * From now all classes should be reloaded anyway and hotreload cannot be reversed
     */
    {
        ScopedSuspendAllThreadsRunning ssat(PandaVM::GetCurrent()->GetRendezvous());
        ASSERT_MANAGED_CODE();
        if (Runtime::GetCurrent()->IsJitEnabled()) {
            /*
             * In case JIT is enabled while hotreloading we must suspend it and clear compiled methods
             * Currently JIT is not running with hotreload
             */
            UNREACHABLE();
        }
        ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
        PandaVM::GetCurrent()->GetThreadManager()->EnumerateThreads([this](ManagedThread *thread) {
            (void)this;  // [[maybe_unused]] in lambda capture list is not possible
            ASSERT(thread->GetThreadLang() == lang_);
            thread->GetInterpreterCache()->Clear();
            return true;
        });
        for (auto &h_cls : classes_) {
            ReloadClassNormal(&h_cls);
        }

        auto class_linker = Runtime::GetCurrent()->GetClassLinker();
        // Updating VTable should be before adding obsolete classes for performance reason
        UpdateVtablesInRuntimeClasses(class_linker);
        AddLoadedPandaFilesToRuntime(class_linker);
        AddObsoleteClassesToRuntime(class_linker);

        LangSpecificHotreloadPart();
    }
    return Error::NONE;
}

ArkHotreloadBase::ArkHotreloadBase(ManagedThread *mthread, panda_file::SourceLang lang) : lang_(lang), thread_(mthread)
{
    /*
     * Scoped object that switch to managed code is language dependent
     * So is should be constructed in superclass
     */
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ != nullptr);
    ASSERT(thread_ == ManagedThread::GetCurrent());
    ASSERT(lang_ == thread_->GetThreadLang());
}

/* virtual */
ArkHotreloadBase::~ArkHotreloadBase()
{
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    for (auto &h_cls : classes_) {
        if (h_cls.tmp_class != nullptr) {
            class_linker->FreeClass(h_cls.tmp_class);
        }
    }
}

// ---------------------------------------------------------
// ----------------------- Validators ----------------------
// ---------------------------------------------------------

Error ArkHotreloadBase::ValidateClassesHotreloadPossibility()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto err = this->LangSpecificValidateClasses();
    if (err != Error::NONE) {
        return err;
    }

    for (const auto &h_cls : classes_) {
        Error return_err = ValidateClassForHotreload(h_cls);
        if (return_err != Error::NONE) {
            return return_err;
        }
    }

    return Error::NONE;
}

std::optional<uint32_t> GetLockOwnerThreadId(Class *cls)
{
    if (Monitor::HoldsLock(cls->GetManagedObject()) != 0U) {
        return Monitor::GetLockOwnerOsThreadID(cls->GetManagedObject());
    }
    return {};
}

Error ArkHotreloadBase::ValidateClassForHotreload(const ClassContainment &h_cls)
{
    Class *clazz = h_cls.tmp_class;
    Class *runtime_class = h_cls.loaded_class;
    if (clazz == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " are failed to be initialized";
        return Error::INTERNAL;
    }

    /*
     * Check if class is not resolved
     * In case class have a lock holded by the same thread it will lead to deadlock
     * In case class is not initialized we should initialize it before hotreloading
     */
    if (!runtime_class->IsInitialized()) {
        if (GetLockOwnerThreadId(runtime_class) == thread_->GetId()) {
            LOG(ERROR, HOTRELOAD) << "Class lock " << h_cls.class_name_ << " are already owned by this thread.";
            return Error::INTERNAL;
        }
        auto class_linker = Runtime::GetCurrent()->GetClassLinker();
        if (!class_linker->InitializeClass(thread_, runtime_class)) {
            LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " cannot be initialized in runtime.";
            return Error::INTERNAL;
        }
    }

    if (clazz->IsInterface()) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " is an interface class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsProxy()) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " is a proxy class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsArrayClass()) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " is an array class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsStringClass()) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " is a string class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsPrimitive()) {
        LOG(ERROR, HOTRELOAD) << "Class " << h_cls.class_name_ << " is a primitive class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }

    return Error::NONE;
}

Type ArkHotreloadBase::RecognizeHotreloadType(ClassContainment *h_cls)
{
    /*
     * Checking for the changes:
     *  - Inheritance changed
     *  - Access flags changed
     *  - Field added/deleted
     *  - Field type changed
     *  - Method added/deleted
     *  - Method signature changed
     *
     * In case there are any of changes above Type is Structural
     * Otherwise it's normal changes
     */
    if (InheritanceChangesCheck(h_cls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FlagsChangesCheck(h_cls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FieldChangesCheck(h_cls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (MethodChangesCheck(h_cls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::InheritanceChangesCheck(ClassContainment *h_cls)
{
    Class *tmp_class = h_cls->tmp_class;
    Class *runtime_class = h_cls->loaded_class;
    if (tmp_class->GetBase() != runtime_class->GetBase()) {
        h_cls->f_changes |= ChangesFlags::F_INHERITANCE;
        return Type::STRUCTURAL;
    }

    auto new_ifaces = tmp_class->GetInterfaces();
    auto old_ifaces = runtime_class->GetInterfaces();
    if (new_ifaces.size() != old_ifaces.size()) {
        h_cls->f_changes |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    PandaUnorderedSet<Class *> ifaces;
    for (auto iface : old_ifaces) {
        ifaces.insert(iface);
    }
    for (auto iface : new_ifaces) {
        if (ifaces.find(iface) == ifaces.end()) {
            h_cls->f_changes |= ChangesFlags::F_INTERFACES;
            return Type::STRUCTURAL;
        }
        ifaces.erase(iface);
    }
    if (!ifaces.empty()) {
        h_cls->f_changes |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::FlagsChangesCheck(ClassContainment *h_cls)
{
    Class *tmp_class = h_cls->tmp_class;
    Class *runtime_class = h_cls->loaded_class;

    // TODO(m.strizhak) research that maybe there are flags that can be changed keeping normal type
    if (tmp_class->GetFlags() != runtime_class->GetFlags()) {
        h_cls->f_changes |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    if (tmp_class->GetAccessFlags() != runtime_class->GetAccessFlags()) {
        h_cls->f_changes |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::FieldChangesCheck(ClassContainment *h_cls)
{
    Class *tmp_class = h_cls->tmp_class;
    Class *runtime_class = h_cls->loaded_class;

    auto old_fields = runtime_class->GetFields();
    auto new_fields = tmp_class->GetFields();
    if (new_fields.size() != old_fields.size()) {
        h_cls->f_changes |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    FieldIdTable field_id_table;
    PandaUnorderedMap<PandaString, Field *> fields_table;
    for (auto &old_field : old_fields) {
        PandaString field_name(utf::Mutf8AsCString(old_field.GetName().data));
        fields_table.insert({field_name, &old_field});
    }

    for (auto &new_field : new_fields) {
        PandaString field_name(utf::Mutf8AsCString(new_field.GetName().data));
        auto old_it = fields_table.find(field_name);
        if (old_it == fields_table.end()) {
            h_cls->f_changes |= ChangesFlags::F_FIELDS_AMOUNT;
            return Type::STRUCTURAL;
        }

        if (old_it->second->GetAccessFlags() != new_field.GetAccessFlags() ||
            old_it->second->GetTypeId() != new_field.GetTypeId()) {
            h_cls->f_changes |= ChangesFlags::F_FIELDS_TYPE;
            return Type::STRUCTURAL;
        }

        field_id_table[fields_table[field_name]->GetFileId()] = new_field.GetFileId();
        fields_table.erase(field_name);
    }

    fields_tables_[runtime_class] = std::move(field_id_table);

    if (!fields_table.empty()) {
        h_cls->f_changes |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

static inline uint32_t GetFileAccessFlags(const Method &method)
{
    return method.GetAccessFlags() & ACC_FILE_MASK;
}

Type ArkHotreloadBase::MethodChangesCheck(ClassContainment *h_cls)
{
    Class *tmp_class = h_cls->tmp_class;
    Class *runtime_class = h_cls->loaded_class;

    auto old_methods = runtime_class->GetMethods();
    auto new_methods = tmp_class->GetMethods();

    if (new_methods.size() > old_methods.size() ||
        tmp_class->GetNumVirtualMethods() > runtime_class->GetNumVirtualMethods()) {
        h_cls->f_changes |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }

    if (new_methods.size() < old_methods.size() ||
        tmp_class->GetNumVirtualMethods() < runtime_class->GetNumVirtualMethods()) {
        h_cls->f_changes |= ChangesFlags::F_METHOD_DELETED;
        return Type::STRUCTURAL;
    }

    for (auto &new_method : new_methods) {
        bool is_name_found = false;
        bool is_exact_found = false;
        for (auto &old_method : old_methods) {
            PandaString old_name = utf::Mutf8AsCString(old_method.GetName().data);
            PandaString new_name = utf::Mutf8AsCString(new_method.GetName().data);
            if (old_name == new_name) {
                is_name_found = true;
                if (old_method.GetProto() == new_method.GetProto() &&
                    GetFileAccessFlags(old_method) == GetFileAccessFlags(new_method)) {
                    methods_table_[&old_method] = &new_method;
                    is_exact_found = true;
                    break;
                }
            }
        }

        if (is_name_found) {
            if (is_exact_found) {
                continue;
            }
            h_cls->f_changes |= ChangesFlags::F_METHOD_SIGN;
            return Type::STRUCTURAL;
        }
        h_cls->f_changes |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }
    return Type::NORMAL;
}

// ---------------------------------------------------------
// ----------------------- Reloaders -----------------------
// ---------------------------------------------------------

// This method is used under assert. So in release build it's unused
[[maybe_unused]] static bool VerifyClassConsistency(const Class *cls)
{
    ASSERT(cls);

    const auto pf = cls->GetPandaFile();
    auto methods = cls->GetMethods();
    auto fields = cls->GetFields();
    const uint8_t *descriptor = cls->GetDescriptor();

    if (cls->GetFileId().GetOffset() != pf->GetClassId(descriptor).GetOffset()) {
        return false;
    }

    for (const auto &method : methods) {
        panda_file::MethodDataAccessor mda(*pf, method.GetFileId());
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        if (mda.GetName() != method.GetName()) {
            return false;
        }
        if (method.GetClass() != cls) {
            return false;
        }
    }

    for (const auto &field : fields) {
        panda_file::FieldDataAccessor fda(*pf, field.GetFileId());
        if (field.GetClass() != cls) {
            return false;
        }
    }

    return true;
}

static void UpdateClassPtrInMethods(Span<Method> methods, Class *cls)
{
    for (auto &method : methods) {
        method.SetClass(cls);
    }
}

static void UpdateClassPtrInFields(Span<Field> fields, Class *cls)
{
    for (auto &field : fields) {
        field.SetClass(cls);
    }
}

static void UpdatePandaFileInClass(Class *runtime_class, const panda_file::File *pf)
{
    const uint8_t *descriptor = runtime_class->GetDescriptor();
    panda_file::File::EntityId class_id = pf->GetClassId(descriptor);
    runtime_class->SetPandaFile(pf);
    runtime_class->SetFileId(class_id);
    runtime_class->SetClassIndex(pf->GetClassIndex(class_id));
    runtime_class->SetMethodIndex(pf->GetMethodIndex(class_id));
    runtime_class->SetFieldIndex(pf->GetFieldIndex(class_id));
}

/*
 * Obsolete methods should be saved by temporary class 'cause it might be continue executing
 * Updating class pointers in methods to keep it consistent with class and panda file
 */
static void UpdateMethods(Class *runtime_class, Class *tmp_class)
{
    auto new_methods = tmp_class->GetMethodsWithCopied();
    auto old_methods = runtime_class->GetMethodsWithCopied();
    uint32_t num_vmethods = tmp_class->GetNumVirtualMethods();
    uint32_t num_cmethods = tmp_class->GetNumCopiedMethods();
    uint32_t num_smethods = new_methods.size() - num_vmethods - num_cmethods;
    UpdateClassPtrInMethods(new_methods, runtime_class);
    UpdateClassPtrInMethods(old_methods, tmp_class);
    runtime_class->SetMethods(new_methods, num_vmethods, num_smethods);
    tmp_class->SetMethods(old_methods, num_vmethods, num_smethods);
}

/*
 * Obsolete fields should be saved by temporary class 'cause it might be used by obselete methods
 * Updating class pointers in fields to keep it consistent with class and panda file
 */
static void UpdateFields(Class *runtime_class, Class *tmp_class)
{
    auto new_fields = tmp_class->GetFields();
    auto old_fields = runtime_class->GetFields();
    uint32_t num_sfields = tmp_class->GetNumStaticFields();
    UpdateClassPtrInFields(new_fields, runtime_class);
    UpdateClassPtrInFields(old_fields, tmp_class);
    runtime_class->SetFields(new_fields, num_sfields);
    tmp_class->SetFields(old_fields, num_sfields);
}

static void UpdateIfaces(Class *runtime_class, Class *tmp_class)
{
    auto new_ifaces = tmp_class->GetInterfaces();
    auto old_ifaces = runtime_class->GetInterfaces();
    runtime_class->SetInterfaces(new_ifaces);
    tmp_class->SetInterfaces(old_ifaces);
}

/*
 * Tables is being copied to runtime class to be possible to resolve virtual calls
 * No need to copy tables to temporary class 'cause new virtual calls should be resolved from runtime class
 */
static void UpdateTables(Class *runtime_class, Class *tmp_class)
{
    ASSERT(tmp_class->GetIMTSize() == runtime_class->GetIMTSize());
    ASSERT(tmp_class->GetVTableSize() == runtime_class->GetVTableSize());
    ITable old_itable = runtime_class->GetITable();
    Span<Method *> old_vtable = runtime_class->GetVTable();
    Span<Method *> new_vtable = tmp_class->GetVTable();
    Span<Method *> old_imt = runtime_class->GetIMT();
    Span<Method *> new_imt = tmp_class->GetIMT();
    runtime_class->SetITable(tmp_class->GetITable());
    tmp_class->SetITable(old_itable);
    if (!old_vtable.empty() && memcpy_s(old_vtable.begin(), old_vtable.size() * sizeof(void *), new_vtable.begin(),
                                        old_vtable.size() * sizeof(void *)) != EOK) {
        LOG(FATAL, RUNTIME) << __func__ << " memcpy_s failed";
    }
    if (!old_imt.empty() && memcpy_s(old_imt.begin(), old_imt.size() * sizeof(void *), new_imt.begin(),
                                     old_imt.size() * sizeof(void *)) != EOK) {
        LOG(FATAL, RUNTIME) << __func__ << " memcpy_s failed";
    }
}

void ArkHotreloadBase::ReloadClassNormal(const ClassContainment *h_cls)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    /*
     * Take class loading lock
     * Clear interpreter cache
     * Update runtime class header:
     *   - Panda file and its id
     *   - methods
     *   - fields
     *   - ifaces
     *   - tables
     *
     * Then adding obsolete classes to special area in class linker
     * to be able to continue executing obsolete methods after hotreloading
     */
    Class *tmp_class = h_cls->tmp_class;
    Class *runtime_class = h_cls->loaded_class;

    // Locking class
    HandleScope<ObjectHeader *> scope(thread_);
    VMHandle<ObjectHeader> managed_class_obj_handle(thread_, runtime_class->GetManagedObject());
    ::panda::ObjectLock lock(managed_class_obj_handle.GetPtr());
    const panda_file::File *new_pf = h_cls->pf;
    const panda_file::File *old_pf = runtime_class->GetPandaFile();

    old_pf->GetPandaCache()->Clear();

    reloaded_classes_.insert(runtime_class);
    UpdatePandaFileInClass(tmp_class, old_pf);
    UpdatePandaFileInClass(runtime_class, new_pf);
    UpdateMethods(runtime_class, tmp_class);
    UpdateFields(runtime_class, tmp_class);
    UpdateIfaces(runtime_class, tmp_class);
    UpdateTables(runtime_class, tmp_class);

    ASSERT(VerifyClassConsistency(runtime_class));
    ASSERT(VerifyClassConsistency(tmp_class));
}

void ArkHotreloadBase::UpdateVtablesInRuntimeClasses(ClassLinker *class_linker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    auto update_vtable = [this](Class *cls) {
        auto vtable = cls->GetVTable();

        if (reloaded_classes_.find(cls) != reloaded_classes_.end()) {
            // This table is already actual
            return true;
        }

        for (auto &method_ptr : vtable) {
            if (methods_table_.find(method_ptr) != methods_table_.end()) {
                method_ptr = methods_table_[method_ptr];
            }
        }
        return true;
    };
    class_linker->GetExtension(lang_)->EnumerateClasses(update_vtable);
}

void ArkHotreloadBase::AddLoadedPandaFilesToRuntime(ClassLinker *class_linker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    for (auto &ptr_pf : panda_files_) {
        class_linker->AddPandaFile(std::move(ptr_pf));
    }
    panda_files_.clear();
}

void ArkHotreloadBase::AddObsoleteClassesToRuntime(ClassLinker *class_linker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    /*
     * Sending all classes in one vector to avoid holding lock for every single class
     * It should be faster because all threads are still stopped
     */
    PandaVector<Class *> obsolete_classes;
    for (const auto &h_cls : classes_) {
        if (h_cls.tmp_class != nullptr) {
            ASSERT(h_cls.tmp_class->GetSourceLang() == lang_);
            obsolete_classes.push_back(h_cls.tmp_class);
        }
    }
    class_linker->GetExtension(lang_)->AddObsoleteClass(obsolete_classes);
    classes_.clear();
}

}  // namespace panda::hotreload