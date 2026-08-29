/*
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

/*---
desc: §6 Assignment-like Contexts — function, method, and lambda return contexts accept widening numeric conversions.
---*/

function widenedFunctionReturn(): long {
    let source: int = 17;
    return source;
}

class ReturnContextHolder {
    public widenedMethodReturn(): double {
        let source: int = 23;
        return source;
    }
}

function main(): void {
    let holder = new ReturnContextHolder();
    let widenedLambda: () => float = (): float => {
        let source: int = 29;
        return source;
    };

    arktest.assertEQ(widenedFunctionReturn(), 17 as long);
    arktest.assertEQ(holder.widenedMethodReturn(), 23.0);
    arktest.assertEQ(widenedLambda(), 29.0f);
}
