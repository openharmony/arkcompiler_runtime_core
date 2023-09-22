/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_PANDA_TASK_RUNNER_H
#define LIBPANDABASE_PANDA_TASK_RUNNER_H

#include "libpandabase/macros.h"

#include <functional>
#include <thread>

// clang-format off
/**
*    panda::TaskRunner runner;
*
*    runner.AddFinalize(foo);
*    runner.AddCallbackOnSuccess(foo1);
*    runner.AddCallbackOnFail(foo2);
*    runner.SetTaskOnSuccess(next_task);
*    bool success = task();
*    TaskRunner::EndTask(runner, success); --> if success --
*                                              |           |
*                                              |           |
*                                              |           |
*                                              |           if has_task_on_success -->-- StartTask(runner, next_task);
*                                              |           |
*                                              |           |
*                                              |           |
*                                              |           else -->------ X ------ call foo1;
*                                              |                                   call foo;
*                                              |
*                                              |
*                                              |
*                                              else ------ if has_task_on_fail -->---- X
*                                                          |
*                                                          |
*                                                          |
*                                                          else -->---- call foo2;
*                                                                       call foo;
*/
// clang-format on

namespace panda {

/**
 * @brief TaskRunner class implements interface to add callbacks and set next tasks to be executed
 * @tparam RunnerT - an inherited class that contains ContextT and specifies StartTask and GetContext methods
 * @tparam ContexT - a class representing the context of the task
 */
template <typename RunnerT, typename ContextT>
class TaskRunner {
public:
    TaskRunner() = default;
    NO_COPY_SEMANTIC(TaskRunner);
    DEFAULT_MOVE_SEMANTIC(TaskRunner);
    ~TaskRunner() = default;

    // We must use && to avoid slicing
    using TaskFunc = std::function<void(RunnerT)>;
    using Callback = std::function<void(ContextT &)>;

    virtual ContextT &GetContext() = 0;
    static void StartTask(RunnerT task_runner, TaskFunc task_func);

    class TaskCallback {
    public:
        TaskCallback() = default;
        NO_COPY_SEMANTIC(TaskCallback);
        NO_MOVE_OPERATOR(TaskCallback);
        TaskCallback(TaskCallback &&task_cb)
            : callback_on_success_(std::move(task_cb.callback_on_success_)),
              callback_on_fail_(std::move(task_cb.callback_on_fail_))
        {
            task_cb.SetNeedMakeCall(false);
        }
        // NOLINTNEXTLINE(modernize-use-equals-default)
        ~TaskCallback()
        {
            ASSERT(!need_make_call_);
        }

        void RunOnSuccess(ContextT &task_ctx)
        {
            if (callback_on_success_) {
                callback_on_success_(task_ctx);
            }
            SetNeedMakeCall(false);
        }

        void RunOnFail(ContextT &task_ctx)
        {
            if (callback_on_fail_) {
                callback_on_fail_(task_ctx);
            }
            SetNeedMakeCall(false);
        }

        template <typename Foo>
        void AddOnSuccess(Foo foo)
        {
            if (LIKELY(callback_on_success_)) {
                callback_on_success_ = NextCallback(std::move(callback_on_success_), std::move(foo));
            } else {
                callback_on_success_ = std::move(foo);
            }
            SetNeedMakeCall(true);
        }

        template <typename Foo>
        void AddOnFail(Foo foo)
        {
            if (LIKELY(callback_on_fail_)) {
                callback_on_fail_ = NextCallback(std::move(callback_on_fail_), std::move(foo));
            } else {
                callback_on_fail_ = std::move(foo);
            }
            SetNeedMakeCall(true);
        }

        void SetNeedMakeCall([[maybe_unused]] bool need_make_call)
        {
#ifndef NDEBUG
            need_make_call_ = need_make_call;
#endif
        }

    private:
        template <typename Foo>
        Callback NextCallback(Callback &&cb, Foo foo)
        {
            return [cb = std::move(cb), foo = std::move(foo)](ContextT &task_ctx) mutable {
                foo(task_ctx);
                cb(task_ctx);
            };
        }

