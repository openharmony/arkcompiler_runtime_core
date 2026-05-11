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

#include <utility>
#include <vector>
#include "ani_xmlsaxparser.h"
#include "stdlib/native/core/stdlib_ani_helpers.h"
#include "Util.h"

namespace ark::ets::sdk::util {

constexpr size_t XML_SAX2_ATTR_ELEMENT_COUNT = 5;
constexpr size_t XML_SAX2_ATTR_LOCALNAME_OFFSET = 0;
constexpr size_t XML_SAX2_ATTR_PREFIX_OFFSET = 1;
[[maybe_unused]] constexpr size_t XML_SAX2_ATTR_URI_OFFSET = 2;
constexpr size_t XML_SAX2_ATTR_VALUE_START_OFFSET = 3;
constexpr size_t XML_SAX2_ATTR_VALUE_END_OFFSET = 4;
constexpr int XML_STARTELEMENT_PARAM = 4;
constexpr int XML_ENDELEMENT_PARAM = 3;
constexpr int XML_CHARACTERS_PARAM = 1;
constexpr size_t XML_CALLBACK_BATCH_SIZE = 128;
constexpr ani_size XML_LOCAL_SCOPE_CAPACITY = 16;
constexpr int BUSINESS_ERROR_CODE = 401;
constexpr int XML_STARTELEMENT_PARAM_THIRD = 3;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
XmlSAXParserHelper::XmlSAXParserHelper(ani_env *env)
{
    env->GetVM(&vm_);
}

XmlSAXParserHelper::~XmlSAXParserHelper()
{
    Cleanup();
}

void XmlSAXParserHelper::AcquireSelf()
{
    // Atomic with relaxed order reason: reference count is only for lifetime management
    refCount_.fetch_add(1, std::memory_order_relaxed);
}

void XmlSAXParserHelper::Release()
{
    // Atomic with acq_rel order reason: synchronize with fetch_add before potential delete
    if (refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
    }
}

ani_long XmlSAXParserHelper::BindNative(ani_env *env, [[maybe_unused]] ani_class self)
{
    auto *helper = new XmlSAXParserHelper(env);
    return reinterpret_cast<ani_long>(helper);
}

ani_boolean XmlSAXParserHelper::ReleaseNative([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class self,
                                              ani_long pointer)
{
    auto *helper = reinterpret_cast<XmlSAXParserHelper *>(pointer);
    if (helper != nullptr) {
        helper->RequestShutdown();
        helper->Release();
        return ANI_TRUE;
    }
    return ANI_FALSE;
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
ani_string XmlSAXParserHelper::ParseInner(ani_env *env, [[maybe_unused]] ani_class self, ani_long pointer,
                                          ani_object parser, ani_object handler, ani_string chunk, ani_boolean isFinal)
{
    auto *helper = reinterpret_cast<XmlSAXParserHelper *>(pointer);
    std::string error;
    if (helper == nullptr) {
        error = "Parser instance is null";
    } else {
        std::string chunkStr;
        if (chunk != nullptr) {
            ani_size strSize;
            env->String_GetUTF8Size(chunk, &strSize);
            chunkStr.resize(strSize + 1);
            ani_size outSize;
            env->String_GetUTF8SubString(chunk, 0, strSize, chunkStr.data(), chunkStr.size(), &outSize);
            chunkStr.resize(outSize);
        }
        error = helper->Parse(env, parser, handler, chunkStr, isFinal == ANI_TRUE);
    }

    ani_string result;
    env->String_NewUTF8(error.c_str(), error.length(), &result);
    return result;
}

void XmlSAXParserHelper::Cleanup()
{
    if (parserCtxt_ != nullptr) {
        xmlFreeParserCtxt(parserCtxt_);
        parserCtxt_ = nullptr;
    }

    ani_env *env = nullptr;
    ani_status status = vm_->GetEnv(ANI_VERSION_1, &env);
    if (status == ANI_OK) {
        if (xmlsaxhandleRef_ != nullptr) {
            env->GlobalReference_Delete(xmlsaxhandleRef_);
            xmlsaxhandleRef_ = nullptr;
        }
        if (xmlsaxhandleCls_ != nullptr) {
            env->GlobalReference_Delete(xmlsaxhandleCls_);
            xmlsaxhandleCls_ = nullptr;
        }
        if (parserRef_ != nullptr) {
            env->GlobalReference_Delete(parserRef_);
            parserRef_ = nullptr;
        }
        if (parserCls_ != nullptr) {
            env->GlobalReference_Delete(parserCls_);
            parserCls_ = nullptr;
        }
        if (mapClsRef_ != nullptr) {
            env->GlobalReference_Delete(mapClsRef_);
            mapClsRef_ = nullptr;
        }
    }

    if (asyncWork_ != nullptr && status == ANI_OK) {
        arkts::concurrency_helpers::DeleteAsyncWork(env, asyncWork_);
        asyncWork_ = nullptr;
    }

    isInitialized_ = false;
}

bool XmlSAXParserHelper::InitParserContext()
{
    if (isInitialized_) {
        return true;
    }

    static xmlSAXHandler saxHandler = [] {
        xmlSAXHandler h = xmlSAXHandler();
        h.initialized = XML_SAX2_MAGIC;
        h.startDocument = StartDocumentCallback;
        h.endDocument = EndDocumentCallback;
        h.startElementNs = StartElementNsCallback;
        h.endElementNs = EndElementNsCallback;
        h.characters = CharactersCallback;
        return h;
    }();

    parserCtxt_ = xmlCreatePushParserCtxt(&saxHandler, this, nullptr, 0, nullptr);
    if (parserCtxt_ == nullptr) {
        SetFatalError("Failed to create XML parser context");
        return false;
    }

    isInitialized_ = true;
    return true;
}

bool XmlSAXParserHelper::ExtractCallbacks(ani_env *env, ani_object parser, ani_object handler)
{
    if (callbacksExtracted_) {
        return true;
    }
    if (handler == nullptr || parser == nullptr) {
        SetFatalError("Handler object is null");
        return false;
    }

    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(handler, &xmlsaxhandleRef_));
    ani_type anixmltype {};
    ANI_FATAL_IF_ERROR(env->Object_GetType(handler, &anixmltype));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(anixmltype, &xmlsaxhandleCls_));

    ANI_FATAL_IF_ERROR(env->Class_FindMethod(reinterpret_cast<ani_class>(xmlsaxhandleCls_), "startDocument", nullptr,
                                             &startDocumentRef_));
    ANI_FATAL_IF_ERROR(
        env->Class_FindMethod(reinterpret_cast<ani_class>(xmlsaxhandleCls_), "endDocument", nullptr, &endDocumentRef_));
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(reinterpret_cast<ani_class>(xmlsaxhandleCls_), "startElement", nullptr,
                                             &startElementRef_));
    ANI_FATAL_IF_ERROR(
        env->Class_FindMethod(reinterpret_cast<ani_class>(xmlsaxhandleCls_), "endElement", nullptr, &endElementRef_));
    ANI_FATAL_IF_ERROR(
        env->Class_FindMethod(reinterpret_cast<ani_class>(xmlsaxhandleCls_), "characters", nullptr, &charactersRef_));

    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(parser, &parserRef_));
    ani_type aniParserType {};
    ANI_FATAL_IF_ERROR(env->Object_GetType(parser, &aniParserType));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(aniParserType, &parserCls_));
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(reinterpret_cast<ani_class>(parserCls_), "handleAsyncParseComplete",
                                             nullptr, &completeRef_));
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(reinterpret_cast<ani_class>(parserCls_), "handleAsyncParseError", nullptr,
                                             &asyncErrorRef_));

    ani_class mapClass = nullptr;
    if (env->FindClass("std.core.Map", &mapClass) == ANI_OK) {
        ANI_FATAL_IF_ERROR(env->GlobalReference_Create(mapClass, &mapClsRef_));
        ANI_FATAL_IF_ERROR(env->Class_FindMethod(mapClass, "<ctor>",
                                                 "X{C{std.core.Iterable}C{std.core.Null}"
                                                 "C{std.core.ReadonlyArray}}:",
                                                 &mapConstructorRef_));
        ANI_FATAL_IF_ERROR(env->Class_FindMethod(mapClass, "set", "YY:C{std.core.Map}", &mapSetRef_));
    }

    callbacksExtracted_ = true;
    return true;
}

