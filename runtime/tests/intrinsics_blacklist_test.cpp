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

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "assembly-emitter.h"
#include "runtime/include/runtime.h"

namespace panda::test {

inline std::string separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class IntrinsicsBlacklistTest : public testing::Test {
public:
    ~IntrinsicsBlacklistTest()
    {
        if (runtime_created_) {
            Runtime::Destroy();
        }
    }

    void CreateRuntime(RuntimeOptions &options)
    {
        ASSERT_FALSE(runtime_created_);
        Runtime::Create(options);
        runtime_created_ = true;
    }

private:
    bool runtime_created_ {false};
};

TEST_F(IntrinsicsBlacklistTest, DisableIntrinsic)
{
    auto source = R"(
.record Math <external>
.function i32 Math.absI32(i32 a0) <external>

.function i32 main() {
    movi v0, 42
    call.short Math.absI32, v0, v0
    return
}
)";
    RuntimeOptions options;
    options.SetLoadRuntimes({"core"});
    options.SetIntrinsicsBlacklist({"Math::absI32"});
    auto exec_path = panda::os::file::File::GetExecutablePath();
    std::string panda_std_lib =
        exec_path.Value() + separator() + ".." + separator() + "pandastdlib" + separator() + "arkstdlib.abc";
    options.SetBootPandaFiles({panda_std_lib});
    CreateRuntime(options);
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

    // There are no implementations of Math.absI32 other than intrinsic, so execution should be aborted
    ASSERT_DEATH(Runtime::GetCurrent()->Execute("_GLOBAL::main", {}), "");
}
}  // namespace panda::test
