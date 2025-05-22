/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>
#include <string>
#include <algorithm>
#include "macros.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"
#include "libpandabase/utils/utf.h"
#include "regex"
#include "intrinsics.h"
#include "utility"
#include "utils/logger.h"

namespace ark::ets::intrinsics {

namespace {
constexpr const char *NAN_LITERAL = "NaN";
constexpr const char *INF_LITERAL = "Infinity";
constexpr const char *NEGINF_LITERAL = "-Infinity";
}  // namespace

static std::string ReplaceSegments(std::string templateString, const std::vector<std::pair<int, int>> &matches,
                                   const std::vector<std::string> &args)
{
    // iter over matches from format_string
    // and forall i-th match we trying to get i-th arg
    // if |args| < |matches| simply doesn't format remainder
    // if |args| > |matches| => append as is
    for (int i = std::min(matches.size(), args.size()) - 1; i >= 0; --i) {
        const auto [start, end] = matches[i];
        templateString.replace(start, end - start, args[i]);
    }
    if (args.size() > matches.size()) {
        const auto accumulated = std::accumulate(args.begin() + matches.size(), args.end(), std::string {},
                                                 [](const auto &acc, const auto &x) { return acc + x; });
        templateString += ' ' + accumulated;
    }
    return templateString;
}

static std::string FormatString(std::string templateString, const std::vector<std::string> &args)
{
    if (args.empty()) {
        return templateString;
    }

    // Find all printf-style format specifiers
    const std::regex pattern(R"(%[+#\- 0*]*(?:\d+|\*)?(?:\.\d+|\.\*)?[hlL]?[diuoxXfFeEgGaAcsn])");
    std::vector<std::pair<int, int>> matches;
    std::smatch match;
    auto searchStart = templateString.cbegin();

    // This code searches through a text template to find all format specifiers (special markers that
    // show where values should be inserted). Format specifiers start with % and include letters like
    // 'd' for numbers, 'f' for decimals, or 's' for text strings.
    //
    // NOTE (templin.konstantin): This is a simplified formatting implementation
    // that differs from "printf"-style formatting:
    // - Instead of proper type-aware formatting, it just converts arguments to strings
    // - Format specifiers (%d, %f, etc.) are treated the same way - they're just placeholders
    // - All values are simply stringified and inserted, ignoring the actual format type
    //
    // On each iteration we try to get smth like %-modifier using regexp
    // Simple example: "Hello, %s; Number: %d"
    // 1st-iteration: match %s at at(7, 9)
    //    span is unique in matches ==> push_back it
    // 2-st: match %d with span (19, 21) ==> push_back it
    // We have next matches: (7, 9) and (19, 21)
    //
    // The search continues until we've looked through the entire template text, building up
    // a complete list of where all the format markers are located.
    //
    // Supported format specifiers include:
    // Integers: %d (decimal), %u (unsigned), %o (octal), %x/%X (hex)
    // Floating-point: %f/%F (decimal), %e/%E (scientific), %g/%G (adaptive), %a/%A (hex float)
    // Other: %c (char), %s (string), %p (pointer), %n (chars written)
    while (std::regex_search(searchStart, templateString.cend(), match, pattern)) {
        auto start = std::distance(templateString.cbegin(), match.prefix().second);
        auto end = std::distance(templateString.cbegin(), match.suffix().first);
        auto span = std::make_pair(static_cast<int>(start), static_cast<int>(end));
        if (std::count(matches.begin(), matches.end(), span) == 0) {
            matches.push_back(span);
        }
        searchStart = match.suffix().first;
    }

    return ReplaceSegments(std::move(templateString), matches, args);
}

// Console output types
enum class ConsoleStream : uint8_t { STDOUT = 0, STDERR = 1 };
enum class ConsoleLevel : uint8_t { DEBUG = 0, INFO = 1, LOG = 2, WARN = 3, ERROR = 4 };

struct ConsoleBuffer {
    std::stringstream stream;
    std::vector<std::string> args;
};
static thread_local ConsoleBuffer g_consoleBuf;
static thread_local ConsoleBuffer g_consoleErrBuf;

static ConsoleStream ConsoleStreamFromLevel(ConsoleLevel level)
{
    return (level <= ConsoleLevel::LOG) ? ConsoleStream::STDOUT : ConsoleStream::STDERR;
}

static std::pair<std::stringstream &, std::vector<std::string> &> GetBuffer(ConsoleLevel level)
{
    return (ConsoleStreamFromLevel(level) == ConsoleStream::STDOUT)
               ? std::pair<std::stringstream &, std::vector<std::string> &>(g_consoleBuf.stream, g_consoleBuf.args)
               : std::pair<std::stringstream &, std::vector<std::string> &>(g_consoleErrBuf.stream,
                                                                            g_consoleErrBuf.args);
}
static ark::Logger::PandaLog2MobileLog ConsoleLevelToMobileLog(ConsoleLevel level)
{
    switch (level) {
        case ConsoleLevel::DEBUG:
            return ark::Logger::PandaLog2MobileLog::DEBUG;
        case ConsoleLevel::INFO:
            return ark::Logger::PandaLog2MobileLog::INFO;
        case ConsoleLevel::LOG:
            return ark::Logger::PandaLog2MobileLog::INFO;
        case ConsoleLevel::WARN:
            return ark::Logger::PandaLog2MobileLog::WARN;
        case ConsoleLevel::ERROR:
            return ark::Logger::PandaLog2MobileLog::ERROR;
    }
    UNREACHABLE();
}
static void ConsoleCommit(ConsoleLevel level)
{
    auto [buf, args] = GetBuffer(level);
    const auto &formatted = FormatString(buf.str(), args);

    if (g_mlogBufPrint != nullptr) {
        g_mlogBufPrint(LOG_ID_MAIN, ConsoleLevelToMobileLog(level), "arkts.console", "%s", formatted.c_str());
    } else {
        (ConsoleStreamFromLevel(level) == ConsoleStream::STDERR ? std::cerr : std::cout) << formatted << std::endl;
    }

    buf.str("");
    args.clear();
}

/*
 * @brief add content to buffer/args
 * - Firstly, we check
 *   - If
 *   - buffer not empty
 *   - or buffer doesn't contains %-modifiers
 *   - count of %-modifiers less than args count (exclude special case with '%%')
 *      ==> write into buffer
 *   - Otherwise:
 *      ==> Write into args skipping " "
 */
template <typename T>
static void ConsoleAddToBuffer(ConsoleLevel level, const T &content)
{
    auto [buf, args] = GetBuffer(level);
    // count actual format specifiers (excluding %%)
    auto countFormatSpecifiers = [](const std::string &str) {
        std::string::size_type pos = 0;
        size_t count = 0;
        while ((pos = str.find('%', pos)) != std::string::npos) {
            // Skip %% (escaped percent)
            if (pos + 1 < str.length() && str[pos + 1] == '%') {
                pos += 2;
                continue;
            }
            count++;
            pos++;
        }
        return count;
    };
    if (buf.str().empty() || buf.str().find('%') == std::string::npos ||
        countFormatSpecifiers(buf.str()) < args.size()) {
        buf << content;
    } else {
        std::stringstream temp;
        temp << content;
        if (temp.str() != " ") {
            args.push_back(temp.str());
        }
    }
}

template <typename T>
struct ConsolePrinter {
    // Default implementation for integral types (except uint8_t which is used for bool)
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, T v)
    {
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, uint8_t>) {
            ConsoleAddToBuffer<T>(level, v);
        }
    }
};

