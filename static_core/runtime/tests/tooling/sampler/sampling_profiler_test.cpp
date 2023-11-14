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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "libpandafile/file.h"
#include "libpandabase/trace/trace.h"
#include "libpandabase/panda_gen_options/generated/base_options.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "runtime/interpreter/runtime_interface.h"
#include "tools/sampler/aspt_converter.h"

namespace panda::tooling::sampler::test {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

static const char *PROFILER_FILENAME = "profiler_result.aspt";
static const char *PANDA_FILE_NAME = "sampling_profiler_test_ark_asm.abc";
static constexpr size_t TEST_CYCLE_THRESHOLD = 100;

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class SamplerTest : public testing::Test {
public:
    // NOLINTNEXTLINE(readability-function-size)
    void SetUp() override
    {
        Logger::Initialize(base_options::Options(""));

        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetInterpreterType("cpp");
        auto exec_path = panda::os::file::File::GetExecutablePath();
        std::string panda_std_lib =
            exec_path.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options.SetBootPandaFiles({panda_std_lib});
        Runtime::Create(options);

        auto pf = panda_file::OpenPandaFileOrZip(PANDA_FILE_NAME);
        Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

        thread_ = panda::MTManagedThread::GetCurrent();
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    void FullfillFakeSample(SampleInfo *ps)
    {
        for (uint32_t i = 0; i < SampleInfo::StackInfo::MAX_STACK_DEPTH; ++i) {
            ps->stack_info.managed_stack[i] = {i, pf_id_};
        }
        ps->thread_info.thread_id = GetThreadId();
        ps->stack_info.managed_stack_size = SampleInfo::StackInfo::MAX_STACK_DEPTH;
    }

    // Friend wrappers for accesing samplers private fields
    static os::thread::NativeHandleType ExtractListenerTid(const Sampler *s_ptr)
    {
        return s_ptr->listener_tid_;
    }

    static os::thread::NativeHandleType ExtractSamplerTid(const Sampler *s_ptr)
    {
        return s_ptr->sampler_tid_;
    }

    static PandaSet<os::thread::ThreadId> ExtractManagedThreads(Sampler *s_ptr)
    {
        // Sending a copy to avoid of datarace
        os::memory::LockHolder holder(s_ptr->managed_threads_lock_);
        PandaSet<os::thread::ThreadId> managed_threads_copy = s_ptr->managed_threads_;
        return managed_threads_copy;
    }

    static size_t ExtractLoadedPFSize(Sampler *s_ptr)
    {
        os::memory::LockHolder holder(s_ptr->loaded_pfs_lock_);
        return s_ptr->loaded_pfs_.size();
    }

    static std::array<int, 2> ExtractPipes(const Sampler *s_ptr)
    {
        // Sending a copy to avoid of datarace
        return s_ptr->communicator_.listener_pipe_;
    }

    static bool ExtractIsActive(const Sampler *s_ptr)
    {
        return s_ptr->is_active_;
    }

    uint32_t GetThreadId()
    {
        return os::thread::GetCurrentThreadId();
    }

protected:
    panda::MTManagedThread *thread_ {nullptr};
    uintptr_t pf_id_ {0};
    uint32_t checksum_ {0};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

TEST_F(SamplerTest, SamplerInitTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), false);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    // Second run
    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

void RunManagedThread(std::atomic<bool> *sync_flag)
{
    auto *m_thr =
        panda::MTManagedThread::Create(panda::Runtime::GetCurrent(), panda::Runtime::GetCurrent()->GetPandaVM());
    m_thr->ManagedCodeBegin();

    *sync_flag = true;
    while (*sync_flag) {
        // Calling safepoint 'cause starting profiler required to stop all managed threads
        interpreter::RuntimeInterface::Safepoint();
    }

    m_thr->ManagedCodeEnd();
    m_thr->Destroy();
}

void RunManagedThreadAndSaveThreadId(std::atomic<bool> *sync_flag, os::thread::ThreadId *id)
{
    auto *m_thr =
        panda::MTManagedThread::Create(panda::Runtime::GetCurrent(), panda::Runtime::GetCurrent()->GetPandaVM());
    m_thr->ManagedCodeBegin();

    *id = os::thread::GetCurrentThreadId();
    *sync_flag = true;
    while (*sync_flag) {
        // Calling safepoint 'cause starting profiler required to stop all managed threads
        interpreter::RuntimeInterface::Safepoint();
    }

    m_thr->ManagedCodeEnd();
    m_thr->Destroy();
}

void RunNativeThread(std::atomic<bool> *sync_flag)
{
    auto *m_thr =
        panda::MTManagedThread::Create(panda::Runtime::GetCurrent(), panda::Runtime::GetCurrent()->GetPandaVM());

    *sync_flag = true;
    while (*sync_flag) {
    }

    m_thr->Destroy();
}

// Testing notification thread started/finished
TEST_F(SamplerTest, SamplerEventThreadNotificationTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_FALSE(ExtractManagedThreads(sp).empty());
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> sync_flag1 = false;
    std::atomic<bool> sync_flag2 = false;
    std::atomic<bool> sync_flag3 = false;
    std::thread managed_thread1(RunManagedThread, &sync_flag1);
    std::thread managed_thread2(RunManagedThread, &sync_flag2);
    std::thread managed_thread3(RunManagedThread, &sync_flag3);

    while (!sync_flag1 || !sync_flag2 || !sync_flag3) {
        ;
    }
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4);

    sync_flag1 = false;
    sync_flag2 = false;
    sync_flag3 = false;
    managed_thread1.join();
    managed_thread2.join();
    managed_thread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing notification thread started/finished
TEST_F(SamplerTest, SamplerCheckThreadIdTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> sync_flag1 = false;
    os::thread::ThreadId mt_id = 0;
    std::thread managed_thread1(RunManagedThreadAndSaveThreadId, &sync_flag1, &mt_id);

    while (!sync_flag1) {
        ;
    }
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 2);
    bool is_passed = false;

    for (const auto &elem : ExtractManagedThreads(sp)) {
        if (elem == mt_id) {
            is_passed = true;
            break;
        }
    }
    ASSERT_TRUE(is_passed);

    sync_flag1 = false;
    managed_thread1.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing thread collection
TEST_F(SamplerTest, SamplerCollectThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> sync_flag1 = false;
    std::atomic<bool> sync_flag2 = false;
    std::atomic<bool> sync_flag3 = false;
    std::thread managed_thread1(RunManagedThread, &sync_flag1);
    std::thread managed_thread2(RunManagedThread, &sync_flag2);
    std::thread managed_thread3(RunManagedThread, &sync_flag3);

    while (!sync_flag1 || !sync_flag2 || !sync_flag3) {
        ;
    }

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4);

    sync_flag1 = false;
    sync_flag2 = false;
    sync_flag3 = false;
    managed_thread1.join();
    managed_thread2.join();
    managed_thread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing native thread collection
TEST_F(SamplerTest, SamplerCollectNativeThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> sync_flag1 = false;
    std::atomic<bool> sync_flag2 = false;
    std::atomic<bool> sync_flag3 = false;
    std::thread managed_thread1(RunManagedThread, &sync_flag1);
    std::thread native_thread2(RunNativeThread, &sync_flag2);

    while (!sync_flag1 || !sync_flag2) {
        ;
    }

    ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 3);
    std::thread native_thread3(RunNativeThread, &sync_flag3);
    while (!sync_flag3) {
        ;
    }

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4);

