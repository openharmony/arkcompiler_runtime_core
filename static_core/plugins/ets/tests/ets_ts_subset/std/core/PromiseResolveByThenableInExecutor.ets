/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

 let value = new Object();
 class Thenable implements PromiseLike<Object> {
   then<U, E>(onFulfilled?: (value: Object) => U | PromiseLike<U>, onRejected?: (error: Error) => E | PromiseLike<E>): PromiseLike<Awaited<U | E>> {
     onFulfilled!(value);
     return Promise.resolve(onFulfilled!("resolved")) as PromiseLike<Awaited<U|E>>;
   }
 };

 let thenable = new Thenable();
 Promise.resolve(thenable).then<void, void>((val: Object): void => {
     if (val !== value) {
       console.log('The promise should be fulfilled with the provided value.');
       return;
     }
     console.log('Test passed.');
 }, () => { console.log('The promise should not be rejected.'); });