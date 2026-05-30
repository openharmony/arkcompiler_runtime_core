/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ani/verify/env_ani_verifier.h"

#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ani/ani_converters.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::ani::verify {

constexpr ani_size VERIFICATION_BOTTOM_FRAME = 0;
constexpr ani_size DEFAULT_NATIVE_FRAME_CAPACITY = 4096;

EnvANIVerifier::EnvANIVerifier(PandaAniEnv *ownerEnv, ANIVerifier *verifier,
                               const __ani_interaction_api *interactionAPI)
    : verifier_(verifier), interactionAPI_(interactionAPI)
{
    DoPushNativeFrame(ownerEnv, VERIFICATION_BOTTOM_FRAME);
}

VEnv *EnvANIVerifier::GetEnv()
{
    ASSERT(!frames_.empty());
    return frames_.back().venv;
}

VEnv *EnvANIVerifier::AttachThread()
{
    ASSERT(!frames_.empty());
    VEnv *curEnv = frames_.back().venv;
    // Threads attached through ANI do not pass through a managed native-call bridge, so verifyANI creates the
    // native frame explicitly and uses it to track local references until DetachCurrentThread().
    PushNativeFrame(PandaAniEnv::FromAniEnv(curEnv->GetEnv()));
    return frames_.back().venv;
}

void EnvANIVerifier::DetachThread()
{
    auto err = PopNativeFrame();
    if (err) {
        verifier_->Report(*err);
    }
}

void EnvANIVerifier::ReportDetachOnUnattachedThread(ani_vm *vm)
{
    PandaEtsVM::FromAniVM(vm)->GetANIVerifier()->Report("Cannot detach current thread, thread is not attached");
}

void EnvANIVerifier::DoPushNativeFrame(PandaAniEnv *ownerEnv, ani_size capacity)
{
    Frame frame {SubFrameType::NATIVE_FRAME,
                 0,
                 {},
                 MakePandaUnique<ArenaAllocator>(SpaceType::SPACE_TYPE_INTERNAL),
                 nullptr,
                 VEnv::Create(ownerEnv),
                 nullptr,
                 capacity};
    frame.refsAllocator = frame.refsAllocatorHolder.get();
    frame.venv = frame.venvHolder.get();
    frames_.push_back(std::move(frame));
}

void EnvANIVerifier::PushNativeFrame(PandaAniEnv *ownerEnv)
{
    ASSERT(!frames_.empty());
    DoPushNativeFrame(ownerEnv, DEFAULT_NATIVE_FRAME_CAPACITY);
}

std::optional<PandaString> EnvANIVerifier::PopNativeFrame()
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            frames_.pop_back();
            return {};
        case SubFrameType::LOCAL_SCOPE:
            return "It is necessary to call DestroyLocalScope(), after calling CreateLocalScope()";
        case SubFrameType::ESCAPE_LOCAL_SCOPE:
            return "It is necessary to call DestroyEscapeLocalScope(), after calling CreateEscapeLocalScope()";
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

void EnvANIVerifier::CreateLocalScope(ani_size capacity)
{
    ASSERT(!frames_.empty());
    ArenaAllocator *allocator = frames_.back().refsAllocator;
    VEnv *venv = frames_.back().venv;
    frames_.push_back(Frame {SubFrameType::LOCAL_SCOPE, 0, {}, {}, allocator, {}, venv, capacity});
}

std::optional<PandaString> EnvANIVerifier::DestroyLocalScope()
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            return "Illegal call DestroyLocalScope() without first calling CreateLocalScope()";
        case SubFrameType::LOCAL_SCOPE:
            frames_.pop_back();
            return {};
        case SubFrameType::ESCAPE_LOCAL_SCOPE:
            return "Illegal call DestroyLocalScope(), after calling CreateEscapeLocalScope()";
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

void EnvANIVerifier::CreateEscapeLocalScope(ani_size capacity)
{
    ASSERT(!frames_.empty());
    ArenaAllocator *allocator = frames_.back().refsAllocator;
    VEnv *venv = frames_.back().venv;
    frames_.push_back(Frame {SubFrameType::ESCAPE_LOCAL_SCOPE, 0, {}, {}, allocator, {}, venv, capacity});
}

std::optional<PandaString> EnvANIVerifier::DestroyEscapeLocalScope([[maybe_unused]] VRef *vref)
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            return "Illegal call DestroyEscapeLocalScope() without first calling CreateEscapeLocalScope()";
        case SubFrameType::LOCAL_SCOPE:
            return "Illegal call DestroyEscapeLocalScope() after calling CreateLocalScope()";
        case SubFrameType::ESCAPE_LOCAL_SCOPE: {
            // Drop top frame
            frames_.pop_back();
            return {};
        }
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

VRef *EnvANIVerifier::AddLocalVerifiedRef(ani_ref ref)
{
    ASSERT(!frames_.empty());

    // frames_ always keeps one bottom frame for verifier-owned state; real native scopes are pushed above it.
    if (UNLIKELY(frames_.size() == 1)) {
        PandaStringStream ss;
        ss << "Local reference created outside of any native scope";
        verifier_->Report(ss.str());
    }

    Frame &frame = frames_.back();

    // The verification bottom frame is not a real ANI scope and does not participate in capacity checks.
    if (UNLIKELY(frame.capacity != VERIFICATION_BOTTOM_FRAME && frame.refs.size() >= frame.capacity)) {
        PandaStringStream ss;
        ss << "Creating " << (frame.refs.size() + 1) << "th local reference in scope with capacity " << frame.capacity;
        verifier_->Report(ss.str());
    }

    InternalRef *iref = frame.refsAllocator->New<InternalRef>(ref);
    auto vref = InternalRef::CastToVRef(iref);
    [[maybe_unused]] auto ret = frame.refs.emplace(vref);
    ASSERT(ret.second);
    return vref;
}

