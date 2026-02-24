/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/tooling/internal_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#include <ani.h>
#include <gtest/gtest.h>

namespace ark::ets::interop::js::testing {

bool CompFrameType(const std::vector<bool> &isStaticFrameV, const std::vector<bool> &compType)
{
    bool res = true;
    ASSERT(compType.size() == isStaticFrameV.size());

    int frameSize = compType.size();
    for (int i = 0; i < frameSize; i++) {
        res = res && (compType[i] == isStaticFrameV[frameSize - i - 1]);
    }
    ASSERT(res);
    return res;
}

static bool TestHybridStack()
{
    std::vector<bool> isStaticFrameV;
    [[maybe_unused]] bool res =
        ForEachFrameInUnionStack([&isStaticFrameV]([[maybe_unused]] const void *frame, bool isStaticFrame) {
            isStaticFrameV.push_back(isStaticFrame);
        });
    ASSERT(res);

    /*
        #  Static MethodName: hybrid_stack.ETSGLOBAL::testHybridTest
        #  Static MethodName: hybrid_stack.ETSGLOBAL::staFun0

        *  Dynamic functionName: dyFun1

        #  Static MethodName: hybrid_stack.ETSGLOBAL::staFun1

        *  Dynamic functionName: dyFun2_1
        *  Dynamic functionName: dyFun2_2
        *  Dynamic functionName: dyFun2

        #  Static MethodName: hybrid_stack.ETSGLOBAL::staFun2

        *  Dynamic functionName: dyFun3_3
        *  Dynamic functionName: dyFun3

        #  Static MethodName: hybrid_stack.ETSGLOBAL::staFun3
        #  Static MethodName: hybrid_stack.ETSGLOBAL::staFun3_3
        #  Static MethodName: hybrid_stack.ETSGLOBAL::testAll

        *  Dynamic functionName: main
        *  Dynamic functionName: func_main_0
        *  Dynamic functionName: main
        *  Dynamic functionName:
        *  Dynamic functionName: func_main_0
    */
    std::vector<bool> compType {false, false, false, false, false, true, true,  true, false,
                                false, true,  false, false, false, true, false, true, true};

    return CompFrameType(isStaticFrameV, compType);
}

static bool TestHybridStackLambda()
{
    std::vector<bool> isStaticFrameV;
    [[maybe_unused]] bool res =
        ForEachFrameInUnionStack([&isStaticFrameV]([[maybe_unused]] const void *frame, bool isStaticFrame) {
            isStaticFrameV.push_back(isStaticFrame);
        });
    ASSERT(res);

    /*
      #  Static MethodName: hybrid_stack.ETSGLOBAL::testHybridTest_lambda     isCFrame:1

      *  Dynamic functionName: dyLambdaFun1

      #  Static MethodName: std.interop.js.DynamicFunction::invokeDynamic     isCFrame:0
      #  Static MethodName: std.interop.js.DynamicFunction::invoke0     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::lambda_invoke-2     isCFrame:0
      #  Static MethodName: hybrid_stack.%%lambda-lambda_invoke-2::invoke0     isCFrame:0
      #  Static MethodName: std.core.Lambda0::unsafeCall     isCFrame:0

      *  Dynamic functionName: dyLambdaFun2

      #  Static MethodName: std.interop.js.DynamicFunction::invokeDynamic     isCFrame:0
      #  Static MethodName: std.interop.js.DynamicFunction::invoke0     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::lambda_invoke-3     isCFrame:0
      #  Static MethodName: hybrid_stack.%%lambda-lambda_invoke-3::invoke0     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::testAll     isCFrame:0

      *  Dynamic functionName: main
      *  Dynamic functionName: func_main_0
      *  Dynamic functionName: main
      *  Dynamic functionName:
      *  Dynamic functionName: func_main_0
    */
    std::vector<bool> compType {false, false, false, false, false, true, true, true,  true,
                                true,  false, true,  true,  true,  true, true, false, true};

    return CompFrameType(isStaticFrameV, compType);
}

static bool TestHybridStackESValue()
{
    std::vector<bool> isStaticFrameV;
    [[maybe_unused]] bool res =
        ForEachFrameInUnionStack([&isStaticFrameV]([[maybe_unused]] const void *frame, bool isStaticFrame) {
            isStaticFrameV.push_back(isStaticFrame);
        });
    ASSERT(res);

    /*
      #  Static MethodName: hybrid_stack.ETSGLOBAL::testHybridTest_ESValue     isCFrame:1
      #  Static MethodName: hybrid_stack.ETSGLOBAL::lambda_invoke-2     isCFrame:0
      #  Static MethodName: hybrid_stack.%%lambda-lambda_invoke-2::invoke0     isCFrame:0
      #  Static MethodName: std.core.Lambda0::unsafeCall     isCFrame:0

      *  Dynamic functionName: dy_es1

      #  Static MethodName: std.interop.ESValue::invoke     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::staESFun1     isCFrame:0

      *  Dynamic functionName: dy_es2
      *  Dynamic functionName: dy_es3

      #  Static MethodName: std.interop.ESValue::invoke     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::staESFun2     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::testAll     isCFrame:0

      *  Dynamic functionName: main
      *  Dynamic functionName: func_main_0
      *  Dynamic functionName: main
      *  Dynamic functionName:
      *  Dynamic functionName: func_main_0
    */
    std::vector<bool> compType {false, false, false, false, false, true, true, true, false,
                                false, true,  true,  false, true,  true, true, true};

    return CompFrameType(isStaticFrameV, compType);
}

static bool TestHybridStackSTValue()
{
    std::vector<bool> isStaticFrameV;
    // NOLINTNEXTLINE
    [[maybe_unused]] bool res =
        ForEachFrameInUnionStack([&isStaticFrameV]([[maybe_unused]] const void *frame, bool isStaticFrame) {
            isStaticFrameV.push_back(isStaticFrame);
        });
    ASSERT(res);

    /*
      #  Static MethodName: hybrid_stack.ETSGLOBAL::testHybridTest_STValue     isCFrame:1
      #  Static MethodName: hybrid_stack.ETSGLOBAL::lambda_invoke-6     isCFrame:0
      #  Static MethodName: hybrid_stack.%%lambda-lambda_invoke-6::invoke0     isCFrame:0
      #  Static MethodName: hybrid_stack.ETSGLOBAL::staSTfun1     isCFrame:0

      *  currnt Stack functionName:dy_ST1

      #  Static MethodName: hybrid_stack.ETSGLOBAL::staSTfun2     isCFrame:0

      *  currnt Stack functionName:dy_ST2
      *  currnt Stack functionName:main
      *  currnt Stack functionName:func_main_0
      *  currnt Stack functionName:main
      *  currnt Stack functionName:
      *  currnt Stack functionName:func_main_0
    */
    std::vector<bool> compType {false, false, false, false, false, false, true, false, true, true, true, true};

    return CompFrameType(isStaticFrameV, compType);
}

class HybridStackTsToEtsTest : public EtsInteropTest {
public:
    static bool GetAniEnv(ani_env **env)
    {
        ani_vm *aniVm;
        ani_size res;

        auto status = ANI_GetCreatedVMs(&aniVm, 1U, &res);
        if (status != ANI_OK || res == 0) {
            return false;
        }

        status = aniVm->GetEnv(ANI_VERSION_1, env);
        return status == ANI_OK && *env != nullptr;
    }

    bool RegisterETSFunction(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "hybrid_stack.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }

        std::array methods = {
            ani_native_function {"testHybridTest", nullptr, reinterpret_cast<void *>(TestHybridStack)},
            ani_native_function {"testHybridTestLambda", nullptr, reinterpret_cast<void *>(TestHybridStackLambda)},
            ani_native_function {"testHybridTestESValue", nullptr, reinterpret_cast<void *>(TestHybridStackESValue)},
            ani_native_function {"testHybridTestSTValue", nullptr, reinterpret_cast<void *>(TestHybridStackSTValue)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size()) ==
               ANI_OK;
    }
};

TEST_F(HybridStackTsToEtsTest, check_ts_to_ets_hybridstack)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));

    ASSERT_TRUE(RegisterETSFunction(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("entry_stack.ts"));
}

}  // namespace ark::ets::interop::js::testing
