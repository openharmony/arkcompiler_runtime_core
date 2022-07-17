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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_LOGGER_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_LOGGER_H

#include "utils/logger.h"

#include "gmock/gmock.h"

#include <sstream>
#include <string>
#include <string_view>

namespace panda::tooling::inspector::test {
class TestLogger : public Logger {
public:
    enum Flag : unsigned {
        FAIL_ON_ERROR = 0b001,
        OUTPUT_ON_FAIL = 0b010,
        OUTPUT_UNBUFFERED = 0b100,
    };

    explicit TestLogger(unsigned flags = 0);
    ~TestLogger() override;
    NO_COPY_SEMANTIC(TestLogger);
    NO_MOVE_SEMANTIC(TestLogger);

    void SetFlag(Flag flag, bool state)
    {
        flags_ = state ? flags_ | flag : flags_ & ~flag;
    }

    std::string_view SetPrefix(std::string_view prefix = {})
    {
        prefix_.swap(prefix);
        return prefix;
    }

    MOCK_METHOD(void, LogLineInternal, (Level, Component, const std::string &), (override));

private:
    std::string FormatLogLine(Level level, Component component, const std::string &message) const;
    void SyncOutputResource() override {}

    unsigned flags_;
    std::string_view prefix_;
    Logger *oldLogger_;
    std::stringstream log_;
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_LOGGER_H
