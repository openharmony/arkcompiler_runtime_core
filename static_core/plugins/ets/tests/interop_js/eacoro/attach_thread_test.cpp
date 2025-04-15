/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <vector>

#include <node_api.h>

#include "macros.h"
#include "plugins/ets/runtime/ani/ani.h"

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vm_api.h"

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value AttachTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value result;
        napi_get_undefined(env, &result);

        ani_vm *etsVM;
        ani_size vmCount;
        [[maybe_unused]] ets_int res = ANI_GetCreatedVMs(&etsVM, 1, &vmCount);
        ASSERT(res == ETS_OK);
        ASSERT(vmCount == 1);

        auto event = os::memory::Event();
        auto t = std::thread([&event, etsVM] {
            ani_env *etsEnv {nullptr};
            ani_option interopEnabled {"--interop=enable", nullptr};
            ani_options aniArgs {1, &interopEnabled};
            etsVM->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv);
            ASSERT(etsEnv != nullptr);
            event.Fire();
            std::function<void()> func = [etsEnv]() {
                ani_class cls;
                [[maybe_unused]] ani_status status = etsEnv->FindClass("Lattach_test/TestClass;", &cls);
                ASSERT(status == ETS_OK);
                ani_static_method method;
                status = etsEnv->Class_FindStaticMethod(cls, "sum", "II:I", &method);
                ASSERT(status == ETS_OK);
                ani_int sum;
                etsEnv->c_api->Class_CallStaticMethod_Int(etsEnv, cls, method, &sum, 5U, 6U);
                ASSERT(sum == 11U);
            };
            func();
            etsVM->DetachCurrentThread();
        });

        event.Wait();
        t.join();

        return result;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"attachTest", 0, AttachTest, 0, 0, 0, napi_enumerable, 0},
        };

        napi_define_properties(env, exports, desc.size(), desc.data());

        return exports;
    }
};

NAPI_MODULE(TEST_MODULE, ark::ets::interop::js::TestModule::Init)

}  // namespace ark::ets::interop::js
