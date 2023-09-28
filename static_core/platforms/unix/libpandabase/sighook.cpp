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

#include "utils/logger.h"
#include <dlfcn.h>

#include <array>
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <os/mutex.h>
#include <type_traits>
#include <utility>
#include <unistd.h>

#include "os/sighook.h"

#include <securec.h>
#include <ucontext.h>

namespace panda {

static decltype(&sigaction) REAL_SIGACTION;
static decltype(&sigprocmask) REAL_SIGPROCMASK;
static bool IS_INIT_REALLY {false};
static bool IS_INIT_KEY_CREATE {false};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
#if PANDA_TARGET_MACOS
__attribute__((init_priority(101))) static os::memory::Mutex REAL_LOCK;
#else
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex REAL_LOCK;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex KEY_CREATE_LOCK;

static os::memory::PandaThreadKey GetHandlingSignalKey()
{
    static os::memory::PandaThreadKey key;
    {
        os::memory::LockHolder lock(KEY_CREATE_LOCK);
        if (!IS_INIT_KEY_CREATE) {
            int rc = os::memory::PandaThreadKeyCreate(&key, nullptr);
            if (rc != 0) {
                LOG(FATAL, RUNTIME) << "failed to create sigchain thread key: " << os::Error(rc).ToString();
            }
            IS_INIT_KEY_CREATE = true;
        }
    }
    return key;
}

static bool GetHandlingSignal()
{
    void *result = os::memory::PandaGetspecific(GetHandlingSignalKey());
    return reinterpret_cast<uintptr_t>(result) != 0;
}

static void SetHandlingSignal(bool value)
{
    os::memory::PandaSetspecific(GetHandlingSignalKey(), reinterpret_cast<void *>(static_cast<uintptr_t>(value)));
}

class SignalHook {
public:
    SignalHook() = default;

    ~SignalHook() = default;

    NO_COPY_SEMANTIC(SignalHook);
    NO_MOVE_SEMANTIC(SignalHook);

    bool IsHook()
    {
        return is_hook_;
    }

    void HookSig(int signo)
    {
        if (!is_hook_) {
            RegisterAction(signo);
            is_hook_ = true;
        }
    }

    void RegisterAction(int signo)
    {
        struct sigaction handler_action = {};
        sigfillset(&handler_action.sa_mask);
        // SIGSEGV from signal handler must be handled as well
        sigdelset(&handler_action.sa_mask, SIGSEGV);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        handler_action.sa_sigaction = SignalHook::Handler;
        // SA_NODEFER+: do not block signals from the signal handler
        // SA_ONSTACK-: call signal handler on the same stack
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        handler_action.sa_flags = SA_RESTART | SA_SIGINFO | SA_NODEFER;
        REAL_SIGACTION(signo, nullptr, &old_action_);
        REAL_SIGACTION(signo, &handler_action, &user_action_);
    }

    void RegisterHookAction(const SighookAction *sa)
    {
        for (SighookAction &handler : hook_action_handlers_) {
            if (handler.sc_sigaction == nullptr) {
                handler = *sa;
                return;
            }
        }
        LOG(FATAL, RUNTIME) << "failed to Register Hook Action, too much handlers";
    }

    void RegisterUserAction(const struct sigaction *new_action)
    {
        user_action_register_ = true;
        if constexpr (std::is_same_v<decltype(user_action_), struct sigaction>) {
            user_action_ = *new_action;
        } else {
            user_action_.sa_flags = new_action->sa_flags;      // NOLINT
            user_action_.sa_handler = new_action->sa_handler;  // NOLINT
#if defined(SA_RESTORER)
            user_action_.sa_restorer = new_action->sa_restorer;  // NOLINT
#endif
            sigemptyset(&user_action_.sa_mask);
            memcpy_s(&user_action_.sa_mask, sizeof(user_action_.sa_mask), &new_action->sa_mask,
                     std::min(sizeof(user_action_.sa_mask), sizeof(new_action->sa_mask)));
        }
    }

