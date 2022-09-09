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

#include "libpandabase/os/native_stack.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "utils/logger.h"
#include "utils/span.h"
#include "verification/jobs/thread_pool.h"
#include "verification/jobs/cache.h"
#include "verification/util/is_system.h"
#include "generated/base_options.h"

// generated
#include "ark_version.h"
#include "generated/verifier_options_gen.h"

#include <csignal>

namespace panda::verifier {

bool RunVerifier(const Options &cli_options)
{
    auto &runtime = *Runtime::GetCurrent();
    auto &class_linker = *runtime.GetClassLinker();
    {
        // block to destruct cache_api ASAP
        auto cache_api = ThreadPool::GetCache()->FastAPI();
        cache_api.ProcessFiles(class_linker.GetBootPandaFiles());
        class_linker.EnumeratePandaFiles([&cache_api](const panda_file::File &pf) {
            cache_api.ProcessFile(pf);
            return true;
        });
    }

    const std::vector<std::string> &class_names = cli_options.GetClasses();
    const std::vector<std::string> &method_names = cli_options.GetMethods();

    std::vector<Method *> enqueued;
    bool result = true;

    auto enqueue_method = [&](Method *method) {
        method->EnqueueForVerification();
        enqueued.push_back(method);
    };

    bool verifyLibraries = runtime.GetVerificationOptions().VerifyRuntimeLibraries;

    auto enqueue_class = [&](const Class &klass) {
        // this is already checked in enqueue_method, but faster to skip them in the first place
        if (!verifyLibraries && IsSystemClass(klass)) {
            LOG(INFO, VERIFIER) << klass.GetName() << " is a system class, skipping";
            return;
        }
        LOG(INFO, VERIFIER) << "Begin verification of class " << klass.GetName();
        for (auto &method : klass.GetMethods()) {
            enqueue_method(&method);
        }
    };

    // we need ScopedManagedCodeThread for verify since it can allocate objects
    ScopedManagedCodeThread managed_obj_thread(MTManagedThread::GetCurrent());
    if (class_names.empty() && method_names.empty()) {
        auto handle_file = [&](const panda_file::File &file) {
            for (auto id : file.GetClasses()) {
                panda_file::File::EntityId entity_id {id};
                if (!file.IsExternal(entity_id)) {
                    auto opt_lang = panda_file::ClassDataAccessor {file, entity_id}.GetSourceLang();
                    ClassLinkerExtension *ext =
                        class_linker.GetExtension(opt_lang.value_or(panda_file::SourceLang::PANDA_ASSEMBLY));
                    if (ext == nullptr) {
                        LOG(ERROR, VERIFIER) << "Error: Class Linker Extension failed to initialize";
                        return false;
                    }
                    const Class *klass = ext->GetClass(file, entity_id);

                    if (klass != nullptr) {
                        enqueue_class(*klass);
                    }
                }
            }
            return true;
        };

        class_linker.EnumeratePandaFiles(handle_file);

        if (verifyLibraries) {
            class_linker.EnumerateBootPandaFiles(handle_file);
        } else if (runtime.GetPandaFiles().empty()) {
            // in this case the last boot-panda-file and only it is actually not a system file and should be
            // verified
            OptionalConstRef<panda_file::File> file;
            class_linker.EnumerateBootPandaFiles([&file](const panda_file::File &pf) {
                file = std::cref(pf);
                return true;
            });
            if (file.HasRef()) {
                handle_file(*file);
            } else {
                LOG(ERROR, VERIFIER) << "No files given to verify";
            }
        }
    } else {
        PandaUnorderedMap<std::string, Class *> classes_by_name;
        ClassLinkerContext *ctx =
            class_linker.GetExtension(runtime.GetLanguageContext(runtime.GetRuntimeType()))->GetBootContext();

        // TODO(romanov) make this a simple method in LibCache when switching from ClassLinker
        auto get_class_by_name = [&](const std::string &class_name) -> Class * {
            auto it = classes_by_name.find(class_name);
            if (it != classes_by_name.end()) {
                return it->second;
            }

            PandaString descriptor;
            const uint8_t *class_name_bytes =
                ClassHelper::GetDescriptor(utf::CStringAsMutf8(class_name.c_str()), &descriptor);
            Class *klass = class_linker.GetClass(class_name_bytes, true, ctx);
            if (klass == nullptr) {
                LOG(ERROR, VERIFIER) << "Error: Cannot resolve class with name " << class_name;
                result = false;
            }

            classes_by_name.emplace(class_name, klass);
            return klass;
        };

        for (const auto &class_name : class_names) {
            Class *klass = get_class_by_name(class_name);
            // the bad case is already handled in get_class_by_name
            if (klass != nullptr) {
                enqueue_class(*klass);
            }
        }

        for (const std::string &fq_method_name : method_names) {
            size_t pos = fq_method_name.find_last_of("::");
            if (pos == std::string::npos) {
                LOG(ERROR, VERIFIER) << "Error: Fully qualified method name must contain '::', was " << fq_method_name;
                result = false;
                break;
            }
            // TODO(romanov) make string_view after removing ClassLinker, now needed for get_class_by_name for
            // technical reasons
            std::string class_name = fq_method_name.substr(0, pos - 1);
            std::string_view unqualified_method_name = std::string_view(fq_method_name).substr(pos + 1);
            if (std::find(class_names.begin(), class_names.end(), class_name) != class_names.end()) {
                // this method was already verified while enumerating class_names
                continue;
            }
            Class *klass = get_class_by_name(class_name);
            if (klass != nullptr) {
                bool method_found = false;
                for (auto &method : klass->GetMethods()) {
                    const char *name_data = utf::Mutf8AsCString(method.GetName().data);
                    if (std::string_view(name_data) == unqualified_method_name) {
                        method_found = true;
                        LOG(INFO, VERIFIER) << "Verification of method '" << fq_method_name << "'";
                        enqueue_method(&method);
                    }
                }
                if (!method_found) {
                    LOG(ERROR, VERIFIER) << "Error: Cannot resolve method with name " << unqualified_method_name
                                         << " in class " << class_name;
                    result = false;
                }
            }
        }
    }

    for (Method *method : enqueued) {
        if (!method->Verify()) {
            LOG(ERROR, VERIFIER) << "Error: method " << method->GetFullName(true) << " failed to verify";
            result = false;
        }
    }
    return result;
}

void BlockSignals()
{
#if defined(PANDA_TARGET_UNIX)
    sigset_t set;
    if (sigemptyset(&set) == -1) {
        LOG(ERROR, RUNTIME) << "sigemptyset failed";
        return;
    }
    int rc = 0;
#ifdef PANDA_TARGET_MOBILE
    rc += sigaddset(&set, SIGPIPE);
    rc += sigaddset(&set, SIGQUIT);
    rc += sigaddset(&set, SIGUSR1);
    rc += sigaddset(&set, SIGUSR2);
#endif  // PANDA_TARGET_MOBILE
    if (rc < 0) {
        LOG(ERROR, RUNTIME) << "sigaddset failed";
        return;
    }

    if (os::native_stack::g_PandaThreadSigmask(SIG_BLOCK, &set, nullptr) != 0) {
        LOG(ERROR, RUNTIME) << "g_PandaThreadSigmask failed";
    }
#endif  // PANDA_TARGET_UNIX
}

void PrintHelp(const PandArgParser &pa_parser)
{
    std::string error = pa_parser.GetErrorString();
    if (!error.empty()) {
        error += "\n\n";
    }
    std::cerr << error << "Usage: verifier [option...] [file]\n"
              << "Verify specified Panda files (given by file and --panda-files) "
              << "or certain classes/methods in them.\n\n"
              << pa_parser.GetHelpString() << std::endl;
}

int Main(int argc, const char **argv)
{
    BlockSignals();
    Span<const char *> sp(argv, argc);
    RuntimeOptions runtime_options(sp[0]);
    Options cli_options(sp[0]);
    PandArgParser pa_parser;
    base_options::Options base_options("");

    PandArg<bool> help("help", false, "Print this message and exit");
    PandArg<bool> options("options", false, "Print verifier options");
    // tail argument
    PandArg<std::string> file("file", "", "path to pandafile");

    cli_options.AddOptions(&pa_parser);

    pa_parser.Add(&help);
    pa_parser.Add(&options);
    pa_parser.PushBackTail(&file);
    pa_parser.EnableTail();

    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return 1;
    }

