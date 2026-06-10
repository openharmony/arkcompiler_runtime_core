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

// CC-OFFNXT(NPC_RULE_ID_HEADER_FILE_PROTECT) buggy code checker
#ifndef ANI_XMLSAXPARSER_H
#define ANI_XMLSAXPARSER_H

#include <ani.h>
#include <libxml/parser.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <string>

#include "libani_helpers/concurrency_helpers.h"

namespace ark::ets::sdk::util {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class XmlSAXParserHelper {
public:
    static ani_long BindNative(ani_env *env, ani_class self);
    static ani_boolean ReleaseNative(ani_env *env, ani_class self, ani_long pointer);
    // CC-OFFNXT(readability-function-size_parameters)
    static ani_string ParseInner(ani_env *env, ani_class self, ani_long pointer, ani_object parser, ani_object handler,
                                 ani_string chunk, ani_boolean isFinal);

    explicit XmlSAXParserHelper(ani_env *env);
    ~XmlSAXParserHelper();
    // CC-OFFNXT(NPC_RULE_ID_BLANK_SPACE_INDENT) buggy code checker
    std::string Parse(ani_env *env, ani_object parser, ani_object handler, const std::string &chunk, bool isFinal);
    void Release();

private:
    enum class SaxEventType {
        START_DOCUMENT,
        END_DOCUMENT,
        START_ELEMENT,
        END_ELEMENT,
        CHARACTERS,
    };

    enum class ParseResultType {
        NONE,
        COMPLETE,
        ERROR,
    };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    struct ParseTask {
        std::string chunk;
        bool isFinal {false};
    };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    struct SaxEvent {
        SaxEventType type {SaxEventType::START_DOCUMENT};
        std::string name;
        std::string uri;
        std::string qname;
        std::string content;
        std::string error;
        std::map<std::string, std::string> attributes;
    };

    bool InitParserContext();
    void Cleanup();
    bool ExtractCallbacks(ani_env *env, ani_object parser, ani_object handler);
    bool StartWorkerIfNeeded(ani_env *env);
    void RequestShutdown();
    // CC-OFFNXT(NPC_RULE_ID_BLANK_SPACE_INDENT) buggy code checker
    void EnqueueParseTask(const std::string &chunk, bool isFinal);
    void ProcessParseTasks(ani_env *env);
    void ExecuteQueuedCallbacks();
    void QueueSaxEvent(ani_env *env, SaxEvent &&event);
    void FinalizeSession(ani_env *env);
    void ResetSession(ani_env *env);
    bool TryScheduleCallbackDispatch(ani_env *env);
    bool TryScheduleParseResultDispatch(ani_env *env);
    void QueueParseResult(ani_env *env, ParseResultType type, std::string error = {});
    // CC-OFFNXT(NPC_RULE_ID_BLANK_SPACE_INDENT) buggy code checker
    void SetFatalError(const std::string &error);
    std::string GetFatalError();
    void AcquireSelf();

    static void ParserWorkerExecute(ani_env *env, void *data);
    static void ParserWorkerComplete(ani_env *env, arkts::concurrency_helpers::WorkStatus status, void *data);
    static void DispatchQueuedCallbacks(void *data);
    static void DispatchParseResult(void *data);

    static void StartDocumentCallback(void *userData);
    static void EndDocumentCallback(void *userData);
    // CC-OFFNXT(readability-function-size_parameters)
    static void StartElementNsCallback(void *userData, const xmlChar *localname, const xmlChar *prefix,
                                       const xmlChar *uri, int namespaceSize, const xmlChar **namespaces,
                                       int attributeSize, int defaultedSize, const xmlChar **attributes);
    static void EndElementNsCallback(void *userData, const xmlChar *localname, const xmlChar *prefix,
                                     const xmlChar *uri);
    static void CharactersCallback(void *userData, const xmlChar *ch, int len);

    std::map<std::string, std::string> ConvertSAX2Attributes(const xmlChar **attrs, int attributeSize);
    ani_object CreateAttributesMap(ani_env *env, const std::map<std::string, std::string> &attrMap);
    // CC-OFFNXT(NPC_RULE_ID_BLANK_SPACE_INDENT) buggy code checker
    ani_string CreateStringValue(ani_env *env, const std::string &str);

    xmlParserCtxtPtr parserCtxt_ {nullptr};
    bool isInitialized_ {false};
    std::string error_ {};

    ani_method startDocumentRef_ {nullptr};
    ani_method endDocumentRef_ {nullptr};
    ani_method startElementRef_ {nullptr};
    ani_method endElementRef_ {nullptr};
    ani_method charactersRef_ {nullptr};
    ani_method completeRef_ {nullptr};
    ani_method asyncErrorRef_ {nullptr};
    ani_method mapConstructorRef_ {nullptr};
    ani_method mapSetRef_ {nullptr};
    ani_ref xmlsaxhandleRef_ {nullptr};
    ani_ref xmlsaxhandleCls_ {nullptr};
    ani_ref parserRef_ {nullptr};
    ani_ref parserCls_ {nullptr};
    ani_ref mapClsRef_ {nullptr};
    ani_vm *vm_ {nullptr};
    arkts::concurrency_helpers::AsyncWork *asyncWork_ {nullptr};
    std::atomic_uint32_t refCount_ {1};
    std::atomic_bool dispatchPending_ {false};
    std::atomic_bool parseResultDispatchPending_ {false};
    bool callbacksExtracted_ {false};
    bool workerStarted_ {false};
    bool shutdownRequested_ {false};
    ParseResultType parseResultType_ {ParseResultType::NONE};
    std::string parseResultError_ {};
    arkts::concurrency_helpers::AniWorkerId mainWorkerId_ {-1};
    std::mutex parseMutex_;
    std::condition_variable parseCv_;
    std::deque<ParseTask> parseTasks_;
    std::mutex callbackMutex_;
    std::deque<SaxEvent> callbackQueue_;
    std::mutex resultMutex_;
    std::mutex errorMutex_;
};

}  // namespace ark::ets::sdk::util

#endif  // ANI_XMLSAXPARSER_H