    struct sigaction GetUserAction()
    {
        if constexpr (std::is_same_v<decltype(user_action_), struct sigaction>) {
            return user_action_;
        } else {
            struct sigaction result {};
            result.sa_flags = user_action_.sa_flags;      // NOLINT
            result.sa_handler = user_action_.sa_handler;  // NOLINT
#if defined(SA_RESTORER)
            result.sa_restorer = user_action_.sa_restorer;
#endif
            memcpy_s(&result.sa_mask, sizeof(result.sa_mask), &user_action_.sa_mask,
                     std::min(sizeof(user_action_.sa_mask), sizeof(result.sa_mask)));
            return result;
        }
    }

    static void Handler(int signo, siginfo_t *siginfo, void *ucontext_raw);
    static void CallOldAction(int signo, siginfo_t *siginfo, void *ucontext_raw);

    void RemoveHookAction(bool (*action)(int, siginfo_t *, void *))
    {
        // no thread safe
        for (size_t i = 0; i < HOOK_LENGTH; ++i) {
            if (hook_action_handlers_[i].sc_sigaction == action) {
                for (size_t j = i; j < HOOK_LENGTH - 1; ++j) {
                    hook_action_handlers_[j] = hook_action_handlers_[j + 1];
                }
                hook_action_handlers_[HOOK_LENGTH - 1].sc_sigaction = nullptr;
                return;
            }
        }
        LOG(FATAL, RUNTIME) << "failed to find remove hook handler";
    }

    bool IsUserActionRegister()
    {
        return user_action_register_;
    }

    void ClearHookActionHandlers()
    {
        for (SighookAction &handler : hook_action_handlers_) {
            handler.sc_sigaction = nullptr;
        }
    }

private:
    static bool SetHandlingSignal(int signo, siginfo_t *siginfo, void *ucontext_raw);