void XmlSAXParserHelper::SetFatalError(const std::string &error)
{
    std::lock_guard<std::mutex> lock(errorMutex_);
    if (error_.empty()) {
        error_ = error;
    }
}

std::string XmlSAXParserHelper::GetFatalError()
{
    std::lock_guard<std::mutex> lock(errorMutex_);
    return error_;
}

bool XmlSAXParserHelper::StartWorkerIfNeeded(ani_env *env)
{
    if (workerStarted_) {
        return true;
    }

    if (mainWorkerId_ < 0) {
        mainWorkerId_ = arkts::concurrency_helpers::GetWorkerId(env);
        if (mainWorkerId_ < 0) {
            SetFatalError("Failed to get current worker id");
            return false;
        }
    }

    if (arkts::concurrency_helpers::CreateAsyncWork(env, ParserWorkerExecute, ParserWorkerComplete, this,
                                                    &asyncWork_) != arkts::concurrency_helpers::WorkStatus::OK) {
        SetFatalError("Failed to create SAX parser async worker");
        return false;
    }

    AcquireSelf();
    if (arkts::concurrency_helpers::QueueAsyncWork(env, asyncWork_) != arkts::concurrency_helpers::WorkStatus::OK) {
        Release();
        arkts::concurrency_helpers::DeleteAsyncWork(env, asyncWork_);
        asyncWork_ = nullptr;
        SetFatalError("Failed to queue SAX parser async worker");
        return false;
    }

    workerStarted_ = true;
    return true;
}

