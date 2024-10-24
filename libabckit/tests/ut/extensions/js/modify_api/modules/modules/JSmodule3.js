/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
export {
    newImportedFuncFromModule4
}
from './JSmodule4';

export default function regularDefaultImportFunc1FromModule3() {
    print('regular default import func1 from module3');
}
export function sameFuncInDifferentModules() {
    print('the same func from module3');
}
export function newImportedFunc() {
    print('new imported func from module3');
}
export function newExportedFunc() {
    print('new exported func from module3');
}
