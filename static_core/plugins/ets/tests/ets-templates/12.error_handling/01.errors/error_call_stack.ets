/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Verify the name, message, and stack format of the stack.
assert: Support column number of stack info, support get error name, message and stack separately.
---*/

function main(): void {
    try {
        throw new Error('error call stack test');
    } catch (e) {
        let err = e as Error;
        assertEQ(typeof err.stack, 'string');
        const stackLines = (err.stack as string).split('\n');
        const stackPattern = new RegExp("\\s*at .*(\\d+):(\\d+)");

        for (const line of stackLines) {
            // The last line is a blank line, skip it
            if (line.trim() === '') {
                continue;
            }
            assertEQ(stackPattern.test(line), true);
        }
    }
}