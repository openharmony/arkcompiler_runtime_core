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

class Test {
    public name: string = '';
    public data1: number = 0;
    public data2: number = 0;
}

type RequiredType = Required<Test>;
type OptionalType = Test;

let optionalTestInEts: OptionalType = new Test();
optionalTestInEts.name = 'optional';
optionalTestInEts.data1 = 1;
optionalTestInEts.data2 = 2;

let requiredTestInEts: RequiredType = {
    name: 'required',
    data1: 3,
    data2: 4
}

function checkOptionalTypeDataInEts(data: OptionalType, data1: number, data2: number): boolean {
    return data.data1 === data1 && data.data2 === data2;
}

function checkRequiredTypeDataInEts(data: RequiredType, data1: number, data2: number): boolean {
    return data.data1 === data1 && data.data2 === data2;
}