    sync_flag1 = false;
    sync_flag2 = false;
    sync_flag3 = false;
    managed_thread1.join();
    native_thread2.join();
    native_thread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing pipes
TEST_F(SamplerTest, SamplerPipesTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    sp->Start(PROFILER_FILENAME);

    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_READ_ID], 0);
    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_WRITE_ID], 0);

    sp->Stop();
    Sampler::Destroy(sp);
}

// Stress testing restart
TEST_F(SamplerTest, ProfilerRestartStressTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD / 10;
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    for (uint32_t i = 0; i < CURRENT_TEST_THRESHOLD; i++) {
        ASSERT_EQ(sp->Start(PROFILER_FILENAME), true);
        sp->Stop();
    }

    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, ThreadCommunicatorTest)
{
    ThreadCommunicator communicator;

    SampleInfo sample_input;
    SampleInfo sample_output;
    FullfillFakeSample(&sample_input);
    ASSERT_TRUE(communicator.Init());
    ASSERT_TRUE(communicator.SendSample(sample_input));
    ASSERT_TRUE(communicator.ReadSample(&sample_output));
    ASSERT_EQ(sample_output, sample_input);
}

static void CommunicatorStressWritterThread(const ThreadCommunicator *com, const SampleInfo &sample,
                                            uint32_t messages_amount)
{
    for (uint32_t i = 0; i < messages_amount; ++i) {
        // If the sample write failed we retrying to send it
        if (!com->SendSample(sample)) {
            std::cerr << "Failed to send a sample" << std::endl;
            abort();
        }
    }
}

