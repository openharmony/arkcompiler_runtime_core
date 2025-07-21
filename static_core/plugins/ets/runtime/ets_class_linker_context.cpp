/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_class_linker_context.h"

#include "ets_platform_types.h"
#include "include/class.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

namespace {

class DecoratorErrorHandler final : public ClassLinkerErrorHandler {
public:
    explicit DecoratorErrorHandler(ClassLinkerErrorHandler *errorHandler) : errorHandler_(errorHandler) {}

    void OnError(ClassLinker::Error error, const PandaString &message) override
    {
        lastError_.emplace(error, message);
    }

    bool HasError() const
    {
        return lastError_.has_value();
    }

    ClassLinker::Error GetErrorCode() const
    {
        ASSERT(HasError());
        return lastError_->first;
    }

    void PropagateError()
    {
        ASSERT(HasError());
        if (errorHandler_ != nullptr) {
            errorHandler_->OnError(lastError_->first, lastError_->second);
        }
    }

    void ClearError()
    {
        lastError_.reset();
    }

private:
    ClassLinkerErrorHandler *errorHandler_ {nullptr};
    std::optional<std::pair<ClassLinker::Error, PandaString>> lastError_;
};

void ReportClassNotFound(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler)
{
    if (errorHandler != nullptr) {
        PandaStringStream ss;
        ss << "Cannot find class " << descriptor;
        errorHandler->OnError(ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
    }
}

Class *LoadFromBootContext(const uint8_t *descriptor, DecoratorErrorHandler &errorHandler, ClassLinkerContext *ctx)
{
    ASSERT(ctx->IsBootContext());
    auto *klass = Runtime::GetCurrent()->GetClassLinker()->GetClass(descriptor, true, ctx, &errorHandler);
    if (errorHandler.HasError()) {
        if (errorHandler.GetErrorCode() == ClassLinker::Error::CLASS_NOT_FOUND) {
            // Clear the error in order to delegate resolution.
            errorHandler.ClearError();
        } else {
            // Report errors occurred during class loading.
            errorHandler.PropagateError();
        }
    }
    return klass;
}

bool TryLoadingClassInChain(const uint8_t *descriptor, DecoratorErrorHandler &errorHandler, ClassLinkerContext *ctx,
                            Class **klass)
{
    ASSERT(ctx != nullptr);
    ASSERT(klass != nullptr);
    ASSERT(*klass == nullptr);

    if (ctx->IsBootContext()) {
        // No need to load by managed code, even if error occurred during loading.
        *klass = LoadFromBootContext(descriptor, errorHandler, ctx);
        return true;
    }

    // All non-boot contexts are represented by EtsClassLinkerContext.
    auto *etsLinkerContext = reinterpret_cast<EtsClassLinkerContext *>(ctx);
    auto *runtimeLinker = etsLinkerContext->GetRuntimeLinker();
    ASSERT(runtimeLinker != nullptr);
    if (runtimeLinker->GetClass() != PlatformTypes()->coreAbcRuntimeLinker &&
        runtimeLinker->GetClass() != PlatformTypes()->memoryRuntimeLinker) {
        // Must call managed implementation.
        return false;
    }

    auto *abcRuntimeLinker = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker);
    auto *parentLinker = abcRuntimeLinker->GetParentLinker();
    ASSERT(parentLinker != nullptr);
    auto *parentContext = parentLinker->GetClassLinkerContext();
    if (!TryLoadingClassInChain(descriptor, errorHandler, parentContext, klass)) {
        // Chain resolution failed, must call managed implementation.
        ASSERT(*klass == nullptr);
        return false;
    }

    // No need to load by managed code, even if error occurred during loading.
    if (*klass == nullptr && !errorHandler.HasError()) {
        *klass = etsLinkerContext->FindAndLoadClass(descriptor, &errorHandler);
    }
    return true;
}

/// @brief Walks through RuntimeLinker chain and enumerates all panda files
bool TryEnumeratePandaFilesInChain(const ClassLinkerContext *ctx,
                                   const std::function<bool(const panda_file::File &)> &cb)
{
    ASSERT(ctx != nullptr);

    if (ctx->IsBootContext()) {
        ctx->EnumeratePandaFiles(cb);
        return true;
    }

    // All non-boot contexts are represented by EtsClassLinkerContext
    auto *etsLinkerContext = reinterpret_cast<const EtsClassLinkerContext *>(ctx);
    auto *runtimeLinker = etsLinkerContext->GetRuntimeLinker();
    ASSERT(runtimeLinker != nullptr);
    if (!runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        // Unexpected behavior, cannot walk through chain
        return false;
    }

    auto *abcRuntimeLinker = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker);
    auto *parentLinker = abcRuntimeLinker->GetParentLinker();
    ASSERT(parentLinker != nullptr);
    auto *parentContext = parentLinker->GetClassLinkerContext();
    if (!TryEnumeratePandaFilesInChain(parentContext, cb)) {
        return false;
    }

