'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
Object.defineProperty(exports, '__esModule', { value: true });
exports.tsExponential = exports.tsFloat = exports.tsByte = exports.tsIntString = 
exports.tsHexadecimal = exports.tsBinary = exports.tsBigInt = exports.tsInt = 
exports.systemBinary = exports.systemHexadecimal = void 0;
let buffer = new ArrayBuffer(1);
let dataView = new DataView(buffer);
dataView.setUint8(0, 255);
exports.systemHexadecimal = 16;
exports.systemBinary = 2;
exports.tsInt = 1;
exports.tsBigInt = 1n;
exports.tsBinary = (256 >> 1).toString(exports.systemBinary);
exports.tsHexadecimal = (300).toString(exports.systemHexadecimal);
exports.tsIntString = '1';
exports.tsByte = dataView;
exports.tsFloat = 3.14;
exports.tsExponential = exports.tsInt.toExponential();
