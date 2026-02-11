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
desc: handleAll pattern wrapping actions with error handling callback.
---*/

function handleAll(
    actions: () => void,
    handling_actions: () => void)
{
    try {
        actions()
    }
    catch (x) { // Type of x is Error
        handling_actions()
    }
}

function main(): void {
    let handled = false

    handleAll(
        (): void => { throw new Error("action failed") },
        (): void => { handled = true }
    )

    arktest.assertTrue(handled)
}