    constexpr static const int HOOK_LENGTH = 2;
    bool is_hook_ {false};
    std::array<SighookAction, HOOK_LENGTH> hook_action_handlers_ {};
    struct sigaction user_action_ {};
    struct sigaction old_action_ = {};
    bool user_action_register_ {false};
};

static std::array<SignalHook, _NSIG + 1> SIGNAL_HOOKS;

void SignalHook::CallOldAction(int signo, siginfo_t *siginfo, void *ucontext_raw)
{
    auto handler_flags = static_cast<size_t>(SIGNAL_HOOKS[signo].old_action_.sa_flags);
    sigset_t mask = SIGNAL_HOOKS[signo].old_action_.sa_mask;
    REAL_SIGPROCMASK(SIG_SETMASK, &mask, nullptr);

    if ((handler_flags & SA_SIGINFO)) {                                              // NOLINT
        SIGNAL_HOOKS[signo].old_action_.sa_sigaction(signo, siginfo, ucontext_raw);  // NOLINT
    } else {
        if (SIGNAL_HOOKS[signo].old_action_.sa_handler == nullptr) {  // NOLINT
            REAL_SIGACTION(signo, &SIGNAL_HOOKS[signo].old_action_, nullptr);
            kill(getpid(), signo);  // send signal again
            return;
        }
        SIGNAL_HOOKS[signo].old_action_.sa_handler(signo);  // NOLINT
    }
}

bool SignalHook::SetHandlingSignal(int signo, siginfo_t *siginfo, void *ucontext_raw)
{
    for (const auto &handler : SIGNAL_HOOKS[signo].hook_action_handlers_) {
        if (handler.sc_sigaction == nullptr) {
            break;
        }

        bool handler_noreturn = ((handler.sc_flags & SIGHOOK_ALLOW_NORETURN) != 0);
        sigset_t previous_mask;
        REAL_SIGPROCMASK(SIG_SETMASK, &handler.sc_mask, &previous_mask);

        bool old_handle_key = GetHandlingSignal();
        if (!handler_noreturn) {
            ::panda::SetHandlingSignal(true);
        }
        if (handler.sc_sigaction(signo, siginfo, ucontext_raw)) {
            ::panda::SetHandlingSignal(old_handle_key);
            return false;
        }

        REAL_SIGPROCMASK(SIG_SETMASK, &previous_mask, nullptr);
        ::panda::SetHandlingSignal(old_handle_key);
    }

    return true;
}

void SignalHook::Handler(int signo, siginfo_t *siginfo, void *ucontext_raw)
{
    if (!GetHandlingSignal()) {
        if (!SetHandlingSignal(signo, siginfo, ucontext_raw)) {
            return;
        }
    }

    // if not set user handler,call linker handler
    if (!SIGNAL_HOOKS[signo].IsUserActionRegister()) {
        CallOldAction(signo, siginfo, ucontext_raw);
        return;
    }

    // call user handler
    auto handler_flags = static_cast<size_t>(SIGNAL_HOOKS[signo].user_action_.sa_flags);
    auto *ucontext = static_cast<ucontext_t *>(ucontext_raw);
    sigset_t mask;
    sigemptyset(&mask);
    constexpr int N = sizeof(sigset_t) * 2;
    for (int i = 0; i < N; ++i) {
        if (sigismember(&ucontext->uc_sigmask, i) == 1 ||
            sigismember(&SIGNAL_HOOKS[signo].user_action_.sa_mask, i) == 1) {
            sigaddset(&mask, i);
        }
    }

    if ((handler_flags & SA_NODEFER) == 0) {  // NOLINT
        sigaddset(&mask, signo);
    }
    REAL_SIGPROCMASK(SIG_SETMASK, &mask, nullptr);

    if ((handler_flags & SA_SIGINFO)) {                                               // NOLINT
        SIGNAL_HOOKS[signo].user_action_.sa_sigaction(signo, siginfo, ucontext_raw);  // NOLINT
    } else {
        auto handler = SIGNAL_HOOKS[signo].user_action_.sa_handler;  // NOLINT
        if (handler == SIG_IGN) {                                    // NOLINT
            return;
        }
        if (handler == SIG_DFL) {  // NOLINT
            LOG(FATAL, RUNTIME) << "Actually signal:" << signo << " | register sigaction's handler == SIG_DFL";
        }
        handler(signo);
    }

    // if user handler not exit, continue call Old Action
    CallOldAction(signo, siginfo, ucontext_raw);
}

template <typename Sigaction>
static bool FindRealSignal(Sigaction *real_fun, [[maybe_unused]] Sigaction hook_fun, const char *name)
{
    void *find_fun = dlsym(RTLD_NEXT, name);
    if (find_fun != nullptr) {
        *real_fun = reinterpret_cast<Sigaction>(find_fun);
    } else {
        find_fun = dlsym(RTLD_DEFAULT, name);
        if (find_fun == nullptr || reinterpret_cast<uintptr_t>(find_fun) == reinterpret_cast<uintptr_t>(hook_fun) ||
            reinterpret_cast<uintptr_t>(find_fun) == reinterpret_cast<uintptr_t>(sigaction)) {
            LOG(ERROR, RUNTIME) << "dlsym(RTLD_DEFAULT, " << name << ") can not find really " << name;
            return false;
        }
        *real_fun = reinterpret_cast<Sigaction>(find_fun);
    }
    LOG(INFO, RUNTIME) << "find " << name << " success";
    return true;
}

#if PANDA_TARGET_MACOS
__attribute__((constructor(102))) static bool InitRealSignalFun()
#else
__attribute__((constructor)) static bool InitRealSignalFun()
#endif
{
    {
        os::memory::LockHolder lock(REAL_LOCK);
        if (!IS_INIT_REALLY) {
            bool is_error = true;
            is_error = is_error && FindRealSignal(&REAL_SIGACTION, sigaction, "sigaction");
            is_error = is_error && FindRealSignal(&REAL_SIGPROCMASK, sigprocmask, "sigprocmask");
            if (is_error) {
                IS_INIT_REALLY = true;
            }
            return is_error;
        }
    }
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
static int RegisterUserHandler(int signal, const struct sigaction *new_action, struct sigaction *old_action,
                               int (*really)(int, const struct sigaction *, struct sigaction *))
{
    // just hook signal in range, other use libc sigaction
    if (signal <= 0 || signal >= _NSIG) {
        LOG(ERROR, RUNTIME) << "illegal signal " << signal;
        return -1;
    }

    if (SIGNAL_HOOKS[signal].IsHook()) {
        auto user_action = SIGNAL_HOOKS[signal].SignalHook::GetUserAction();
        if (new_action != nullptr) {
            SIGNAL_HOOKS[signal].RegisterUserAction(new_action);
        }
        if (old_action != nullptr) {
            *old_action = user_action;
        }
        return 0;
    }

    return really(signal, new_action, old_action);
}

int RegisterUserMask(int how, const sigset_t *new_set, sigset_t *old_set,
                     int (*really)(int, const sigset_t *, sigset_t *))
{
    if (GetHandlingSignal()) {
        return really(how, new_set, old_set);
    }

    if (new_set == nullptr) {
        return really(how, new_set, old_set);
    }

    sigset_t build_sigset = *new_set;
    if (how == SIG_BLOCK || how == SIG_SETMASK) {
        for (int i = 1; i < _NSIG; ++i) {
            if (SIGNAL_HOOKS[i].IsHook() && (sigismember(&build_sigset, i) != 0)) {
                sigdelset(&build_sigset, i);
            }
        }
    }
    const sigset_t *build_sigset_const = &build_sigset;
    return really(how, build_sigset_const, old_set);
}

// NOTE: #2681
// when use ADDRESS_SANITIZER, will exposed a bug,try to define 'sigaction' will happen SIGSEGV
#ifdef USE_ADDRESS_SANITIZER
// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" int sigaction([[maybe_unused]] int __sig, [[maybe_unused]] const struct sigaction *__restrict __act,
                         [[maybe_unused]] struct sigaction *__oact)  // NOLINT(readability-identifier-naming)
{
    if (!InitRealSignalFun()) {
        return -1;
    }
    return RegisterUserHandler(__sig, __act, __oact, REAL_SIGACTION);
}
#else
// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" int sigactionStub([[maybe_unused]] int __sig, [[maybe_unused]] const struct sigaction *__restrict __act,
                             [[maybe_unused]] struct sigaction *__oact)  // NOLINT(readability-identifier-naming)
{
    if (!InitRealSignalFun()) {
        return -1;
    }
    return RegisterUserHandler(__sig, __act, __oact, REAL_SIGACTION);
}
#endif  // USE_ADDRESS_SANITIZER

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" int sigprocmask(int how, const sigset_t *new_set, sigset_t *old_set)  // NOLINT
{
    if (!InitRealSignalFun()) {
        return -1;
    }
    return RegisterUserMask(how, new_set, old_set, REAL_SIGPROCMASK);
}