    parentContext->EnumeratePandaFiles(cb);
    return true;
}

}  // namespace

Class *EtsClassLinkerContext::LoadClass(const uint8_t *descriptor, [[maybe_unused]] bool needCopyDescriptor,
                                        ClassLinkerErrorHandler *errorHandler)
{
    auto *klass = FindClass(descriptor);
    if (klass != nullptr) {
        return klass;
    }

    ASSERT(!PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    // Try loading the class without invoking managed code.
    auto succeeded = TryLoadingClassFromNative(descriptor, errorHandler, &klass);
    if (succeeded) {
        return klass;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    if (coro == nullptr || !coro->IsManagedCode()) {
        // Do not invoke managed code from non-managed threads (e.g. JIT or AOT).
        ReportClassNotFound(descriptor, errorHandler);
        return nullptr;
    }

    auto clsName = ClassHelper::GetName(descriptor);
    auto etsClsName = EtsString::CreateFromMUtf8(clsName.c_str());
    const auto *runtimeLinker = GetRuntimeLinker();
    ASSERT(runtimeLinker != nullptr);
    ASSERT(etsClsName != nullptr);
    std::array args {Value(runtimeLinker->GetCoreType()), Value(etsClsName->GetCoreType()), Value(ETS_TRUE)};

    auto *loadClass = runtimeLinker->GetClass()->GetInstanceMethod("loadClass", "Lstd/core/String;Z:Lstd/core/Class;");
    ASSERT(loadClass != nullptr);
    auto res = loadClass->GetPandaMethod()->Invoke(coro, args.data());
    if (!coro->HasPendingException()) {
        return Class::FromClassObject(res.GetAs<ObjectHeader *>());
    }

    ReportClassNotFound(descriptor, errorHandler);
    return nullptr;
}

void EtsClassLinkerContext::EnumeratePandaFilesImpl(const std::function<bool(const panda_file::File &)> &cb) const
{
    ASSERT(PandaEtsVM::GetCurrent()->GetMutatorLock()->HasLock());
    auto *runtimeLinker = GetRuntimeLinker();
    ASSERT(runtimeLinker != nullptr);
    if (!runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        return;
    }
    auto *contextAbcFiles = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker)->GetAbcFiles();
    ASSERT(contextAbcFiles != nullptr);
    for (size_t i = 0, end = contextAbcFiles->GetLength(); i < end; ++i) {
        auto *pf = EtsAbcFile::FromEtsObject(contextAbcFiles->Get(i))->GetPandaFile();
        if (!cb(*pf)) {
            break;
        }
    }
}

void EtsClassLinkerContext::EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> &cb) const
{
    EnumeratePandaFilesImpl(cb);
}

void EtsClassLinkerContext::EnumeratePandaFilesInChain(const std::function<bool(const panda_file::File &)> &cb) const
{
    [[maybe_unused]] bool succeeded = TryEnumeratePandaFilesInChain(this, cb);
    // `RuntimeLinker` chain must be always traversed correctly,
    // because `AbcRuntimeLinker` is the only base class for all user-defined linkers
    ASSERT(succeeded);
}

Class *EtsClassLinkerContext::FindAndLoadClass(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler)
{
    Class *klass = nullptr;
    EnumeratePandaFilesImpl([this, &klass, descriptor, errorHandler](const auto &pf) {
        auto classId = pf.GetClassId(descriptor);
        if (!classId.IsValid() || pf.IsExternal(classId)) {
            return true;
        }
        klass = Runtime::GetCurrent()->GetClassLinker()->LoadClass(pf, classId, this, errorHandler);
        return false;
    });
    return klass;
}

bool EtsClassLinkerContext::TryLoadingClassFromNative(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler,
                                                      Class **klass)
{
    ASSERT(klass != nullptr);
    ASSERT(*klass == nullptr);

    DecoratorErrorHandler handler(errorHandler);
    auto succeeded = TryLoadingClassInChain(descriptor, handler, this, klass);
    if (handler.HasError()) {
        // Report errors occurred during class loading
        ASSERT(*klass == nullptr);
        handler.PropagateError();
    } else if (succeeded && *klass == nullptr) {
        ReportClassNotFound(descriptor, errorHandler);
    }
    return succeeded;
}

}  // namespace ark::ets
