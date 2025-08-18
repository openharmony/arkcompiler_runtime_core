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

// this will be compiled to <cctor>
// and be executed when the module is initing
let module = ESValue.load(`../../plugins/ets/tests/interop_js/tests/test_init_module/test_init_module_dynamic`);
let initModuleTest = module.getProperty(`initModuleTest`);

let magicToSet = initModuleTest.getProperty(`initMagic`).toString();
initModuleTest.setProperty(`isInited`, magicToSet);

export class SomeClass {
    foo: number = 0x1234;
    static {
        // this will be executed when the class is initing
        let shouldNotInitClassTest = module.getProperty(`shouldNotInitClassTest`);
        let magicToSet = shouldNotInitClassTest.getProperty(`initMagic`).toString();
        shouldNotInitClassTest.setProperty(`isInited`, magicToSet);
    }
}