extern "C" void RegisterHookHandler(int signal, const SighookAction *sa)
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    SIGNAL_HOOKS[signal].RegisterHookAction(sa);
    SIGNAL_HOOKS[signal].HookSig(signal);
}

extern "C" void RemoveHookHandler(int signal, bool (*action)(int, siginfo_t *, void *))
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    SIGNAL_HOOKS[signal].RemoveHookAction(action);
}

extern "C" void CheckOldHookHandler(int signal)
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    // get old action
    struct sigaction old_action {};
    REAL_SIGACTION(signal, nullptr, &old_action);

    if (old_action.sa_sigaction != SignalHook::Handler) {  // NOLINT
        LOG(ERROR, RUNTIME) << "error: Check old hook handler found unexpected action "
                            << (old_action.sa_sigaction != nullptr);  // NOLINT
        SIGNAL_HOOKS[signal].RegisterAction(signal);
    }
}

extern "C" void AddSpecialSignalHandlerFn(int signal, SigchainAction *sa)
{
    LOG(DEBUG, RUNTIME) << "panda sighook RegisterHookHandler is used, signal:" << signal << " action:" << sa;
    RegisterHookHandler(signal, reinterpret_cast<SighookAction *>(sa));
}
extern "C" void RemoveSpecialSignalHandlerFn(int signal, bool (*fn)(int, siginfo_t *, void *))
{
    LOG(DEBUG, RUNTIME) << "panda sighook RemoveHookHandler is used, signal:"
                        << "sigaction";
    RemoveHookHandler(signal, fn);
}

extern "C" void EnsureFrontOfChain(int signal)
{
    LOG(DEBUG, RUNTIME) << "panda sighook CheckOldHookHandler is used, signal:" << signal;
    CheckOldHookHandler(signal);
}

void ClearSignalHooksHandlersArray()
{
    IS_INIT_REALLY = false;
    IS_INIT_KEY_CREATE = false;
    for (int i = 1; i < _NSIG; i++) {
        SIGNAL_HOOKS[i].ClearHookActionHandlers();
    }
}

}  // namespace panda
