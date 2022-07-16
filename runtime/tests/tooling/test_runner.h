/*
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
#ifndef PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H
#define PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H

#include "test_util.h"

namespace panda::tooling::test {
class TestRunner : public PtHooks {
public:
    TestRunner(const char *test_name, DebugInterface *runtime_debug_interface)
    {
        debug_interface_ = runtime_debug_interface;
        test_name_ = test_name;
        test_ = TestUtil::GetTest(test_name);
        test_->debug_interface = runtime_debug_interface;
        TestUtil::Reset();
        runtime_debug_interface->RegisterHooks(this);
    }

    void Run()
    {
        if (test_->scenario) {
            test_->scenario();
        }
    }

    void Breakpoint(PtThread thread, Method *method, const PtLocation &location) override
    {
        if (test_->breakpoint) {
            test_->breakpoint(thread, method, location);
        }
    }

    void LoadModule(std::string_view panda_file_name) override
    {
        if (test_->load_module) {
            test_->load_module(panda_file_name);
        }
    }

    void Paused(PauseReason reason) override
    {
        if (test_->paused) {
            test_->paused(reason);
        }
    };

    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                   Method *catch_method, const PtLocation &catch_location) override
    {
        if (test_->exception) {
            test_->exception(thread, method, location, object, catch_method, catch_location);
        }
    }

    void ExceptionCatch(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object) override
    {
        if (test_->exception_catch) {
            test_->exception_catch(thread, method, location, object);
        }
    }

    void PropertyAccess(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                        PtProperty property) override
    {
        if (test_->property_access) {
            test_->property_access(thread, method, location, object, property);
        }
    }

    void PropertyModification(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                              PtProperty property, VRegValue new_value) override
    {
        if (test_->property_modification) {
            test_->property_modification(thread, method, location, object, property, new_value);
        }
    }

    void FramePop(PtThread thread, Method *method, bool was_popped_by_exception) override
    {
        if (test_->frame_pop) {
            test_->frame_pop(thread, method, was_popped_by_exception);
        }
    }

    void GarbageCollectionStart() override
    {
        if (test_->garbage_collection_start) {
            test_->garbage_collection_start();
        }
    }

    void GarbageCollectionFinish() override
    {
        if (test_->garbage_collection_finish) {
            test_->garbage_collection_finish();
        }
    }

    void MethodEntry(PtThread thread, Method *method) override
    {
        if (test_->method_entry) {
            test_->method_entry(thread, method);
        }
    }

    void MethodExit(PtThread thread, Method *method, bool was_popped_by_exception, VRegValue return_value) override
    {
        if (test_->method_exit) {
            test_->method_exit(thread, method, was_popped_by_exception, return_value);
        }
    }

    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override
    {
        if (test_->single_step) {
            test_->single_step(thread, method, location);
        }
    }

    void ThreadStart(PtThread thread) override
    {
        if (test_->thread_start) {
            test_->thread_start(thread);
        }
    }

    void ThreadEnd(PtThread thread) override
    {
        if (test_->thread_end) {
            test_->thread_end(thread);
        }
    }

    void VmDeath() override
    {
        if (test_->vm_death) {
            test_->vm_death();
        }
        TestUtil::Event(DebugEvent::VM_DEATH);
    }

    void VmInitialization(PtThread thread) override
    {
        if (test_->vm_init) {
            test_->vm_init(thread);
        }
        TestUtil::Event(DebugEvent::VM_INITIALIZATION);
    }

    void VmStart() override
    {
        if (test_->vm_start) {
            test_->vm_start();
        }
    }

    void ExceptionRevoked(ExceptionWrapper reason, ExceptionID exception_id) override
    {
        if (test_->exception_revoked) {
            test_->exception_revoked(reason, exception_id);
        }
    }

    void ExecutionContextCreated(ExecutionContextWrapper context) override
    {
        if (test_->execution_context_created) {
            test_->execution_context_created(context);
        }
    }

    void ExecutionContextDestroyed(ExecutionContextWrapper context) override
    {
        if (test_->execution_context_destroyed) {
            test_->execution_context_destroyed(context);
        }
    }

    void ExecutionContextsCleared() override
    {
        if (test_->execution_context_cleared) {
            test_->execution_context_cleared();
        }
    }

    void InspectRequested(PtObject object, PtObject hints) override
    {
        if (test_->inspect_requested) {
            test_->inspect_requested(object, hints);
        }
    }

    void ClassLoad(PtThread thread, BaseClass *klass) override
    {
        if (test_->class_load) {
            test_->class_load(thread, klass);
        }
    }

    void ClassPrepare(PtThread thread, BaseClass *klass) override
    {
        if (test_->class_prepare) {
            test_->class_prepare(thread, klass);
        }
    }

    void MonitorWait(PtThread thread, ObjectHeader *object, int64_t timeout) override
    {
        if (test_->monitor_wait) {
            test_->monitor_wait(thread, object, timeout);
        }
    }

    void MonitorWaited(PtThread thread, ObjectHeader *object, bool timed_out) override
    {
        if (test_->monitor_waited) {
            test_->monitor_waited(thread, object, timed_out);
        }
    }

    void MonitorContendedEnter(PtThread thread, ObjectHeader *object) override
    {
        if (test_->monitor_contended_enter) {
            test_->monitor_contended_enter(thread, object);
        }
    }

    void MonitorContendedEntered(PtThread thread, ObjectHeader *object) override
    {
        if (test_->monitor_contended_entered) {
            test_->monitor_contended_entered(thread, object);
        }
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, PtThread thread, size_t size) override
    {
        if (test_->object_alloc) {
            test_->object_alloc(klass, object, thread, size);
        }
    }

    void TerminateTest()
    {
        debug_interface_->UnregisterHooks();
        if (TestUtil::IsTestFinished()) {
            return;
        }
        LOG(FATAL, DEBUGGER) << "Test " << test_name_ << " failed";
    }

private:
    DebugInterface *debug_interface_;
    const char *test_name_;
    ApiTest *test_;
};
}  // namespace panda::tooling::test

#endif  // PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H