void XmlSAXParserHelper::RequestShutdown()
{
    {
        std::lock_guard<std::mutex> lock(parseMutex_);
        shutdownRequested_ = true;
    }
    parseCv_.notify_all();
}

void XmlSAXParserHelper::EnqueueParseTask(const std::string &chunk, bool isFinal)
{
    {
        std::lock_guard<std::mutex> lock(parseMutex_);
        parseTasks_.push_back(ParseTask {chunk, isFinal});
    }
    parseCv_.notify_one();
}

std::string XmlSAXParserHelper::Parse(ani_env *env, ani_object parser, ani_object handler, const std::string &chunk,
                                      bool isFinal)
{
    std::string fatalError = GetFatalError();
    if (!fatalError.empty()) {
        return fatalError;
    }

    if (!ExtractCallbacks(env, parser, handler)) {
        return GetFatalError();
    }
    if (!StartWorkerIfNeeded(env)) {
        return GetFatalError();
    }

    // The caller thread only feeds chunks into the worker queue. libxml2 parsing
    // and SAX event production happen on the async worker.
    EnqueueParseTask(chunk, isFinal);
    return "";
}

void XmlSAXParserHelper::ParserWorkerExecute(ani_env *env, void *data)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(data);
    helper->ProcessParseTasks(env);
}

void XmlSAXParserHelper::ParserWorkerComplete(ani_env *env,
                                              [[maybe_unused]] arkts::concurrency_helpers::WorkStatus status,
                                              void *data)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(data);
    helper->workerStarted_ = false;
    {
        std::lock_guard<std::mutex> lock(helper->parseMutex_);
        helper->shutdownRequested_ = false;
    }
    if (helper->asyncWork_ != nullptr) {
        arkts::concurrency_helpers::DeleteAsyncWork(env, helper->asyncWork_);
        helper->asyncWork_ = nullptr;
    }

    // Parse completion is reported separately from the SAX FIFO so that
    // start/end/characters callbacks keep their original ordering semantics.
    std::string fatalError = helper->GetFatalError();
    if (fatalError.empty()) {
        helper->QueueParseResult(env, ParseResultType::COMPLETE);
    } else {
        helper->QueueParseResult(env, ParseResultType::ERROR, fatalError);
    }
    helper->Release();
}