struct ConsolePrintln {
    static void Print(ObjectHeader *header [[maybe_unused]])
    {
        ConsoleCommit(ConsoleLevel::INFO);
    }
    static void PrintError(ObjectHeader *header [[maybe_unused]])
    {
        ConsoleCommit(ConsoleLevel::ERROR);
    }
};
template <>
struct ConsolePrinter<uint8_t> {
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, uint8_t v)
    {
        ConsoleAddToBuffer(level, v == 0 ? "false" : "true");
    }
};
template <>
struct ConsolePrinter<uint16_t> {
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, uint16_t c)
    {
        const utf::Utf8Char utf8Ch = utf::ConvertUtf16ToUtf8(c, 0, false);
        ConsoleAddToBuffer(level, std::string_view(reinterpret_cast<const char *>(utf8Ch.ch.data()), utf8Ch.n));
    }
};
template <>
struct ConsolePrinter<int8_t> {
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, int8_t v)
    {
        ConsoleAddToBuffer<int32_t>(level, static_cast<int32_t>(v));
    }
};
template <>
struct ConsolePrinter<EtsString *> {
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, EtsString *str)
    {
        auto s = str->GetCoreType();
        if (s->IsUtf16()) {
            uint16_t *vdataPtr = s->GetDataUtf16();
            uint32_t vlength = s->GetLength();
            size_t mutf8Len = utf::Utf16ToMUtf8Size(vdataPtr, vlength);

            PandaVector<uint8_t> out(mutf8Len);
            utf::ConvertRegionUtf16ToMUtf8(vdataPtr, out.data(), vlength, mutf8Len, 0);

            ConsoleAddToBuffer(level, reinterpret_cast<const char *>(out.data()));
        } else {
            ConsoleAddToBuffer(level,
                               std::string_view(reinterpret_cast<const char *>(s->GetDataMUtf8()), s->GetLength()));
        }
    }
};
template <typename T>
struct FloatingPointPrinter {
    static void Print(ObjectHeader *header [[maybe_unused]], ConsoleLevel level, T v)
    {
        auto coroutine = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope scope(coroutine);

        if (std::isnan(v)) {
            StdConsolePrintString(header,
                                  EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NAN_LITERAL)).GetPtr());
        } else if (!std::isfinite(v)) {
            if (v < 0) {
                StdConsolePrintString(
                    header, EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NEGINF_LITERAL)).GetPtr());
            } else {
                StdConsolePrintString(
                    header, EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(INF_LITERAL)).GetPtr());
            }
        } else {
            ConsoleAddToBuffer<T>(level, v);
        }
    }
};
template <>
struct ConsolePrinter<float> : FloatingPointPrinter<float> {
};
template <>
struct ConsolePrinter<double> : FloatingPointPrinter<double> {
};