TEST_F(SamplerTest, ThreadCommunicatorMultithreadTest)
{
    constexpr uint32_t MESSAGES_AMOUNT = TEST_CYCLE_THRESHOLD * 100;

    ThreadCommunicator communicator;
    SampleInfo sample_output;
    SampleInfo sample_input;
    FullfillFakeSample(&sample_input);
    ASSERT_TRUE(communicator.Init());

    std::thread sender(CommunicatorStressWritterThread, &communicator, sample_input, MESSAGES_AMOUNT);
    for (uint32_t i = 0; i < MESSAGES_AMOUNT; ++i) {
        // If the sample write failed we retrying to send it
        if (!communicator.ReadSample(&sample_output)) {
            std::cerr << "Failed to read a sample" << std::endl;
            abort();
        }
        ASSERT_EQ(sample_output, sample_input);
    }
    sender.join();
}

// Testing reader and writer by writing and reading from .aspt one sample
TEST_F(SamplerTest, StreamWriterReaderTest)
{
    const char *stream_test_filename = "stream_writer_reader_test.aspt";
    SampleInfo sample_output;
    SampleInfo sample_input;

    {
        StreamWriter writer(stream_test_filename);
        FullfillFakeSample(&sample_input);

        writer.WriteSample(sample_input);
    }

    SampleReader reader(stream_test_filename);
    ASSERT_TRUE(reader.GetNextSample(&sample_output));
    ASSERT_EQ(sample_output, sample_input);
    ASSERT_FALSE(reader.GetNextSample(&sample_output));
    ASSERT_FALSE(reader.GetNextModule(nullptr));
}

// Testing reader and writer by writing and reading from .aspt lots of samples
TEST_F(SamplerTest, StreamWriterReaderLotsSamplesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *stream_test_filename = "stream_writer_reader_test_lots_samples.aspt";
    SampleInfo sample_output;
    SampleInfo sample_input;

    {
        StreamWriter writer(stream_test_filename);
        FullfillFakeSample(&sample_input);

        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteSample(sample_input);
        }
    }

    SampleReader reader(stream_test_filename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextSample(&sample_output));
        ASSERT_EQ(sample_output, sample_input);
    }
    ASSERT_FALSE(reader.GetNextSample(&sample_output));
    ASSERT_FALSE(reader.GetNextModule(nullptr));
}

// Testing reader and writer by writing and reading from .aspt one module
TEST_F(SamplerTest, ModuleWriterReaderTest)
{
    const char *stream_test_filename = "stream_module_test_filename.aspt";
    FileInfo module_input = {pf_id_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo module_output = {};

    {
        StreamWriter writer(stream_test_filename);
        writer.WriteModule(module_input);
    }

    SampleReader reader(stream_test_filename);
    ASSERT_TRUE(reader.GetNextModule(&module_output));
    ASSERT_EQ(module_output, module_input);
    ASSERT_FALSE(reader.GetNextModule(&module_output));
    ASSERT_FALSE(reader.GetNextSample(nullptr));
}

// Testing reader and writer by writing and reading from .aspt lots of modules
TEST_F(SamplerTest, ModuleWriterReaderLotsModulesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *stream_test_filename = "stream_lots_modules_test_filename.aspt";
    FileInfo module_input = {pf_id_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo module_output = {};

    {
        StreamWriter writer(stream_test_filename);
        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteModule(module_input);
        }
    }

    SampleReader reader(stream_test_filename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextModule(&module_output));
        ASSERT_EQ(module_output, module_input);
    }
    ASSERT_FALSE(reader.GetNextModule(&module_output));
    ASSERT_FALSE(reader.GetNextSample(nullptr));
}

// Testing reader and writer by writing and reading from .aspt lots of modules
TEST_F(SamplerTest, WriterReaderLotsRowsModulesAndSamplesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *stream_test_filename = "stream_lots_modules_and_samples_test_filename.aspt";
    FileInfo module_input = {pf_id_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo module_output = {};
    SampleInfo sample_output;
    SampleInfo sample_input;

    {
        StreamWriter writer(stream_test_filename);
        FullfillFakeSample(&sample_input);
        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteModule(module_input);
            writer.WriteSample(sample_input);
        }
    }

    SampleReader reader(stream_test_filename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextModule(&module_output));
        ASSERT_EQ(module_output, module_input);
    }

    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextSample(&sample_output));
        ASSERT_EQ(sample_output, sample_input);
    }

    ASSERT_FALSE(reader.GetNextModule(&module_output));
    ASSERT_FALSE(reader.GetNextSample(&sample_output));
}

