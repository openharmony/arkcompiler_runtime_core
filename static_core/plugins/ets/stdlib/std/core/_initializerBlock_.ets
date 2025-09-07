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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    // 5 lower bits stores kind's id
    TypeKindMask = ((1 << 6) - 1).toByte();

    ObjectType = Type.of(new Object()) as ClassType;
    console = Console.getInstance();
    coroutine = new Coroutine();

    MAX_REGULAR_INTEGER = new BigInt(Number.MAX_SAFE_INTEGER)
    FAST_TO_STRING_HANDLER_MAP = () => {
        let result = new ToStringHandlerMap
        result[2] = (instance: BigInt):string => { return instance.toStringAsBinary() }
        result[4] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(4) }
        result[8] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(8) }
        result[10] = (instance: BigInt):string => { return instance.toStringAsDecimal() }
        result[16] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(16) }
        result[32] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(32) }
        return result
    }()
}