void XmlSAXParserHelper::ProcessParseTasks([[maybe_unused]] ani_env *env)
{
    while (true) {
        ParseTask task;
        {
            std::unique_lock<std::mutex> lock(parseMutex_);
            parseCv_.wait(lock, [this]() { return shutdownRequested_ || !parseTasks_.empty(); });
            if (shutdownRequested_ && parseTasks_.empty()) {
                break;
            }
            task = std::move(parseTasks_.front());
            parseTasks_.pop_front();
        }

        if (!InitParserContext()) {
            break;
        }

        int parseResult =
            xmlParseChunk(parserCtxt_, task.chunk.c_str(), static_cast<int>(task.chunk.length()), task.isFinal ? 1 : 0);
        if (parseResult != 0) {
            std::string error = "XML parsing failed";
            const xmlError *lastError = xmlCtxtGetLastError(parserCtxt_);
            if (lastError != nullptr && lastError->message != nullptr) {
                error += ": ";
                error += lastError->message;
            }
            SetFatalError(error);
            break;
        }

        if (task.isFinal) {
            std::lock_guard<std::mutex> lock(parseMutex_);
            shutdownRequested_ = true;
            break;
        }
    }

    if (parserCtxt_ != nullptr) {
        xmlFreeParserCtxt(parserCtxt_);
        parserCtxt_ = nullptr;
    }
    isInitialized_ = false;
}

void XmlSAXParserHelper::QueueSaxEvent(ani_env *env, SaxEvent &&event)
{
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackQueue_.push_back(std::move(event));
    }
    // Worker-side SAX callbacks only enqueue data and schedule a main-worker dispatch.
    TryScheduleCallbackDispatch(env);
}

bool XmlSAXParserHelper::TryScheduleCallbackDispatch(ani_env *env)
{
    if (env == nullptr) {
        SetFatalError("Failed to get worker env for SAX callback dispatch");
        return false;
    }

    bool expected = false;
    // Atomic with acq_rel order reason: ensure prior callbacks are visible before dispatch
    if (!dispatchPending_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return true;
    }

    AcquireSelf();
    auto status = arkts::concurrency_helpers::SendEvent(env, mainWorkerId_, DispatchQueuedCallbacks, this);
    if (status != arkts::concurrency_helpers::WorkStatus::OK) {
        // Atomic with release order reason: reset flag on SendEvent failure
        dispatchPending_.store(false, std::memory_order_release);
        Release();
        SetFatalError("Failed to dispatch SAX callbacks to the main worker");
        return false;
    }
    return true;
}

void XmlSAXParserHelper::QueueParseResult(ani_env *env, ParseResultType type, std::string error)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        parseResultType_ = type;
        parseResultError_ = std::move(error);
    }
    TryScheduleParseResultDispatch(env);
}

bool XmlSAXParserHelper::TryScheduleParseResultDispatch(ani_env *env)
{
    if (env == nullptr) {
        SetFatalError("Failed to get worker env for SAX parse result dispatch");
        return false;
    }

    bool expected = false;
    // Atomic with acq_rel order reason: synchronize with result store before dispatch
    if (!parseResultDispatchPending_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return true;
    }

    AcquireSelf();
    auto status = arkts::concurrency_helpers::SendEvent(env, mainWorkerId_, DispatchParseResult, this);
    if (status != arkts::concurrency_helpers::WorkStatus::OK) {
        // Atomic with release order reason: reset flag on SendEvent failure
        parseResultDispatchPending_.store(false, std::memory_order_release);
        Release();
        SetFatalError("Failed to dispatch SAX parse result to the main worker");
        return false;
    }
    return true;
}

void XmlSAXParserHelper::DispatchQueuedCallbacks(void *data)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(data);
    helper->ExecuteQueuedCallbacks();
    // Atomic with release order reason: dispatch complete, allow new dispatches
    helper->dispatchPending_.store(false, std::memory_order_release);

    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK) {
        bool hasPendingCallbacks = false;
        {
            std::lock_guard<std::mutex> lock(helper->callbackMutex_);
            hasPendingCallbacks = !helper->callbackQueue_.empty();
        }
        if (hasPendingCallbacks) {
            helper->TryScheduleCallbackDispatch(env);
        }
    }

    helper->Release();
}