// Send sample to listener and check it inside the file
TEST_F(SamplerTest, ListenerWriteFakeSampleTest)
{
    const char *stream_test_filename = "listener_write_fake_sample_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(stream_test_filename), true);

    SampleInfo sample_output;
    SampleInfo sample_input;
    FullfillFakeSample(&sample_input);
    sp->GetCommunicator().SendSample(sample_input);
    sp->Stop();

    bool status = true;
    bool is_passed = false;
    SampleReader reader(stream_test_filename);
    while (status) {
        status = reader.GetNextSample(&sample_output);
        if (sample_output == sample_input) {
            is_passed = true;
            break;
        }
    }

    ASSERT_TRUE(is_passed);

    Sampler::Destroy(sp);
}

// Send lots of sample to listener and check it inside the file
TEST_F(SamplerTest, ListenerWriteLotsFakeSampleTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *stream_test_filename = "listener_write_lots_fake_sample_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(stream_test_filename), true);

    SampleInfo sample_output;
    SampleInfo sample_input;
    size_t sent_samples_counter = 0;
    FullfillFakeSample(&sample_input);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        if (sp->GetCommunicator().SendSample(sample_input)) {
            ++sent_samples_counter;
        }
    }
    sp->Stop();

    bool status = true;
    size_t amount_of_samples = 0;
    SampleReader reader(stream_test_filename);
    while (status) {
        if (sample_output == sample_input) {
            ++amount_of_samples;
        }
        status = reader.GetNextSample(&sample_output);
    }

    ASSERT_EQ(amount_of_samples, sent_samples_counter);

    Sampler::Destroy(sp);
}

// Checking that sampler collect panda files correctly
TEST_F(SamplerTest, CollectPandaFilesTest)
{
    const char *stream_test_filename = "collect_panda_file_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(stream_test_filename), true);
    sp->Stop();

    FileInfo module_info;
    SampleReader reader(stream_test_filename);
    bool status = false;
    while (reader.GetNextModule(&module_info)) {
        auto pf_ptr = reinterpret_cast<panda_file::File *>(module_info.ptr);
        ASSERT_EQ(pf_ptr->GetFullFileName(), module_info.pathname);
        status = true;
    }
    ASSERT_TRUE(status);
    Sampler::Destroy(sp);
}

// Checking that sampler collect panda files correctly
TEST_F(SamplerTest, WriteModuleEventTest)
{
    const char *stream_test_filename = "collect_panda_file_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(stream_test_filename), true);

    auto exec_path = panda::os::file::File::GetExecutablePath();
    std::string pandafile =
        exec_path.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";

    auto pf = panda_file::OpenPandaFileOrZip(pandafile);
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

    // NOTE(mmorozov, skurnevich)
    // this test may fail if the loaded module managed to dump into the trace
    // ASSERT_EQ(ExtractLoadedPFSize(sp), 1);

    sp->Stop();

    FileInfo module_info;
    SampleReader reader(stream_test_filename);
    bool status = false;
    while (reader.GetNextModule(&module_info)) {
        auto pf_ptr = reinterpret_cast<panda_file::File *>(module_info.ptr);
        ASSERT_EQ(pf_ptr->GetFullFileName(), module_info.pathname);
        status = true;
    }
    ASSERT_TRUE(status);

    Sampler::Destroy(sp);
}

// Sampling big pandasm program and convert it
TEST_F(SamplerTest, ProfilerSamplerSignalHandlerTest)
{
    const char *stream_test_filename = "sampler_signal_handler_test.aspt";
    const char *result_test_filename = "sampler_signal_handler_test.csv";
    size_t sample_counter = 0;

    {
        auto *sp = Sampler::Create();
        ASSERT_NE(sp, nullptr);
        ASSERT_EQ(sp->Start(stream_test_filename), true);

        {
            ASSERT_TRUE(Runtime::GetCurrent()->Execute("_GLOBAL::main", {}));
        }
        sp->Stop();

        SampleInfo sample;
        SampleReader reader(stream_test_filename);
        bool is_find = false;

        while (reader.GetNextSample(&sample)) {
            ++sample_counter;
            if (sample.stack_info.managed_stack_size == 2) {
                is_find = true;
                continue;
            }
            ASSERT_NE(sample.stack_info.managed_stack_size, 0);
            ASSERT_EQ(GetThreadId(), sample.thread_info.thread_id);
        }

        ASSERT_EQ(is_find, true);

        Sampler::Destroy(sp);
    }

    // Checking converter
    {
        AsptConverter conv(stream_test_filename);
        ASSERT_EQ(conv.CollectTracesStats(), sample_counter);
        ASSERT_TRUE(conv.CollectModules());
        ASSERT_TRUE(conv.DumpResolvedTracesAsCSV(result_test_filename));
    }
}

}  // namespace panda::tooling::sampler::test
