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

#ifndef PANDA_RUNTIME_DPROFILER_TRACE_H_
#define PANDA_RUNTIME_DPROFILER_TRACE_H_

#include <memory>
#include <string>
#include <unordered_set>
#include "macros.h"
#include "include/mem/panda_containers.h"
#include "include/mem/panda_string.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/mem/panda_containers.h"
#include "include/runtime.h"
#include "include/runtime_notification.h"
#include "runtime/include/method.h"
#include "runtime/include/language_context.h"
#include "os/mutex.h"

namespace panda {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex trace_lock;
enum EventFlag {
    TRACE_METHOD_ENTER = 0x00,
    TRACE_METHOD_EXIT = 0x01,
    TRACE_METHOD_UNWIND = 0x02,
};
class Trace : public RuntimeListener {
public:
    static constexpr size_t ENCODE_EVENT_BITS = 2;
    static void WriteDataByte(uint8_t *data, uint64_t value, uint8_t size)
    {
        for (uint8_t i = 0; i < size; i++) {
            *data = static_cast<uint8_t>(value);
            value = value >> WRITE_LENGTH;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            data++;
        }
    }

    static uint64_t GetDataFromBuffer(const uint8_t *buffer, size_t num)
    {
        uint64_t data = 0;
        for (size_t i = 0; i < num; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
            data |= static_cast<uint64_t>(buffer[i]) << (i * 8U);
        }
        return data;
    }

    static void StartTracing(const char *trace_filename, size_t buffer_size);

    static void TriggerTracing();

    void MethodEntry(ManagedThread *thread, Method *method) override;
    void MethodExit(ManagedThread *thread, Method *method) override;

    void SaveTracingData();

    static void StopTracing();

    static bool is_tracing;

    ~Trace() override;

    uint64_t GetAverageTime();

protected:
    explicit Trace(PandaUniquePtr<panda::os::unix::file::File> trace_file, size_t buffer_size);
    uint32_t EncodeMethodToId(Method *method);
    virtual PandaString GetThreadName(ManagedThread *thread) = 0;
    virtual PandaString GetMethodDetailInfo(Method *method) = 0;

private:
    static constexpr size_t TRACE_HEADER_REAL_LENGTH = 18U;

    static const char TRACE_STAR_CHAR = '*';
    static const uint16_t TRACE_HEADER_LENGTH = 32;
    static const uint32_t MAGIC_VALUE = 0x574f4c53;
    static const uint16_t TRACE_VERSION = 3;
    static const uint16_t TRACE_ITEM_SIZE = 14;

    static const uint32_t FILE_SIZE = 8 * 1024 * 1024;

    // used to define the number we need to right shift
    static const uint8_t WRITE_LENGTH = 8;

    // used to define the number of this item we have writed  just now
    const uint8_t NUMBER_OF_2BYTES = 2;
    const uint8_t NUMBER_OF_4BYTES = 4;
    const uint8_t NUMBER_OF_8BYTES = 8;

    const int32_t LOOP_NUMBER = 10000;
    const int32_t DIVIDE_NUMBER = 10;

    PandaUniquePtr<RuntimeListener> listener_;

    os::memory::Mutex methods_lock_;
    // all methods are encoded to id, and put method、id into this map
    PandaMap<Method *, uint32_t> method_id_pandamap_ GUARDED_BY(methods_lock_);
    PandaVector<Method *> methods_called_vector_ GUARDED_BY(methods_lock_);

    os::memory::Mutex thread_info_lock_;
    PandaSet<PandaString> thread_info_set_ GUARDED_BY(thread_info_lock_);

    uint32_t EncodeMethodAndEventToId(Method *method, EventFlag flag);
    Method *DecodeIdToMethod(uint32_t id);

    void GetCalledMethods(size_t end_offset, PandaSet<Method *> *called_methods);

    void GetTimes(uint32_t *thread_time, uint32_t *real_time);

    void WriteInfoToBuf(const ManagedThread *thread, Method *method, EventFlag event, uint32_t thread_time,
                        uint32_t real_time);

    void RecordThreadsInfo(PandaOStringStream *os);
    void RecordMethodsInfo(PandaOStringStream *os, const PandaSet<Method *> &called_methods);

    void GetUniqueMethods(size_t buffer_length, PandaSet<Method *> *called_methods_set);

    static Trace *volatile singleton_trace GUARDED_BY(trace_lock);

    PandaUniquePtr<panda::os::unix::file::File> trace_file_;
    const size_t BUFFER_SIZE;

    PandaUniquePtr<uint8_t[]> buffer_;  // NOLINT(modernize-avoid-c-arrays)

    const uint64_t TRACE_START_TIME;

    const uint64_t BASE_CPU_TIME;

    std::atomic<int32_t> buffer_offset_;

    bool overbrim_;

    static LanguageContext ctx;

    NO_COPY_SEMANTIC(Trace);
    NO_MOVE_SEMANTIC(Trace);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_DPROFILER_TRACE_H_
