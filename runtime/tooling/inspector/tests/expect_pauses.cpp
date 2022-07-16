/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "expect_pauses.h"
#include "client.h"
#include "json_object_matcher.h"
#include "test_method.h"

#include "macros.h"
#include "method.h"
#include "utils/json_parser.h"
#include "utils/utf.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

using testing::_;
using testing::Matcher;

namespace panda::tooling::inspector::test {
Pause::CallFrame CallFrame(Method *method, size_t lineNumber)
{
    return {utf::Mutf8AsCString(method->GetName().data), lineNumber};
}

Pause::CallFrame CallFrame(const TestMethod &method, size_t lineNumber)
{
    return {utf::Mutf8AsCString(method.Get()->GetName().data), lineNumber};
}

namespace {
class OnPause {
public:
    // google-explicit-constructor conflicts with CodeCheck w.r.t. initializer_list constructors.
    explicit OnPause(std::initializer_list<Pause> pauses)  // NOLINT(google-explicit-constructor)
        : pauses_(new std::vector<Pause>(std::rbegin(pauses), std::rend(pauses)))
    {
    }

    ~OnPause()
    {
        if (pauses_.use_count() == 1) {
            EXPECT_TRUE(pauses_->empty());
        }
    }

    DEFAULT_COPY_SEMANTIC(OnPause);
    DEFAULT_MOVE_SEMANTIC(OnPause);

    void operator()(const JsonObject &result);

private:
    std::shared_ptr<std::vector<Pause>> pauses_;
};
}  // namespace

void OnPause::operator()(const JsonObject &result)
{
    ASSERT_FALSE(pauses_->empty());

    std::vector<Matcher<JsonObject::JsonObjPointer>> callFrameMatchers;

    std::transform(
        pauses_->back().callFrames.begin(), pauses_->back().callFrames.end(), std::back_inserter(callFrameMatchers),
        [](auto &callFrame) {
            return Pointee(JsonProperties(
                JsonProperty<JsonObject::StringT> {"callFrameId", _},
                JsonProperty<JsonObject::StringT> {"functionName", callFrame.functionName},
                JsonProperty<JsonObject::JsonObjPointer> {
                    "location",
                    Pointee(JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", _},
                                           JsonProperty<JsonObject::NumT> {"lineNumber", callFrame.lineNumber}))},
                JsonProperty<JsonObject::StringT> {"url", _}, JsonProperty<JsonObject::ArrayT> {"scopeChain", _},
                JsonProperty<JsonObject::JsonObjPointer> {
                    "this", Pointee(JsonProperties(JsonProperty<JsonObject::StringT> {"type", "undefined"}))}));
        });

    auto resultMatcher =
        JsonProperties(JsonProperty<JsonObject::StringT> {"reason", pauses_->back().reason},
                       JsonProperty<JsonObject::ArrayT> {"callFrames", JsonElementsAreArray(callFrameMatchers)});

    EXPECT_THAT(result, resultMatcher);

    pauses_->pop_back();
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
}

void ExpectPauses(Client &client, std::initializer_list<Pause> pauses)
{
    client.OnCall("Debugger.paused", OnPause(pauses));
}
}  // namespace panda::tooling::inspector::test
