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

package unit.test;

class B {}

function foo(val: Any): boolean {
    let a = ESValue.wrap(val);
    let b = ESValue.wrap(val);
    let c = a.getProperty('a');
    let d = ESValue.wrap(new B());
    return !a.isStaticObject() && !b.isStaticObject() && ESValue.areStrictlyEqual(a, b) && c.isNumber() && d.isStaticObject();
}
