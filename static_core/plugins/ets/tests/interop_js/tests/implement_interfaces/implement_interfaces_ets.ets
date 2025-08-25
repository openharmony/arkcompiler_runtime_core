/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
//NOTE(andreypetukhov) enable after fixibng #12808

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/implement_interfaces/arkts_class')
let arkTestClass = module.getProperty('ArkTestClass');

function arktsClassGetProperty(): boolean {
   return arkTestClass.getProperty('name').toString() === 'TestArkTSClass' &&
      arkTestClass.getProperty('id').toNumber() === 555;
}

function arktsClassUseStringFn():boolean{
  return arkTestClass.getProperty('greet').invoke().toString() === 'This is TestArkTSClass';
}

function arktsClassUseNumberFn():boolean{
  return arkTestClass.invokeMethod('myId').toNumber() === 555;
}
// function arktsClassUseStringFn():boolean{

//   return ArkTestClass.greet() == 'This is TestArkTSClass'
// }

// function arktsClassUseNumberFn():boolean{
//   return ArkTestClass.myId() as number == 555
// }
