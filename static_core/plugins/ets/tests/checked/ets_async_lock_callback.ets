/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

//! CHECKER      Check for method->isStatic assert
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_async_lock_callback.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_async_lock_callback.Test::wrapper,.*,COMPILED/

type AsyncLockCallback<T> = () => T | Promise<T>;

class Test {
    public lockAsync<T>(callback: AsyncLockCallback<T>): Promise<T> {
        return launch this.wrapper<T>(callback) as Promise<T>;
    }

    private wrapper<T>(callback: AsyncLockCallback<T>): T | Promise<T> {
        let result = callback();
        return result;
    }
}

function main(): int {
    let test = new Test();
    for (let i = 0; i < 100; i++) {
        let promise = test.lockAsync((): int => {
            console.log("callback");
            return 1;
        })
        await promise;
    }
    return 0;
}
