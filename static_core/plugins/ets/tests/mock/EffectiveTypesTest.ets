/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

class EffectiveTypes {
    public static native foo(arg: int): int;

    public static native foo(arg: double): int;

    public static native foo(): double;

    public static native fooUnity(arg: double): double;

    public static main(): int {
        let res1: int = EffectiveTypes.foo(1);
        let res2: int = EffectiveTypes.foo(1.0);
        let res3: double = EffectiveTypes.foo();
        let res4: double = EffectiveTypes.fooUnity(1.0);
        if (res1 == 0 && res2 == 1 && res3 == 2.0 && res4 == 3.0) {
            return 0;
        }
        return 19;
    }
}