void XmlSAXParserHelper::DispatchParseResult(void *data)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(data);
    // Atomic with release order reason: parse result dispatch complete
    helper->parseResultDispatchPending_.store(false, std::memory_order_release);

    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK) {
        // Atomic with acquire order reason: check if callback dispatch is pending
        bool hasPendingCallbacks = helper->dispatchPending_.load(std::memory_order_acquire);
        if (!hasPendingCallbacks) {
            std::lock_guard<std::mutex> lock(helper->callbackMutex_);
            hasPendingCallbacks = !helper->callbackQueue_.empty();
        }
        if (hasPendingCallbacks) {
            // Preserve competitor semantics: drain already-produced SAX callbacks
            // before reporting the final parse outcome to ETS.
            helper->TryScheduleParseResultDispatch(env);
        } else {
            helper->FinalizeSession(env);
        }
    }

    helper->Release();
}

void XmlSAXParserHelper::ResetSession(ani_env *env)
{
    if (env == nullptr) {
        return;
    }

    if (parserCtxt_ != nullptr) {
        xmlFreeParserCtxt(parserCtxt_);
        parserCtxt_ = nullptr;
    }

    if (xmlsaxhandleRef_ != nullptr) {
        env->GlobalReference_Delete(xmlsaxhandleRef_);
        xmlsaxhandleRef_ = nullptr;
    }
    if (xmlsaxhandleCls_ != nullptr) {
        env->GlobalReference_Delete(xmlsaxhandleCls_);
        xmlsaxhandleCls_ = nullptr;
    }
    if (parserRef_ != nullptr) {
        env->GlobalReference_Delete(parserRef_);
        parserRef_ = nullptr;
    }
    if (parserCls_ != nullptr) {
        env->GlobalReference_Delete(parserCls_);
        parserCls_ = nullptr;
    }
    if (mapClsRef_ != nullptr) {
        env->GlobalReference_Delete(mapClsRef_);
        mapClsRef_ = nullptr;
    }

    startDocumentRef_ = nullptr;
    endDocumentRef_ = nullptr;
    startElementRef_ = nullptr;
    endElementRef_ = nullptr;
    charactersRef_ = nullptr;
    completeRef_ = nullptr;
    asyncErrorRef_ = nullptr;
    mapConstructorRef_ = nullptr;
    mapSetRef_ = nullptr;

    {
        std::lock_guard<std::mutex> lock(parseMutex_);
        shutdownRequested_ = false;
        parseTasks_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackQueue_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(errorMutex_);
        error_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        parseResultType_ = ParseResultType::NONE;
        parseResultError_.clear();
    }

    callbacksExtracted_ = false;
    // Atomic with release order reason: reset dispatch flags for next session
    dispatchPending_.store(false, std::memory_order_release);
    // Atomic with release order reason: reset result dispatch flag for next session
    parseResultDispatchPending_.store(false, std::memory_order_release);
    isInitialized_ = false;
}

