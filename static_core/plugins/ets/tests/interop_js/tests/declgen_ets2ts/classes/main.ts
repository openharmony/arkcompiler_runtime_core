/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

declare const process: { env: Record<string, unknown>; exit: Function };
declare const require: Function;

const PANDA_FILES = 'panda-files';
const BOOT_PANDA_FILES = 'boot-panda-files';
const LOAD_RUNTIMES = 'load-runtimes';

(globalThis as unknown as Record<string, {}>).Panda = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');

const etsOpts = {
	[PANDA_FILES]: process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
	[BOOT_PANDA_FILES]: `${process.env.ARK_ETS_STDLIB_PATH}:${process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH}`,
	[LOAD_RUNTIMES]: 'ets',
};
const createRes = (globalThis as unknown as Record<string, Record<string, Function>>).Panda.createRuntime(etsOpts);
if (!createRes) {
	console.log('Cannot create ETS runtime');
	process.exit(1);
}

(globalThis as unknown as Record<string, {}>).require = require;

import { main } from './classes';

main();