    if (runtime_options.IsVersion()) {
        PrintPandaVersion();
        return 0;
    }

    if (help.GetValue()) {
        PrintHelp(pa_parser);
        return 0;
    }

    if (options.GetValue()) {
        std::cout << pa_parser.GetRegularArgs() << std::endl;
        return 0;
    }

    auto cli_options_err = cli_options.Validate();
    if (cli_options_err) {
        std::cerr << "Error: " << cli_options_err.value().GetMessage() << std::endl;
        return 1;
    }

    auto boot_panda_files = cli_options.GetBootPandaFiles();
    auto panda_files = cli_options.GetPandaFiles();
    std::string main_file_name = file.GetValue();

    if (!main_file_name.empty()) {
        if (panda_files.empty()) {
            boot_panda_files.push_back(main_file_name);
        } else {
            auto found_iter = std::find_if(panda_files.begin(), panda_files.end(),
                                           [&](auto &file_name) { return main_file_name == file_name; });
            if (found_iter == panda_files.end()) {
                panda_files.push_back(main_file_name);
            }
        }
    }

    runtime_options.SetBootPandaFiles(boot_panda_files);
    runtime_options.SetPandaFiles(panda_files);
    runtime_options.SetLoadRuntimes(cli_options.GetLoadRuntimes());