VMethod *EnvANIVerifier::GetVerifiedMethod(ani_method method)
{
    return static_cast<VMethod *>(verifier_->AddMethod(ToInternalMethod(method)));
}

VStaticMethod *EnvANIVerifier::GetVerifiedStaticMethod(ani_static_method staticMethod)
{
    return static_cast<VStaticMethod *>(verifier_->AddMethod(ToInternalMethod(staticMethod)));
}

VFunction *EnvANIVerifier::GetVerifiedFunction(ani_function function)
{
    return static_cast<VFunction *>(verifier_->AddMethod(ToInternalMethod(function)));
}

VField *EnvANIVerifier::GetVerifiedField(ani_field field)
{
    return static_cast<VField *>(verifier_->AddField(ToInternalField(field)));
}

VStaticField *EnvANIVerifier::GetVerifiedStaticField(ani_static_field staticField)
{
    return static_cast<VStaticField *>(verifier_->AddField(ToInternalField(staticField)));
}

VVariable *EnvANIVerifier::GetVerifiedVariable(ani_variable variable)
{
    return static_cast<VVariable *>(verifier_->AddField(ToInternalField(variable)));
}

void EnvANIVerifier::DeleteLocalVerifiedRef(VRef *vref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();

    auto it = frame.refs.find(vref);
    if (it == frame.refs.cend()) {
        LOG(ERROR, RUNTIME) << "Attempted to delete non-existent localRef: " << vref;
        return;
    }
    ASSERT(it != frame.refs.cend());
    frame.refs.erase(it);
}

bool EnvANIVerifier::IsValidLocalRef(VRef *vref) const
{
    ASSERT(!frames_.empty());
    for (auto it = frames_.crbegin(); it != frames_.crend(); ++it) {
        if (IsValidRefInFrame(*it, vref)) {
            return true;
        }
        if (it->frameType == SubFrameType::NATIVE_FRAME) {
            return false;
        }
    }
    return false;
}

bool EnvANIVerifier::IsValidRefInCurrentScope(VRef *vref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();
    auto it = frame.refs.find(vref);

    return (it != frame.refs.cend());
}

bool EnvANIVerifier::IsValidWeakRef(VWRef *vwref) const
{
    return verifier_->IsValidWeakRef(vwref);
}

bool EnvANIVerifier::IsValidRef(VRef *vref) const
{
    if (IsValidWeakRef(reinterpret_cast<VWRef *>(vref))) {
        return true;
    }
    if (IsValidLocalRef(vref)) {
        return true;
    }
    if (IsValidGlobalVerifiedRef(vref)) {
        return true;
    }
    if (IsValidStackRef(vref)) {
        return true;
    }
    return false;
}

bool EnvANIVerifier::IsValidGlobalVerifiedRef(VRef *vref) const
{
    return verifier_->IsValidGlobalVerifiedRef(vref);
}

bool EnvANIVerifier::IsValidMethod(impl::VMethod *vmethod) const
{
    return verifier_->IsValidMethod(vmethod);
}

bool EnvANIVerifier::IsValidField(impl::VField *vfield) const
{
    return verifier_->IsValidField(vfield);
}

bool EnvANIVerifier::CanBeDeletedFromCurrentScope(VRef *vref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();
    return IsValidRefInFrame(frame, vref);
}

/*static*/
bool EnvANIVerifier::IsValidRefInFrame(const Frame &frame, VRef *vref)
{
    return frame.refs.find(vref) != frame.refs.cend();
}

VRef *EnvANIVerifier::AddGlobalVerifiedRef(ani_ref gref)
{
    return verifier_->AddGlobalVerifiedRef(gref);
}

void EnvANIVerifier::DeleteGlobalVerifiedRef(VRef *vgref)
{
    if (!IsValidGlobalVerifiedRef(vgref)) {
        return;
    }
    verifier_->DeleteGlobalVerifiedRef(vgref);
}

VWRef *EnvANIVerifier::AddVerifiedWeakRef(ani_wref wref)
{
    return verifier_->AddVerifiedWeakRef(wref);
}

void EnvANIVerifier::DeleteVerifiedWeakRef(VWRef *vwref)
{
    if (!verifier_->IsValidWeakRef(vwref)) {
        return;
    }
    verifier_->DeleteVerifiedWeakRef(vwref);
}

bool EnvANIVerifier::IsValidStackRef(VRef *vref) const
{
    return verifier_->IsValidStackRef(vref);
}

VResolver *EnvANIVerifier::AddGlobalVerifiedResolver(ani_resolver resolver)
{
    return verifier_->AddGlobalVerifiedResolver(resolver);
}

void EnvANIVerifier::DeleteGlobalResolver(VResolver *vresolver)
{
    return verifier_->DeleteGlobalResolver(vresolver);
}

bool EnvANIVerifier::IsValidGlobalResolver(VResolver *vresolver) const
{
    return verifier_->IsValidGlobalResolver(vresolver);
}

bool EnvANIVerifier::IsInNativeFrame() const
{
    return !frames_.empty() && frames_.back().frameType == SubFrameType::NATIVE_FRAME;
}
bool EnvANIVerifier::IsInLocalScope() const
{
    return !frames_.empty() && frames_.back().frameType == SubFrameType::LOCAL_SCOPE;
}
bool EnvANIVerifier::IsInEscapeLocalScope() const
{
    return !frames_.empty() && frames_.back().frameType == SubFrameType::ESCAPE_LOCAL_SCOPE;
}

}  // namespace ark::ets::ani::verify
