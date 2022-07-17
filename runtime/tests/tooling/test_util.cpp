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

#include "test_util.h"

#include "include/method.h"
#include "include/tooling/pt_location.h"

namespace panda::tooling::test {

TestMap TestUtil::test_map_;
os::memory::Mutex TestUtil::event_mutex_;
os::memory::ConditionVariable TestUtil::event_cv_;
DebugEvent TestUtil::last_event_ = DebugEvent::UNINITIALIZED;
bool TestUtil::initialized_ = false;
os::memory::Mutex TestUtil::suspend_mutex_;
os::memory::ConditionVariable TestUtil::suspend_cv_;
bool TestUtil::suspended_;
PtThread TestUtil::last_event_thread_ = PtThread::NONE;
PtLocation TestUtil::last_event_location_("", EntityId(0), 0);
TestExtractorFactory *TestUtil::extractor_factory_;

std::vector<panda_file::LocalVariableInfo> TestUtil::GetVariables(Method *method, uint32_t offset)
{
    auto methodId = method->GetFileId();
    auto pf = method->GetPandaFile();
    PtLocation location(pf->GetFilename().c_str(), methodId, offset);
    return GetVariables(pf, location);
}

std::vector<panda_file::LocalVariableInfo> TestUtil::GetVariables(const panda_file::File *pf, PtLocation location)
{
    auto extractor = extractor_factory_->MakeTestExtractor(pf);
    return extractor->GetLocalVariableInfo(location.GetMethodId(), location.GetBytecodeOffset());
}

int32_t TestUtil::GetValueRegister(Method *method, const char *varName, uint32_t offset)
{
    auto variables = TestUtil::GetVariables(method, offset);
    for (const auto &var : variables) {
        if (var.name == varName) {
            return var.reg_number;
        }
    }

    return -2;  // TODO(maksenov): Replace with invalid register constant;
}

std::ostream &operator<<(std::ostream &out, DebugEvent value)
{
    const char *s = nullptr;

#define ADD_CASE(entry) \
    case (entry):       \
        s = #entry;     \
        break;

    switch (value) {
        ADD_CASE(DebugEvent::BREAKPOINT);
        ADD_CASE(DebugEvent::LOAD_MODULE);
        ADD_CASE(DebugEvent::PAUSED);
        ADD_CASE(DebugEvent::EXCEPTION);
        ADD_CASE(DebugEvent::EXCEPTION_CATCH);
        ADD_CASE(DebugEvent::FIELD_ACCESS);
        ADD_CASE(DebugEvent::FIELD_MODIFICATION);
        ADD_CASE(DebugEvent::FRAME_POP);
        ADD_CASE(DebugEvent::GARBAGE_COLLECTIION_START);
        ADD_CASE(DebugEvent::GARBAGE_COLLECTIION_FINISH);
        ADD_CASE(DebugEvent::METHOD_ENTRY);
        ADD_CASE(DebugEvent::METHOD_EXIT);
        ADD_CASE(DebugEvent::SINGLE_STEP);
        ADD_CASE(DebugEvent::THREAD_START);
        ADD_CASE(DebugEvent::THREAD_END);
        ADD_CASE(DebugEvent::VM_START);
        ADD_CASE(DebugEvent::VM_INITIALIZATION);
        ADD_CASE(DebugEvent::VM_DEATH);
        ADD_CASE(DebugEvent::EXCEPTION_REVOKED);
        ADD_CASE(DebugEvent::EXECUTION_CONTEXT_CREATED);
        ADD_CASE(DebugEvent::EXECUTION_CONTEXT_DESTROYED);
        ADD_CASE(DebugEvent::EXECUTION_CONTEXT_CLEARED);
        ADD_CASE(DebugEvent::INSPECT_REQUESTED);
        ADD_CASE(DebugEvent::CLASS_LOAD);
        ADD_CASE(DebugEvent::CLASS_PREPARE);
        ADD_CASE(DebugEvent::MONITOR_WAIT);
        ADD_CASE(DebugEvent::MONITOR_WAITED);
        ADD_CASE(DebugEvent::MONITOR_CONTENDED_ENTER);
        ADD_CASE(DebugEvent::MONITOR_CONTENDED_ENTERED);
        ADD_CASE(DebugEvent::UNINITIALIZED);
        default: {
            ASSERT(false && "Unknown DebugEvent");  // NOLINT
        }
    }

#undef ADD_CASE

    return out << s;
}

#ifndef PANDA_TARGET_MOBILE
std::ostream &operator<<(std::ostream &out, std::nullptr_t)
{
    return out << "nullptr";
}
#endif  // PANDA_TARGET_MOBILE

ApiTest::ApiTest()
{
    scenario = []() {
        ASSERT_EXITED();
        return true;
    };
}

void SetExtractorFactoryForTest(TestExtractorFactory *factory)
{
    TestUtil::SetExtractorFactory(factory);
}
}  // namespace panda::tooling::test
