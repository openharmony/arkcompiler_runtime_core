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
    UserClass,
    createUserClassFromTs,
    instanceUserClass,
} from 'main_js';

function checkUserClassMethod(): boolean {
    const TSClass = new UserClass();

    return TSClass.compare(TSClass.get) as boolean;
}

function checkCreateUserClassFromTs(): boolean {
    const TSClass = createUserClassFromTs();

    return TSClass.compare(TSClass.get) as boolean;
}


function checkInstanceUserClass(): boolean {
    const TSClass = instanceUserClass;

    return TSClass.compare(TSClass.get) as boolean;
}
