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
import {
    Config
} from './config';

export class Mybar {
    static test1() {
        if (Config.isDebug) {
            print('Mybar.test1: Config.isDebug is true');
        } else {
            print('Mybar.test1: Config.isDebug is false');
        }
    }

    test2() {
        if (!Config.isDebug) {
            print('Mybar.test2: Config.isDebug is false');
        } else {
            print('Mybar.test2: Config.isDebug is true');
        }
    }
}