        Callback callback_on_success_;
        Callback callback_on_fail_;
#ifndef NDEBUG
        bool need_make_call_ {false};
#endif
    };

    class Task {
    public:
        Task() = default;
        NO_COPY_SEMANTIC(Task);
        NO_MOVE_OPERATOR(Task);
        Task(Task &&task) : task_func_(std::move(task.task_func_)), has_task_(task.has_task_)
        {
            task.has_task_ = false;
        }
        ~Task() = default;

        explicit operator bool() const
        {
            return has_task_;
        }

        template <typename Foo>
        void SetTaskFunc(Foo foo)
        {
            ASSERT(!has_task_);
            task_func_ = std::move(foo);
            has_task_ = true;
        }

        TaskFunc GetTaskFunc()
        {
            has_task_ = false;
            return std::move(task_func_);
        }

    private:
        TaskFunc task_func_;
        bool has_task_ {false};
    };

    /// @brief Starts a chain of callbacks on success
    void RunCallbackOnSuccess()
    {
        auto &context = GetContext();
        task_cb_.RunOnSuccess(context);
    }

    /// @brief Starts a chain of callbacks on fail
    void RunCallbackOnFail()
    {
        auto &context = GetContext();
        task_cb_.RunOnFail(context);
    }

    /**
     * @brief Adds callback to the chain on success
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddCallbackOnSuccess(Foo foo)
    {
        ASSERT(!task_on_success_);
        task_cb_.AddOnSuccess(std::move(foo));
    }

    /**
     * @brief Adds callback to the chain on fail
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddCallbackOnFail(Foo foo)
    {
        ASSERT(!task_on_fail_);
        task_cb_.AddOnFail(std::move(foo));
    }

    /**
     * @brief Adds callback to the chain on success and on fail
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddFinalize(Foo foo)
    {
        AddCallbackOnSuccess(foo);
        AddCallbackOnFail(std::move(foo));
    }

    /**
     * @brief Sets next task on success. After this call you must not add callback on success
     * @param foo - callable object that gets RunnerT &&
     */
    template <typename Foo>
    void SetTaskOnSuccess(Foo foo)
    {
        ASSERT(!task_on_success_);
        task_on_success_.SetTaskFunc(std::move(foo));
    }

    /**
     * @brief Sets next task on fail. After this call you must not add callback on fail
     * @param foo - callable object that gets RunnerT &&
     */
    template <typename Foo>
    void SetTaskOnFail(Foo foo)
    {
        ASSERT(!task_on_fail_);
        task_on_fail_.SetTaskFunc(std::move(foo));
    }

    /**
     * @brief Completes the current task and starts the next one if it was set.
     * Otherwise, it calls callbacks depending on @param success
     * @param task_runner - Current TaskRunner
     * @param success - result of current task
     */
    static void EndTask(RunnerT task_runner, bool success)
    {
        auto &base_runner = static_cast<TaskRunner &>(task_runner);
        auto task_on_success = std::move(base_runner.task_on_success_);
        auto task_on_fail = std::move(base_runner.task_on_fail_);
        ASSERT(!base_runner.task_on_success_ && !base_runner.task_on_fail_);
        ContextT &task_ctx = task_runner.GetContext();
        if (success) {
            if (task_on_success) {
                RunnerT::StartTask(std::move(task_runner), task_on_success.GetTaskFunc());
                return;
            }
            base_runner.task_cb_.RunOnSuccess(task_ctx);
        } else {
            if (task_on_fail) {
                RunnerT::StartTask(std::move(task_runner), task_on_fail.GetTaskFunc());
                return;
            }
            base_runner.task_cb_.RunOnFail(task_ctx);
        }
    }

private:
    TaskCallback task_cb_;
    Task task_on_success_;
    Task task_on_fail_;
};

}  // namespace panda

#endif  // LIBPANDABASE_PANDA_TASK_RUNNER_H
