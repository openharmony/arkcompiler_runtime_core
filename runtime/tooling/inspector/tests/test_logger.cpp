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

#include "test_logger.h"

#include "macros.h"
#include "utils/logger.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using testing::AnyNumber;
using testing::UnitTest;

namespace panda::tooling::inspector::test {
TestLogger::TestLogger(unsigned flags) : Logger(Level::DEBUG, ComponentMask().set(Component::DEBUGGER)), flags_(flags)
{
    if ((flags_ & OUTPUT_UNBUFFERED) != 0) {
        flags_ &= ~OUTPUT_ON_FAIL;
    }

    oldLogger_ = Logger::logger;
    Logger::logger = this;

    EXPECT_CALL(*this, LogLineInternal).Times(AnyNumber());

    ON_CALL(*this, LogLineInternal).WillByDefault([this](auto level, auto component, auto &message) {
        if ((flags_ & OUTPUT_UNBUFFERED) != 0) {
            std::cout << "   " << FormatLogLine(level, component, message) << '\n' << std::flush;
        }

        if ((flags_ & OUTPUT_ON_FAIL) != 0) {
            log_ << "          " << FormatLogLine(level, component, message) << '\n';
        }

        if ((flags_ & FAIL_ON_ERROR) != 0) {
            ASSERT_NE(level, Level::ERROR) << "Log line: " << FormatLogLine(level, component, message);
        }
    });
}

TestLogger::~TestLogger()
{
    Logger::logger = oldLogger_;

    if ((flags_ & OUTPUT_ON_FAIL) != 0) {
        if (auto *testInfo = UnitTest::GetInstance()->current_test_info()) {
            if (testInfo->result()->Failed()) {
                std::cout << "Full log:\n" << log_.rdbuf() << std::flush;
            }
        }
    }
}

std::string TestLogger::FormatLogLine(Level level, Component /* component  */, const std::string &message) const
{
    std::stringstream logLine;

    switch (level) {
        case Logger::Level::FATAL:
            logLine << "F";
            break;
        case Logger::Level::ERROR:
            logLine << "E";
            break;
        case Logger::Level::WARNING:
            logLine << "W";
            break;
        case Logger::Level::INFO:
            logLine << "I";
            break;
        case Logger::Level::DEBUG:
            logLine << "D";
            break;
        default:
            UNREACHABLE();
    }

    if (!prefix_.empty()) {
        logLine << "[" << prefix_ << "]";
    }

    logLine << ": " << message;
    return logLine.str();
}
}  // namespace panda::tooling::inspector::test
