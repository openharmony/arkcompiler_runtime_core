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

export class Base {
    baseVal: number = 1;
    getCurrent() {
        return this;
    }
}

export class Child extends Base {
    childVal: number = 2;
    getSuper() {
        return super.getCurrent();
    }
}

export let baseFunc = new Base().getCurrent;
export let childFunc = new Child().getSuper;

export let fooBaseObj = {
    foo: baseFunc,
    fooFunc: function() {
        return this.foo();
    }
};
export let fooChildObj = {
    foo: childFunc,
    fooFunc: function() {
        return this.foo();
    }
};

export let arrowFunc = () => {
    return this;
};