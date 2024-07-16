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
exports.SimpleObject = exports.handler = exports.notFound = exports.tsInt = void 0;
exports.tsInt = 1;
exports.notFound = 'Not found';
exports.handler = {
    get: function (target, property) {
        if (!isNaN(property)) {
            return target.items[property];
        }
        if (typeof target[property] === 'function') {
            return target[property]();
        }
        if (target[property]) {
            return target[property];
        }
        return exports.notFound;
    },
    set: function (target, property, value) {
        if (!isNaN(property)) {
            target.items[property] = value;
            return true;
        }
        target[property] = value;
        return true;
    }
};
exports.SimpleObject = {
    int: exports.tsInt,
    items: [],
    add: function () { return exports.tsInt + exports.tsInt; },
};