void XmlSAXParserHelper::FinalizeSession(ani_env *env)
{
    if (env == nullptr) {
        return;
    }

    ParseResultType resultType = ParseResultType::NONE;
    std::string resultError;
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        resultType = parseResultType_;
        resultError = parseResultError_;
    }
    if (resultType == ParseResultType::NONE) {
        return;
    }

    // Preserve any callback exception that is already pending on the main worker,
    // then restore it after session teardown.
    ani_error preservedError = nullptr;
    ani_boolean hasPendingError = ANI_FALSE;
    if (env->ExistUnhandledError(&hasPendingError) == ANI_OK && hasPendingError == ANI_TRUE) {
        env->GetUnhandledError(&preservedError);
        env->ResetError();
    }

    switch (resultType) {
        case ParseResultType::COMPLETE:
            if (completeRef_ != nullptr && parserRef_ != nullptr) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                env->Object_CallMethod_Void(static_cast<ani_object>(parserRef_), completeRef_);
            }
            break;
        case ParseResultType::ERROR:
            if (asyncErrorRef_ != nullptr && parserRef_ != nullptr) {
                ani_string errorValue = CreateStringValue(env, resultError);
                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_value xmlargs[XML_CHARACTERS_PARAM];
                xmlargs[0].r = errorValue;
                env->Object_CallMethod_Void_A(static_cast<ani_object>(parserRef_), asyncErrorRef_, xmlargs);
            } else {
                ThrowBusinessError(env, BUSINESS_ERROR_CODE, resultError);
            }
            break;
        default:
            break;
    }

    ani_error sessionError = nullptr;
    ani_boolean hasSessionError = ANI_FALSE;
    if (env->ExistUnhandledError(&hasSessionError) == ANI_OK && hasSessionError == ANI_TRUE) {
        env->GetUnhandledError(&sessionError);
        env->ResetError();
    }

    ResetSession(env);

    if (preservedError != nullptr) {
        env->ThrowError(preservedError);
        return;
    }
    if (sessionError != nullptr) {
        env->ThrowError(sessionError);
    }
}

void XmlSAXParserHelper::ExecuteQueuedCallbacks()
{
    ani_env *env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        return;
    }

    std::deque<SaxEvent> localQueue;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        size_t batchSize = std::min(callbackQueue_.size(), XML_CALLBACK_BATCH_SIZE);
        for (size_t i = 0; i < batchSize; i++) {
            localQueue.push_back(std::move(callbackQueue_.front()));
            callbackQueue_.pop_front();
        }
    }

    for (auto &event : localQueue) {
        bool localScopeCreated = env->CreateLocalScope(XML_LOCAL_SCOPE_CAPACITY) == ANI_OK;
        switch (event.type) {
            case SaxEventType::START_DOCUMENT:
                if (startDocumentRef_ != nullptr) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    env->Object_CallMethod_Void(static_cast<ani_object>(xmlsaxhandleRef_), startDocumentRef_);
                }
                break;
            case SaxEventType::END_DOCUMENT:
                if (endDocumentRef_ != nullptr) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    env->Object_CallMethod_Void(static_cast<ani_object>(xmlsaxhandleRef_), endDocumentRef_);
                }
                break;
            case SaxEventType::START_ELEMENT: {
                if (startElementRef_ == nullptr) {
                    break;
                }
                ani_string nameValue = CreateStringValue(env, event.name);
                ani_string uriValue = CreateStringValue(env, event.uri);
                ani_string qnameValue = CreateStringValue(env, event.qname);
                ani_ref undefinedValue = nullptr;
                env->GetUndefined(&undefinedValue);

                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_ref args[XML_STARTELEMENT_PARAM] = {
                    static_cast<ani_ref>(nameValue),
                    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                    uriValue ? static_cast<ani_ref>(uriValue) : undefinedValue,
                    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                    qnameValue ? static_cast<ani_ref>(qnameValue) : undefinedValue,
                    undefinedValue,
                };
                ani_object mapObj = CreateAttributesMap(env, event.attributes);
                if (mapObj != nullptr) {
                    args[XML_STARTELEMENT_PARAM_THIRD] = static_cast<ani_ref>(mapObj);
                }

                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_value xmlargs[XML_STARTELEMENT_PARAM];
                for (int i = 0; i < XML_STARTELEMENT_PARAM; i++) {
                    xmlargs[i].r = args[i];
                }
                env->Object_CallMethod_Void_A(static_cast<ani_object>(xmlsaxhandleRef_), startElementRef_, xmlargs);
                break;
            }
            case SaxEventType::END_ELEMENT: {
                if (endElementRef_ == nullptr) {
                    break;
                }
                ani_string nameValue = CreateStringValue(env, event.name);
                ani_string uriValue = CreateStringValue(env, event.uri);
                ani_string qnameValue = CreateStringValue(env, event.qname);
                ani_ref undefinedValue = nullptr;
                env->GetUndefined(&undefinedValue);

                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_ref args[XML_ENDELEMENT_PARAM] = {
                    static_cast<ani_ref>(nameValue),
                    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                    uriValue ? static_cast<ani_ref>(uriValue) : undefinedValue,
                    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                    qnameValue ? static_cast<ani_ref>(qnameValue) : undefinedValue,
                };

                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_value xmlargs[XML_ENDELEMENT_PARAM];
                for (int i = 0; i < XML_ENDELEMENT_PARAM; i++) {
                    xmlargs[i].r = args[i];
                }
                env->Object_CallMethod_Void_A(static_cast<ani_object>(xmlsaxhandleRef_), endElementRef_, xmlargs);
                break;
            }
            case SaxEventType::CHARACTERS: {
                if (charactersRef_ == nullptr) {
                    break;
                }
                ani_string contentValue = CreateStringValue(env, event.content);
                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                ani_value xmlargs[XML_CHARACTERS_PARAM];
                xmlargs[0].r = contentValue;
                env->Object_CallMethod_Void_A(static_cast<ani_object>(xmlsaxhandleRef_), charactersRef_, xmlargs);
                break;
            }
            default:
                break;
        }
        if (localScopeCreated) {
            env->DestroyLocalScope();
        }
    }
}

