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

#include "verification/config/config_load.h"
#include "verification/config/context/context.h"
#include "verification/config/debug_breakpoint/breakpoint_private.h"
#include "verification/config/whitelist/whitelist_private.h"
#include "verification/jobs/thread_pool.h"
#include "verification/cache/results_cache.h"
#include "verification/util/is_system.h"

#include "events/events.h"
#include "runtime/bridge/bridge.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/value-inl.h"
#include "runtime/interpreter/frame.h"
#include "runtime/interpreter/interpreter.h"
#include "libpandabase/utils/hash.h"
#include "libpandabase/utils/span.h"
#include "libpandabase/utils/utf.h"
#include "libpandabase/os/mutex.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/debug_data_accessor-inl.h"
#include "libpandafile/debug_helpers.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/line_number_program.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/shorty_iterator.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "libpandafile/type_helper.h"

namespace panda {

Method::Proto::Proto(const panda_file::File &pf, panda_file::File::EntityId proto_id)
{
    panda_file::ProtoDataAccessor pda(pf, proto_id);

    pda.EnumerateTypes([this](panda_file::Type type) { shorty_.push_back(type); });

    auto ref_num = pda.GetRefNum();
    for (size_t ref_idx = 0; ref_idx < ref_num; ++ref_idx) {
        auto id = pda.GetReferenceType(ref_idx);
        ref_types_.emplace_back(utf::Mutf8AsCString(pf.GetStringData(id).data));
    }
}

bool Method::ProtoId::operator==(const Method::ProtoId &other) const
{
    if (&pf_ == &other.pf_) {
        return proto_id_ == other.proto_id_;
    }

    panda_file::ProtoDataAccessor pda1(pf_, proto_id_);
    panda_file::ProtoDataAccessor pda2(other.pf_, other.proto_id_);
    return pda1.IsEqual(&pda2);
}

bool Method::ProtoId::operator==(const Proto &other) const
{
    const auto &shorties = other.GetShorty();
    const auto &ref_types = other.GetRefTypes();

    bool equal = true;
    size_t shorty_idx = 0;
    panda_file::ProtoDataAccessor pda(pf_, proto_id_);
    pda.EnumerateTypes([&equal, &shorties, &shorty_idx](panda_file::Type type) {
        equal = equal && shorty_idx < shorties.size() && type == shorties[shorty_idx];
        ++shorty_idx;
    });
    if (!equal || shorty_idx != shorties.size() || pda.GetRefNum() != ref_types.size()) {
        return false;
    }

    // check ref types
    for (size_t ref_idx = 0; ref_idx < ref_types.size(); ++ref_idx) {
        auto id = pda.GetReferenceType(ref_idx);
        if (ref_types[ref_idx] != utf::Mutf8AsCString(pf_.GetStringData(id).data)) {
            return false;
        }
    }

    return true;
}

std::string_view Method::Proto::GetReturnTypeDescriptor() const
{
    auto ret_type = GetReturnType();
    if (!ret_type.IsPrimitive()) {
        return ref_types_[0];
    }

    switch (ret_type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return "V";
        case panda_file::Type::TypeId::U1:
            return "Z";
        case panda_file::Type::TypeId::I8:
            return "B";
        case panda_file::Type::TypeId::U8:
            return "H";
        case panda_file::Type::TypeId::I16:
            return "S";
        case panda_file::Type::TypeId::U16:
            return "C";
        case panda_file::Type::TypeId::I32:
            return "I";
        case panda_file::Type::TypeId::U32:
            return "U";
        case panda_file::Type::TypeId::F32:
            return "F";
        case panda_file::Type::TypeId::I64:
            return "J";
        case panda_file::Type::TypeId::U64:
            return "Q";
        case panda_file::Type::TypeId::F64:
            return "D";
        case panda_file::Type::TypeId::TAGGED:
            return "A";
        default:
            UNREACHABLE();
    }
}

uint32_t Method::GetFullNameHashFromString(const PandaString &str)
{
    return GetHash32String(reinterpret_cast<const uint8_t *>(str.c_str()));
}

uint32_t Method::GetClassNameHashFromString(const PandaString &str)
{
    return GetHash32String(reinterpret_cast<const uint8_t *>(str.c_str()));
}

Method::UniqId Method::CalcUniqId(const uint8_t *class_descr, const uint8_t *name)
{
    auto constexpr HALF = 32ULL;
    constexpr uint64_t NO_FILE = 0xFFFFFFFFULL << HALF;
    uint64_t hash = PseudoFnvHashString(class_descr);
    hash = PseudoFnvHashString(name, hash);
    return NO_FILE | hash;
}

Method::Method(Class *klass, const panda_file::File *pf, panda_file::File::EntityId file_id,
               panda_file::File::EntityId code_id, uint32_t access_flags, uint32_t num_args, const uint16_t *shorty)
    : access_flags_(access_flags),
      num_args_(num_args),
      stor_16_pair_({0, 0}),
      class_word_(static_cast<ClassHelper::classWordSize>(ToObjPtrType(klass))),
      compiled_entry_point_(nullptr),
      panda_file_(pf),
      pointer_(),

