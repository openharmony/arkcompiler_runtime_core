/**
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

function getWeakMap(): WeakMap<string, string> {
    return new WeakMap<string, string>();
}

class Foo {
    val: WeakMap<string, string> = new WeakMap<string, string>();
    get mapVal(): WeakMap<string, string> {
        return this.val;
    }
    set mapVal(val: WeakMap<string, string>) {
        this.val = val;
    }
}

