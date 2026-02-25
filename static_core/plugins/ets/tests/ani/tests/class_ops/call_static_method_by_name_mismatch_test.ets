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
/*---
file_name: ani_test_class_ops_call_static_method_by_name_mismatch
---*/

class TypeProvider {
    static retBool(): boolean { return true; }
    static retByte(): byte { return 1; }
    static retChar(): char { return c'a'; }
    static retShort(): short { return 2; }
    static retInt(): int { return 3; }
    static retLong(): long { return 4; }
    static retFloat(): float { return 5.0; }
    static retDouble(): double { return 6.0; }
    static retRef(): Array<int> { return new Array<int>(100); }
    static retVoid(): void {}
}