      file_id_(file_id),
      code_id_(code_id),
      shorty_(shorty)
{
    // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering constraints
    // imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    pointer_.native_pointer_.store(nullptr, std::memory_order_relaxed);
    SetCompilationStatus(CompilationStage::NOT_COMPILED);
}

Value Method::Invoke(ManagedThread *thread, Value *args, bool proxy_call)
{
    ASSERT(!thread->HasPendingException());
    return InvokeImpl<InvokeHelperStatic>(thread, GetNumArgs(), args, proxy_call);
}

panda_file::Type Method::GetReturnType() const
{
    panda_file::ShortyIterator it(shorty_);
    return *it;
}

panda_file::Type Method::GetArgType(size_t idx) const
{
    if (!IsStatic()) {
        if (idx == 0) {
            return panda_file::Type(panda_file::Type::TypeId::REFERENCE);
        }

        --idx;
    }

    panda_file::ProtoDataAccessor pda(*(panda_file_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
    return pda.GetArgType(idx);
}

panda_file::File::StringData Method::GetRefReturnType() const
{
    ASSERT(GetReturnType().IsReference());
    panda_file::ProtoDataAccessor pda(*(panda_file_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
    panda_file::File::EntityId class_id = pda.GetReferenceType(0);
    return panda_file_->GetStringData(class_id);
}

panda_file::File::StringData Method::GetRefArgType(size_t idx) const
{
    if (!IsStatic()) {
        if (idx == 0) {
            return panda_file_->GetStringData(panda_file::MethodDataAccessor::GetClassId(*(panda_file_), file_id_));
        }

        --idx;
    }

    // in case of reference return type first idx corresponds to it
    if (GetReturnType().IsReference()) {
        ++idx;
    }
    panda_file::ProtoDataAccessor pda(*(panda_file_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
    panda_file::File::EntityId class_id = pda.GetReferenceType(idx);
    return panda_file_->GetStringData(class_id);
}

panda_file::File::StringData Method::GetName() const
{
    return panda_file::MethodDataAccessor::GetName(*(panda_file_), file_id_);
}

PandaString Method::GetFullName(bool with_signature) const
{
    PandaOStringStream ss;
    int ref_idx = 0;
    if (with_signature) {
        auto return_type = GetReturnType();
        if (return_type.IsReference()) {
            ss << ClassHelper::GetName(GetRefReturnType().data) << ' ';
        } else {
            ss << return_type << ' ';
        }
    }
    if (GetClass() != nullptr) {
        ss << PandaString(GetClass()->GetName());
    }
    ss << "::" << utf::Mutf8AsCString(Method::GetName().data);
    if (!with_signature) {
        return ss.str();
    }
    const char *sep = "";
    ss << '(';
    panda_file::ProtoDataAccessor pda(*(panda_file_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
    for (size_t i = 0; i < GetNumArgs(); i++) {
        auto type = GetEffectiveArgType(i);
        if (type.IsReference()) {
            ss << sep << ClassHelper::GetName(GetRefArgType(ref_idx++).data);
        } else {
            ss << sep << type;
        }
        sep = ", ";
    }
    ss << ')';
    return ss.str();
}

panda_file::File::StringData Method::GetClassName() const
{
    return panda_file_->GetStringData(panda_file::MethodDataAccessor::GetClassId(*(panda_file_), file_id_));
}

Method::Proto Method::GetProto() const
{
    return Proto(*(panda_file_), panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
}

Method::ProtoId Method::GetProtoId() const
{
    return ProtoId(*(panda_file_), panda_file::MethodDataAccessor::GetProtoId(*(panda_file_), file_id_));
}

uint32_t Method::GetNumericalAnnotation(AnnotationField field_id) const
{
    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    return mda.GetNumericalAnnotation(field_id);
}

panda_file::File::StringData Method::GetStringDataAnnotation(AnnotationField field_id) const
{
    ASSERT(field_id >= AnnotationField::STRING_DATA_BEGIN && field_id <= AnnotationField::STRING_DATA_END);
    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    uint32_t str_offset = mda.GetNumericalAnnotation(field_id);
    if (str_offset == 0) {
        return {0, nullptr};
    }
    return panda_file_->GetStringData(panda_file::File::EntityId(str_offset));
}

int64_t Method::GetBranchTakenCounter(uint32_t pc)
{
    auto profiling_data = GetProfilingData();
    if (profiling_data == nullptr) {
        return 0;
    }
    return profiling_data->GetBranchTakenCounter(pc);
}

int64_t Method::GetBranchNotTakenCounter(uint32_t pc)
{
    auto profiling_data = GetProfilingData();
    if (profiling_data == nullptr) {
        return 0;
    }
    return profiling_data->GetBranchNotTakenCounter(pc);
}

uint32_t Method::FindCatchBlock(const Class *cls, uint32_t pc) const
{
    ASSERT(!IsAbstract());

    auto *thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> exception(thread, thread->GetException());
    thread->ClearException();

    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    panda_file::CodeDataAccessor cda(*(panda_file_), mda.GetCodeId().value());

    uint32_t pc_offset = panda_file::INVALID_OFFSET;

    cda.EnumerateTryBlocks([&pc_offset, cls, pc, this](panda_file::CodeDataAccessor::TryBlock &try_block) {
        if ((try_block.GetStartPc() <= pc) && ((try_block.GetStartPc() + try_block.GetLength()) > pc)) {
            try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
                auto type_idx = catch_block.GetTypeIdx();
                if (type_idx == panda_file::INVALID_INDEX) {
                    pc_offset = catch_block.GetHandlerPc();
                    return false;
                }

                auto type_id = GetClass()->ResolveClassIndex(type_idx);
                auto *handler_class = Runtime::GetCurrent()->GetClassLinker()->GetClass(*this, type_id);
                if (cls->IsSubClassOf(handler_class)) {
                    pc_offset = catch_block.GetHandlerPc();
                    return false;
                }
                return true;
            });
        }
        return pc_offset == panda_file::INVALID_OFFSET;
    });

    thread->SetException(exception.GetPtr());

    return pc_offset;
}

panda_file::Type Method::GetEffectiveArgType(size_t idx) const
{
    return panda_file::GetEffectiveType(GetArgType(idx));
}

panda_file::Type Method::GetEffectiveReturnType() const
{
    return panda_file::GetEffectiveType(GetReturnType());
}

int32_t Method::GetLineNumFromBytecodeOffset(uint32_t bc_offset) const
{
    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    return panda_file::debug_helpers::GetLineNumber(mda, bc_offset, panda_file_);
}

panda_file::File::StringData Method::GetClassSourceFile() const
{
    panda_file::ClassDataAccessor cda(*(panda_file_), GetClass()->GetFileId());
    auto source_file_id = cda.GetSourceFileId();
    if (!source_file_id) {
        return {0, nullptr};
    }

    return panda_file_->GetStringData(source_file_id.value());
}

bool Method::IsVerified() const
{
    if (IsIntrinsic()) {
        return true;
    }
    auto stage = GetVerificationStage();
    return stage == VerificationStage::VERIFIED_OK || stage == VerificationStage::VERIFIED_FAIL;
}

void Method::WaitForVerification()
{
    if (GetVerificationStage() == VerificationStage::WAITING) {
        LOG(DEBUG, VERIFIER) << "Method '" << GetFullName() << std::hex << "' is waiting to be verified";
        panda::verifier::ThreadPool::WaitForVerification(
            [this] { return GetVerificationStage() == VerificationStage::WAITING; },
            [this] { SetVerificationStage(VerificationStage::VERIFIED_FAIL); });
    }
}

void Method::SetVerified(bool result)
{
    if (IsIntrinsic()) {
        return;
    }
    verifier::VerificationResultCache::CacheResult(GetUniqId(), result);
    SetVerificationStage(result ? VerificationStage::VERIFIED_OK : VerificationStage::VERIFIED_FAIL);
    panda::verifier::ThreadPool::SignalMethodVerified();
}

bool Method::Verify()
{
    if (IsIntrinsic()) {
        return true;
    }
    auto stage = GetVerificationStage();
    if (stage == VerificationStage::VERIFIED_OK) {
        return true;
    }
    if (stage == VerificationStage::VERIFIED_FAIL) {
        return false;
    }

    EnqueueForVerification();
    WaitForVerification();

    return Verify();
}

Method::~Method()
{
    WaitForVerification();
}

bool Method::AddJobInQueue()
{
    if (code_id_.IsValid() && !verifier::debug::SkipVerification(GetUniqId())) {
        if (ExchangeVerificationStage(VerificationStage::WAITING) == VerificationStage::WAITING) {
            return true;
        }
        if (verifier::VerificationResultCache::Enabled()) {
            auto status = verifier::VerificationResultCache::Check(GetUniqId());
            switch (status) {
                case verifier::VerificationResultCache::Status::OK:
                    SetVerificationStage(VerificationStage::VERIFIED_OK);
                    LOG(DEBUG, VERIFIER) << "Verification result of method '" << GetFullName() << "' was cached: OK";
                    return true;
                case verifier::VerificationResultCache::Status::FAILED:
                    SetVerificationStage(VerificationStage::VERIFIED_FAIL);
                    LOG(DEBUG, VERIFIER) << "Verification result of method '" << GetFullName() << "' was cached: FAIL";
                    return true;
                default:
                    break;
            }
        }
        if (panda::verifier::ThreadPool::Enqueue(this)) {
            LOG(DEBUG, VERIFIER) << "Method '" << GetFullName() << "' enqueued for verification";
        } else {
            LOG(WARNING, VERIFIER) << "Method '" << GetFullName() << "' cannot be enqueued for verification";
            SetVerificationStage(VerificationStage::VERIFIED_FAIL);
        }
        return true;
    }

    return false;
}

void Method::EnqueueForVerification()
{
    if (IsIntrinsic()) {
        LOG(DEBUG, VERIFIER) << "Intrinsic method " << GetFullName(true) << " doesn't need to be verified";
        return;
    }
    if (GetVerificationStage() != VerificationStage::NOT_VERIFIED) {
        LOG(DEBUG, VERIFIER) << "Method " << GetFullName(true) << " is already verified";
        return;
    }
    auto &&verif_options = Runtime::GetCurrent()->GetVerificationOptions();
    if (verif_options.IsEnabled()) {
        verifier::debug::DebugContext::GetCurrent().AddMethod(*this, verif_options.Mode == VerificationMode::DEBUG);

        bool is_system = false;
        if (!verif_options.VerifyRuntimeLibraries) {
            auto *klass = GetClass();
            if (klass != nullptr) {
                is_system = verifier::IsSystemClass(*klass);
            }
        }
        if (is_system) {
            LOG(DEBUG, VERIFIER) << "Skipping verification of system method " << GetFullName(true);
        } else if (AddJobInQueue()) {
            return;
        }
    }
    if (verif_options.Show.Status) {
        LOG(DEBUG, VERIFIER) << "Verification result of method '" << GetFullName(true) << "': SKIP";
    }
    SetVerified(true);
}

Method::VerificationStage Method::GetVerificationStage() const
{
    // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
    // should become visible
    return BitsToVerificationStage(access_flags_.load(std::memory_order_acquire));
}

void Method::SetVerificationStage(VerificationStage stage)
{
    // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and on
    // writes before the store
    access_flags_.fetch_or((static_cast<uint32_t>(stage) << VERIFICATION_STATUS_SHIFT), std::memory_order_acq_rel);
}

Method::VerificationStage Method::ExchangeVerificationStage(VerificationStage stage)
{
    // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and on
    // writes before the store
    return BitsToVerificationStage(
        access_flags_.fetch_or((static_cast<uint32_t>(stage) << VERIFICATION_STATUS_SHIFT), std::memory_order_acq_rel));
}

Method::VerificationStage Method::BitsToVerificationStage(uint32_t bits)
{
    uint32_t val = (bits & VERIFICATION_STATUS_MASK) >> VERIFICATION_STATUS_SHIFT;
    // To avoid if - else for conversion set bit index to VerificationStage
    // y = 4x / 3 function for integers is used. It produces correct values for
    // all correct inputs:
    //                state  value __builtin_ffs 4x/3 VerificationStage
    //         not verified:  000        0        0     NOT_VERIFIED
    //              waiting:  100        3        3     WAITING
    // verification success:  110        2        2     VERIFIED_OK
    //  verification failed:  101        1        1     VERIFIED_FAIL
    return static_cast<VerificationStage>(4U * panda_bit_utils_ffs(val) / 3U);
}

void Method::StartProfiling()
{
#ifdef PANDA_WITH_ECMASCRIPT
    // Object handles can be created during class initialization, so check lock state only after GC is started.
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() || Locks::mutator_lock->HasLock() ||
           ManagedThread::GetCurrent()->GetThreadLang() == panda::panda_file::SourceLang::ECMASCRIPT);
#else
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() || Locks::mutator_lock->HasLock());
#endif

    // Some thread already started profiling
    if (IsProfilingWithoutLock()) {
        return;
    }

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    PandaVector<uint32_t> vcalls;
    PandaVector<uint32_t> branches;

    Span<const uint8_t> instructions(GetInstructions(), GetCodeSize());
    for (BytecodeInstruction inst(instructions.begin()); inst.GetAddress() < instructions.end();
         inst = inst.GetNext()) {
        if (inst.HasFlag(BytecodeInstruction::Flags::CALL_VIRT)) {
            vcalls.push_back(inst.GetAddress() - GetInstructions());
        }
        if (inst.HasFlag(BytecodeInstruction::Flags::JUMP)) {
            branches.push_back(inst.GetAddress() - GetInstructions());
        }
    }
    if (vcalls.empty() && branches.empty()) {
        return;
    }
    ASSERT(std::is_sorted(vcalls.begin(), vcalls.end()));

    auto data = allocator->Alloc(RoundUp(RoundUp(sizeof(ProfilingData), alignof(CallSiteInlineCache)) +
                                             sizeof(CallSiteInlineCache) * vcalls.size(),
                                         alignof(BranchData)) +
                                 sizeof(BranchData) * branches.size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto vcalls_mem = reinterpret_cast<uint8_t *>(data) + RoundUp(sizeof(ProfilingData), alignof(CallSiteInlineCache));
    auto branches_data_offset = RoundUp(RoundUp(sizeof(ProfilingData), alignof(CallSiteInlineCache)) +
                                            sizeof(CallSiteInlineCache) * vcalls.size(),
                                        alignof(BranchData));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto branches_mem = reinterpret_cast<uint8_t *>(data) + branches_data_offset;
    auto profiling_data = new (data)
        ProfilingData(CallSiteInlineCache::From(vcalls_mem, vcalls), BranchData::From(branches_mem, branches));

    ProfilingData *old_value = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    while (!pointer_.profiling_data_.compare_exchange_weak(old_value, profiling_data)) {
        if (old_value != nullptr) {
            // We're late, some thread already started profiling.
            allocator->Delete(data);
            return;
        }
    }
    EVENT_INTERP_PROFILING(events::InterpProfilingAction::START, GetFullName(), vcalls.size());
}

void Method::StopProfiling()
{
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() || Locks::mutator_lock->HasLock());

    if (!IsProfilingWithoutLock()) {
        return;
    }

    EVENT_INTERP_PROFILING(events::InterpProfilingAction::STOP, GetFullName(),
                           GetProfilingData()->GetInlineCaches().size());

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Free(GetProfilingData());
    // Atomic with release order reason: data race with profiling_data_ with dependecies on writes before the store
    // which should become visible acquire NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    pointer_.profiling_data_.store(nullptr, std::memory_order_release);
}

bool Method::IsProxy() const
{
    return GetClass()->IsProxy();
}

}  // namespace panda