// NOLINTNEXTLINE(misc-unused-parameters)
std::map<std::string, std::string> XmlSAXParserHelper::ConvertSAX2Attributes(const xmlChar **attrs, int attributeSize)
{
    std::map<std::string, std::string> attrMap;
    for (int i = 0; i < attributeSize; i++) {
        int offset = i * XML_SAX2_ATTR_ELEMENT_COUNT;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const xmlChar *localname = attrs[offset + XML_SAX2_ATTR_LOCALNAME_OFFSET];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const xmlChar *prefix = attrs[offset + XML_SAX2_ATTR_PREFIX_OFFSET];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const xmlChar *valueStart = attrs[offset + XML_SAX2_ATTR_VALUE_START_OFFSET];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const xmlChar *valueEnd = attrs[offset + XML_SAX2_ATTR_VALUE_END_OFFSET];

        if (localname != nullptr && valueStart != nullptr && valueEnd != nullptr) {
            std::string attrName;
            const char *localnameStr = reinterpret_cast<const char *>(localname);

            if (prefix != nullptr && *prefix != 0) {
                const char *prefixStr = reinterpret_cast<const char *>(prefix);
                attrName = prefixStr;
                attrName += ":";
                attrName += localnameStr;
            } else {
                attrName = localnameStr;
            }

            const char *valueStartStr = reinterpret_cast<const char *>(valueStart);
            const char *valueEndStr = reinterpret_cast<const char *>(valueEnd);
            std::string value(valueStartStr, valueEndStr - valueStartStr);
            attrMap[attrName] = value;
        }
    }
    return attrMap;
}

// NOLINTNEXTLINE(misc-unused-parameters)
ani_object XmlSAXParserHelper::CreateAttributesMap(ani_env *env, const std::map<std::string, std::string> &attrMap)
{
    if (mapClsRef_ == nullptr || mapConstructorRef_ == nullptr) {
        return nullptr;
    }
    auto mapClass = reinterpret_cast<ani_class>(mapClsRef_);

    ani_object attributesMap = nullptr;
    ani_ref undefinedRef {};
    env->GetUndefined(&undefinedRef);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (env->Object_New(mapClass, mapConstructorRef_, &attributesMap, undefinedRef) != ANI_OK) {
        return nullptr;
    }

    if (mapSetRef_ == nullptr) {
        return attributesMap;
    }

    for (const auto &[key, value] : attrMap) {
        ani_string keyStr;
        env->String_NewUTF8(key.c_str(), key.length(), &keyStr);
        ani_string valueStr;
        env->String_NewUTF8(value.c_str(), value.length(), &valueStr);
        ani_ref setResult = nullptr;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Object_CallMethod_Ref(attributesMap, mapSetRef_, &setResult, keyStr, valueStr);
    }

    return attributesMap;
}