    base_options.SetLogComponents(cli_options.GetLogComponents());
    base_options.SetLogLevel(cli_options.GetLogLevel());
    base_options.SetLogStream(cli_options.GetLogStream());
    base_options.SetLogFile(cli_options.GetLogFile());
    Logger::Initialize(base_options);

    runtime_options.SetLimitStandardAlloc(cli_options.IsLimitStandardAlloc());
    runtime_options.SetInternalAllocatorType(cli_options.GetInternalAllocatorType());
    runtime_options.SetInternalMemorySizeLimit(cli_options.GetInternalMemorySizeLimit());

    runtime_options.SetVerificationMode(cli_options.IsDebugMode() ? VerificationMode::DEBUG
                                                                  : VerificationMode::AHEAD_OF_TIME);
    runtime_options.SetVerificationConfigFile(cli_options.GetConfigFile());
    runtime_options.SetVerificationCacheFile(cli_options.GetCacheFile());
    runtime_options.SetVerificationUpdateCache(cli_options.IsUpdateCache());
    runtime_options.SetVerifyRuntimeLibraries(cli_options.IsVerifyRuntimeLibraries());
    uint32_t threads = cli_options.GetThreads();
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
        // hardware_concurrency can return 0 if the value is not computable or well defined
        if (threads == 0) {
            threads = 1;
        } else if (threads > MAX_THREADS) {
            threads = MAX_THREADS;
        }
    }
    runtime_options.SetVerificationThreads(threads);

    if (!Runtime::Create(runtime_options)) {
        std::cerr << "Error: cannot create runtime" << std::endl;
        return -1;
    }

    int ret = RunVerifier(cli_options) ? 0 : -1;

    if (cli_options.IsPrintMemoryStatistics()) {
        std::cout << Runtime::GetCurrent()->GetMemoryStatistics();
    }
    if (!Runtime::Destroy()) {
        std::cerr << "Error: cannot destroy runtime" << std::endl;
        return -1;
    }
    return ret;
}

}  // namespace panda::verifier

int main(int argc, const char **argv)
{
    return panda::verifier::Main(argc, argv);
}
