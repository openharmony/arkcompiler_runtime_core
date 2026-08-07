/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { jsObj, JsClass, jsArr, add, asyncGreet } from 'testDecl';

// EAWorker(true) - GetProperty(name)
function checkEAWorkerWithInteropGetProperty(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        return jsObj.name === 'hello';
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - SetProperty(name)
function checkEAWorkerWithInteropSetProperty(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        jsObj.value = 99;
        return jsObj.value === 99;
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - InvokeMethod(name)
function checkEAWorkerWithInteropInvokeMethod(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        return jsObj.greet('worker') === 'hello, worker';
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - Invoke (function call)
function checkEAWorkerWithInteropInvokeFunction(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        return add(10, 20) === 30;
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - Instantiate
function checkEAWorkerWithInteropInstantiate(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        let instance = new JsClass('from_worker');
        return instance.name === 'from_worker';
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - IsInstanceOf
function checkEAWorkerWithInteropIsInstanceOf(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        let instance = new JsClass('test');
        return instance instanceof JsClass;
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - HasOwnProperty
function checkEAWorkerWithInteropHasOwnProperty(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let callback = ():boolean => {
        return JSRuntime.hasOwnProperty(jsObj, 'name');
    };
    let res = eaw.run<boolean>(callback).Await();
    eaw.join();
    return res;
}

// EAWorker(true) - Invoke (async function)
function checkEAWorkerWithInteropAsyncFunction(): boolean {
    let eaw = new EAWorker(true);
    eaw.start();
    let job = new CompletableJob<boolean>();
    let callback = () => {
        let p = asyncGreet('worker') as Promise<Object>;
        p.then((x: Object): void => {
            job.finish(x as string === 'hello, worker');
        });
    };
    eaw.run<void>(callback);
    eaw.join();
    return job.Await();
}

// EAWorker() - GetProperty(name)
function checkEAWorkerWithoutInteropGetProperty(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let name = jsObj.name;
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - SetProperty(name)
function checkEAWorkerWithoutInteropSetProperty(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            jsObj.value = 1;
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - GetProperty(index)
function checkEAWorkerWithoutInteropGetPropertyByIndex(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let val = JSRuntime.getElementObject(jsArr, 0);
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - InvokeMethod(name)
function checkEAWorkerWithoutInteropInvokeMethod(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            jsObj.greet('fail');
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - Invoke (function call)
function checkEAWorkerWithoutInteropInvokeFunction(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            add(1, 2);
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - Instantiate
function checkEAWorkerWithoutInteropInstantiate(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let instance = new JsClass('fail');
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - IsInstanceOf
function checkEAWorkerWithoutInteropIsInstanceOf(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let instance = new JsClass('test');
            let r = instance instanceof JsClass;
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - HasOwnProperty
function checkEAWorkerWithoutInteropHasOwnProperty(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let r = JSRuntime.hasOwnProperty(jsObj, 'name');
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}

// EAWorker() - Invoke (async function)
function checkEAWorkerWithoutInteropAsyncFunction(): boolean {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            asyncGreet('fail');
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        arktest.assertTrue(e.toString().indexOf('NoInteropContextError') !== -1);
        res = true;
        eaw.join();
    }
    return res;
}
