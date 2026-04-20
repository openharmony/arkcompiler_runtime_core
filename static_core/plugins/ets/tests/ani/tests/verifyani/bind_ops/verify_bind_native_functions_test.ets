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
arktsconfig: arktsconfig.json
---*/

export native function moduleSum(a: int, b: int): int;
export native function moduleOverloaded(a: int): int;
export native function moduleOverloaded(a: long): long;

namespace NativeNs {
    export native function nsSum(a: int, b: int): int;
}

class InstanceBindClass {
    public native foo(): int;
    public native foo(value: int): int;

    public notNative(): int {
        return 0;
    }
}

class StaticBindClass {
    public static native foo(): int;
    public static native foo(value: int): int;
    public native instanceFoo(): int;

    public static notNative(): int {
        return 0;
    }
}

class StaticChildBindClass extends StaticBindClass {}
