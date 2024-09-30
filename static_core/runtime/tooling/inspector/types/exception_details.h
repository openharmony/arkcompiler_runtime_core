/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H
#define PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H

#include "macros.h"
#include "numeric_id.h"
#include "remote_object.h"

#include <functional>
#include <optional>
#include <string>

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class ExceptionDetails final {
public:
    ExceptionDetails(size_t exceptionId, std::string text, size_t lineNumber, size_t columnNumber)
        : exceptionId_(exceptionId), text_(std::move(text)), lineNumber_(lineNumber), columnNumber_(columnNumber)
    {
    }

    std::function<void(JsonObjectBuilder &)> ToJson() const;

    size_t GetExceptionId() const
    {
        return exceptionId_;
    }

    const std::string &GetText() const
    {
        return text_;
    }

    size_t GetLine() const
    {
        return lineNumber_;
    }

    size_t GetColumn() const
    {
        return columnNumber_;
    }

    ScriptId GetScriptId() const
    {
        return scriptId_.value_or(0);
    }

    ExceptionDetails &SetScriptId(ScriptId scriptId)
    {
        scriptId_ = scriptId;
        return *this;
    }

    bool HasScriptId() const
    {
        return scriptId_.has_value();
    }

    const std::string &GetUrl() const
    {
        ASSERT(HasUrl());
        return url_.value();
    }

    ExceptionDetails &SetUrl(std::string_view url)
    {
        url_ = url;
        return *this;
    }

    bool HasUrl() const
    {
        return url_.has_value();
    }

    const std::optional<RemoteObject> &GetExceptionObject() const
    {
        return exception_;
    }

    ExceptionDetails &SetExceptionObject(RemoteObject &&exception)
    {
        exception_ = std::move(exception);
        return *this;
    }

    bool HasExceptionObject() const
    {
        return exception_.has_value();
    }

    ExecutionContextId GetExecutionContextId() const
    {
        return executionContextId_.value_or(-1);
    }

    ExceptionDetails &SetExecutionContextId(ExecutionContextId executionContextId)
    {
        executionContextId_ = executionContextId;
        return *this;
    }

    bool HasExecutionContextId() const
    {
        return executionContextId_.has_value();
    }

private:
    size_t exceptionId_ {0};
    std::string text_;
    size_t lineNumber_ {0};
    size_t columnNumber_ {0};
    std::optional<ScriptId> scriptId_;
    std::optional<std::string> url_;
    std::optional<RemoteObject> exception_;
    std::optional<ExecutionContextId> executionContextId_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H
