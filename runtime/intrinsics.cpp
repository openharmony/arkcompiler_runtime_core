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

#include <chrono>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include "libpandabase/macros.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/span.h"
#include "libpandabase/utils/time.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_status.h"
#include "runtime/interpreter/frame.h"
#include "utils/math_helpers.h"
#include "intrinsics.h"

namespace panda::intrinsics {

uint8_t IsInfF64(double v)
{
    return static_cast<uint8_t>(std::isinf(v));
}

uint8_t IsInfF32(float v)
{
    return static_cast<uint8_t>(std::isinf(v));
}

int32_t AbsI32(int32_t v)
{
    return std::abs(v);
}

int64_t AbsI64(int64_t v)
{
    return std::abs(v);
}

float AbsF32(float v)
{
    return std::abs(v);
}

double AbsF64(double v)
{
    return std::abs(v);
}

float SinF32(float v)
{
    return std::sin(v);
}

double SinF64(double v)
{
    return std::sin(v);
}

float CosF32(float v)
{
    return std::cos(v);
}

double CosF64(double v)
{
    return std::cos(v);
}

float PowF32(float base, float exp)
{
    return std::pow(base, exp);
}

double PowF64(double base, double exp)
{
    return std::pow(base, exp);
}

float SqrtF32(float v)
{
    return std::sqrt(v);
}

double SqrtF64(double v)
{
    return std::sqrt(v);
}

int32_t MinI32(int32_t a, int32_t b)
{
    return std::min(a, b);
}

int64_t MinI64(int64_t a, int64_t b)
{
    return std::min(a, b);
}

float MinF32(float a, float b)
{
    return panda::helpers::math::min(a, b);
}

double MinF64(double a, double b)
{
    return panda::helpers::math::min(a, b);
}

int32_t MaxI32(int32_t a, int32_t b)
{
    return std::max(a, b);
}

int64_t MaxI64(int64_t a, int64_t b)
{
    return std::max(a, b);
}

float MaxF32(float a, float b)
{
    return panda::helpers::math::max(a, b);
}

double MaxF64(double a, double b)
{
    return panda::helpers::math::max(a, b);
}

template <bool is_err, class T>
void Print(T v)
{
    if (is_err) {
        std::cerr << v;
    } else {
        std::cout << v;
    }
}

template <bool is_err, class T>
void PrintW(T v)
{
    if (is_err) {
        std::wcerr << v;
    } else {
        std::wcout << v;
    }
}

template <bool is_err>
void PrintStringInternal(coretypes::String *v)
{
    if (v->IsUtf16()) {
        Span<const char16_t> sp(reinterpret_cast<const char16_t *>(v->GetDataUtf16()), v->GetLength());
        for (wchar_t c : sp) {
            PrintW<is_err>(c);
        }
    } else {
        Span<const char> sp(reinterpret_cast<const char *>(v->GetDataMUtf8()), v->GetLength());
        for (char c : sp) {
            Print<is_err>(c);
        }
    }
}

void PrintString(coretypes::String *v)
{
    PrintStringInternal<false>(v);
}

void PrintF32(float v)
{
    Print<false>(v);
}

void PrintF64(double v)
{
    Print<false>(v);
}

void PrintI32(int32_t v)
{
    Print<false>(v);
}

void PrintU32(uint32_t v)
{
    Print<false>(v);
}

void PrintI64(int64_t v)
{
    Print<false>(v);
}

void PrintU64(uint64_t v)
{
    Print<false>(v);
}

int64_t NanoTime()
{
    return time::GetCurrentTimeInNanos();
}

void Assert(uint8_t cond)
{
    if (cond == 0) {
        Runtime::Abort();
    }
}

void UnknownIntrinsic()
{
    std::cerr << "UnknownIntrinsic\n";
    Runtime::Abort();
}

void AssertPrint(uint8_t cond, coretypes::String *s)
{
    if (cond == 0) {
        PrintStringInternal<true>(s);
        Runtime::Abort();
    }
}

#ifndef PANDA_PRODUCT_BUILD
uint8_t CompileMethod(coretypes::String *full_method_name)
{
    auto name = ConvertToString(full_method_name);
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();

    size_t pos = name.find_last_of("::");
    if (pos == std::string_view::npos) {
        return 1;
    }
    auto class_name = PandaString(name.substr(0, pos - 1));
    auto method_name = PandaString(name.substr(pos + 1));

    PandaString descriptor;
    auto class_name_bytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(class_name.c_str()), &descriptor);
    auto method_name_bytes = utf::CStringAsMutf8(method_name.c_str());

