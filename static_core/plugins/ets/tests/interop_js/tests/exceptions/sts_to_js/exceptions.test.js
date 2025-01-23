
/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const stsVm = require('lib/module/ets_interop_js_napi');

const throwErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorFromSts');
const throwErrorWithCauseFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorWithCauseFromSts');
const throwCustomErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwCustomErrorFromSts');

const throwExceptionFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwExceptionFromSts');
const throwExceptionWithCauseFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwExceptionWithCauseFromSts');
const throwCustomExceptionFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwCustomExceptionFromSts');

module.exports = {
    throwErrorFromSts,
    throwErrorWithCauseFromSts,
    throwCustomErrorFromSts,
    throwExceptionFromSts,
    throwExceptionWithCauseFromSts,
    throwCustomExceptionFromSts,
};