extern "C" {
void StdConsolePrintln(ObjectHeader *header [[maybe_unused]])
{
    ConsolePrintln::Print(header);
}

void StdConsolePrintlnError(ObjectHeader *header [[maybe_unused]])
{
    ConsolePrintln::PrintError(header);
}

void StdConsolePrintBool(ObjectHeader *header [[maybe_unused]], uint8_t v)
{
    ConsolePrinter<uint8_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintBoolError(ObjectHeader *header [[maybe_unused]], uint8_t v)
{
    ConsolePrinter<uint8_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintChar(ObjectHeader *header [[maybe_unused]], uint16_t v)
{
    ConsolePrinter<uint16_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintCharError(ObjectHeader *header [[maybe_unused]], uint16_t v)
{
    ConsolePrinter<uint16_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintString(ObjectHeader *header [[maybe_unused]], EtsString *v)
{
    ConsolePrinter<EtsString *>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintStringError(ObjectHeader *header [[maybe_unused]], EtsString *v)
{
    ConsolePrinter<EtsString *>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintI32(ObjectHeader *header [[maybe_unused]], int32_t v)
{
    ConsolePrinter<int32_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintI32Error(ObjectHeader *header [[maybe_unused]], int32_t v)
{
    ConsolePrinter<int32_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintI16(ObjectHeader *header [[maybe_unused]], int16_t v)
{
    ConsolePrinter<int16_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintI16Error(ObjectHeader *header [[maybe_unused]], int16_t v)
{
    ConsolePrinter<int16_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintI8(ObjectHeader *header [[maybe_unused]], int8_t v)
{
    ConsolePrinter<int8_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintI8Error(ObjectHeader *header [[maybe_unused]], int8_t v)
{
    ConsolePrinter<int8_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintI64(ObjectHeader *header [[maybe_unused]], int64_t v)
{
    ConsolePrinter<int64_t>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintI64Error(ObjectHeader *header [[maybe_unused]], int64_t v)
{
    ConsolePrinter<int64_t>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintF32(ObjectHeader *header [[maybe_unused]], float v)
{
    ConsolePrinter<float>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintF32Error(ObjectHeader *header [[maybe_unused]], float v)
{
    ConsolePrinter<float>::Print(header, ConsoleLevel::ERROR, v);
}

void StdConsolePrintF64(ObjectHeader *header [[maybe_unused]], double v)
{
    ConsolePrinter<double>::Print(header, ConsoleLevel::INFO, v);
}

void StdConsolePrintF64Error(ObjectHeader *header [[maybe_unused]], double v)
{
    ConsolePrinter<double>::Print(header, ConsoleLevel::ERROR, v);
}
}

}  // namespace ark::ets::intrinsics