ani_string XmlSAXParserHelper::CreateStringValue(ani_env *env, const std::string &str)
{
    ani_string value = nullptr;
    if (!str.empty()) {
        env->String_NewUTF8(str.c_str(), str.length(), &value);
    }
    return value;
}

void XmlSAXParserHelper::StartDocumentCallback(void *userData)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(userData);
    SaxEvent event;
    event.type = SaxEventType::START_DOCUMENT;
    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        helper->SetFatalError("Failed to get worker env in startDocument callback");
        return;
    }
    helper->QueueSaxEvent(env, std::move(event));
}

void XmlSAXParserHelper::EndDocumentCallback(void *userData)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(userData);
    SaxEvent event;
    event.type = SaxEventType::END_DOCUMENT;
    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        helper->SetFatalError("Failed to get worker env in endDocument callback");
        return;
    }
    helper->QueueSaxEvent(env, std::move(event));
}

// CC-OFFNXT(readability-function-size_parameters)
void XmlSAXParserHelper::StartElementNsCallback(void *userData, [[maybe_unused]] const xmlChar *localname,
                                                [[maybe_unused]] const xmlChar *prefix,
                                                [[maybe_unused]] const xmlChar *uri, [[maybe_unused]] int namespaceSize,
                                                [[maybe_unused]] const xmlChar **namespaces, int attributeSize,
                                                [[maybe_unused]] int defaultedSize,
                                                [[maybe_unused]] const xmlChar **attributes)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(userData);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    std::string nameStr = localname ? reinterpret_cast<const char *>(localname) : "";
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    std::string uriStr = uri ? reinterpret_cast<const char *>(uri) : "";
    std::string qnameStr;
    if (prefix != nullptr && *prefix != 0) {
        qnameStr = reinterpret_cast<const char *>(prefix);
        qnameStr += ":";
        qnameStr += reinterpret_cast<const char *>(localname);
    } else {
        qnameStr = "";
    }

    SaxEvent event;
    event.type = SaxEventType::START_ELEMENT;
    event.name = std::move(nameStr);
    event.uri = std::move(uriStr);
    event.qname = std::move(qnameStr);
    event.attributes = helper->ConvertSAX2Attributes(attributes, attributeSize);
    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        helper->SetFatalError("Failed to get worker env in startElement callback");
        return;
    }
    helper->QueueSaxEvent(env, std::move(event));
}

// NOLINTNEXTLINE(misc-unused-parameters)
void XmlSAXParserHelper::EndElementNsCallback(void *userData, const xmlChar *localname, const xmlChar *prefix,
                                              [[maybe_unused]] const xmlChar *uri)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(userData);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    std::string nameStr = localname ? reinterpret_cast<const char *>(localname) : "";
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    std::string uriStr = uri ? reinterpret_cast<const char *>(uri) : "";
    std::string qnameStr;
    if (prefix != nullptr && *prefix != 0) {
        qnameStr = reinterpret_cast<const char *>(prefix);
        qnameStr += ":";
        qnameStr += reinterpret_cast<const char *>(localname);
    } else {
        qnameStr = "";
    }

    SaxEvent event;
    event.type = SaxEventType::END_ELEMENT;
    event.name = std::move(nameStr);
    event.uri = std::move(uriStr);
    event.qname = std::move(qnameStr);
    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        helper->SetFatalError("Failed to get worker env in endElement callback");
        return;
    }
    helper->QueueSaxEvent(env, std::move(event));
}

void XmlSAXParserHelper::CharactersCallback(void *userData, [[maybe_unused]] const xmlChar *xmlCh, int len)
{
    auto *helper = static_cast<XmlSAXParserHelper *>(userData);
    std::string contentStr;
    if (xmlCh != nullptr && len > 0) {
        contentStr = std::string(reinterpret_cast<const char *>(xmlCh), len);
    }

    SaxEvent event;
    event.type = SaxEventType::CHARACTERS;
    event.content = std::move(contentStr);
    ani_env *env = nullptr;
    if (helper->vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        helper->SetFatalError("Failed to get worker env in characters callback");
        return;
    }
    helper->QueueSaxEvent(env, std::move(event));
}

}  // namespace ark::ets::sdk::util