    ClassLinkerExtension *ext = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = class_linker->GetClass(class_name_bytes, true, ext->GetBootContext());
    if (cls == nullptr) {
        static constexpr uint8_t CLASS_IS_NULL = 2;
        return CLASS_IS_NULL;
    }

    auto method = cls->GetDirectMethod(method_name_bytes);
    if (method == nullptr) {
        static constexpr uint8_t METHOD_IS_NULL = 3;
        return METHOD_IS_NULL;
    }

    if (method->IsAbstract()) {
        static constexpr uint8_t ABSTRACT_ERROR = 4;
        return ABSTRACT_ERROR;
    }
    if (method->HasCompiledCode()) {
        return 0;
    }
    auto *compiler = Runtime::GetCurrent()->GetPandaVM()->GetCompiler();
    auto status = method->GetCompilationStatus();
    for (; (status != Method::COMPILED) && (status != Method::FAILED); status = method->GetCompilationStatus()) {
        if (status == Method::NOT_COMPILED) {
            ASSERT(!method->HasCompiledCode());
            compiler->CompileMethod(method, 0, false);
        }
        static constexpr uint64_t SLEEP_MS = 10;
        MTManagedThread::GetCurrent()->TimedWait(ThreadStatus::IS_COMPILER_WAITING, SLEEP_MS, 0);
    }
    static constexpr uint8_t COMPILATION_FAILED = 5;
    return (status == Method::COMPILED ? 0 : COMPILATION_FAILED);
}
#endif  // PANDA_PRODUCT_BUILD

// TODO(kbaladurin) : Convert methods should be implemented in managed library

int32_t ConvertStringToI32(coretypes::String *s)
{
    return static_cast<int32_t>(PandaStringToLL(ConvertToString(s)));
}

uint32_t ConvertStringToU32(coretypes::String *s)
{
    return static_cast<uint32_t>(PandaStringToULL(ConvertToString(s)));
}

int64_t ConvertStringToI64(coretypes::String *s)
{
    return static_cast<int64_t>(PandaStringToLL(ConvertToString(s)));
}

uint64_t ConvertStringToU64(coretypes::String *s)
{
    return static_cast<uint64_t>(PandaStringToULL(ConvertToString(s)));
}

float ConvertStringToF32(coretypes::String *s)
{
    return PandaStringToF(ConvertToString(s));
}

double ConvertStringToF64(coretypes::String *s)
{
    return PandaStringToD(ConvertToString(s));
}

void SystemExit(int32_t status)
{
    Runtime::Halt(status);
}

ObjectHeader *ObjectCreateNonMovable(coretypes::Class *cls)
{
    ASSERT(cls != nullptr);
    return ObjectHeader::CreateNonMovable(cls->GetRuntimeClass());
}

void ObjectMonitorEnter(ObjectHeader *header)
{
    if (header == nullptr) {
        panda::ThrowNullPointerException();
        return;
    }
    auto res = Monitor::MonitorEnter(header);
    // Expected results: OK, ILLEGAL
    ASSERT(res != Monitor::State::INTERRUPTED);
    if (UNLIKELY(res != Monitor::State::OK)) {
        // This should never happen
        LOG(FATAL, RUNTIME) << "MonitorEnter for " << std::hex << header << " returned Illegal state!";
    }
}

void ObjectMonitorExit(ObjectHeader *header)
{
    if (header == nullptr) {
        panda::ThrowNullPointerException();
        return;
    }
    auto res = Monitor::MonitorExit(header);
    // Expected results: OK, ILLEGAL
    ASSERT(res != Monitor::State::INTERRUPTED);
    if (res == Monitor::State::ILLEGAL) {
        PandaStringStream ss;
        ss << "MonitorExit for object " << std::hex << header << " returned Illegal state";
        panda::ThrowIllegalMonitorStateException(ss.str());
    }
}

void ObjectWait(ObjectHeader *header)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_WAITING, 0, 0);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectTimedWait(ObjectHeader *header, uint64_t timeout)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_TIMED_WAITING, timeout, 0);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectTimedWaitNanos(ObjectHeader *header, uint64_t timeout, uint64_t nanos)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_TIMED_WAITING, timeout, nanos);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectNotify(ObjectHeader *header)
{
    Monitor::State state = Monitor::Notify(header);
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::Notify() failed";
}

void ObjectNotifyAll(ObjectHeader *header)
{
    Monitor::State state = Monitor::NotifyAll(header);
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::NotifyAll() failed";
}

}  // namespace panda::intrinsics

#include <intrinsics_gen.h>
