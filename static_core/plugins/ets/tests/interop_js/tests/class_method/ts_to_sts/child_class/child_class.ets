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

import {
    ChildClass,
    UserClass,
    createChildClassFromTs,
    instanceChildClass,
} from 'main_js';

function checkChildClassMethod(): boolean {
    const TSClass = new ChildClass();

    return TSClass.compare(TSClass.get) as boolean;
}

function checkCreateChildClassFromTs(): boolean {
    const TSClass = createChildClassFromTs();

    return TSClass.compare(TSClass.get) as boolean;
}


function checkInstanceChildClass(): boolean {
    const TSClass = instanceChildClass;

    return TSClass.compare(TSClass.get) as boolean;
}

function checkChildClassMethodOwnMethod(): boolean {
    const TSClass = new ChildClass();
    const ParentClass = new UserClass();

    return TSClass.childMethodCompare(ParentClass.get) as boolean;
}

function checkCreateChildClassFromTsOwnMethod(): boolean {
    const TSClass = createChildClassFromTs();
    const ParentClass = new UserClass();
     
    return TSClass.childMethodCompare(ParentClass.get) as boolean;
}


function checkInstanceChildClassOwnMethod(): boolean {
    const TSClass = instanceChildClass;
    const ParentClass = new UserClass();

    return TSClass.childMethodCompare(ParentClass.get) as boolean;
}