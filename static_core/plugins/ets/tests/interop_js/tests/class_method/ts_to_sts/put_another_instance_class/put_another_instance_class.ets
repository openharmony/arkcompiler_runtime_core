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
    ChildClass,
    createChildClassFromTs,
    instanceChildClass,
    InterfaceClass,
    createInterfaceClassFromTs,
    instanceInterfaceClass,
    StaticClass,
    createStaticClassFromTs,
    PrivateClass,
    createPrivateClassFromTs,
    instancePrivateClass,
    ChildProtectedClass,
    createChildProtectedClassFromTs,
    instanceChildProtectedClass,
    ProtectedClass,
    createProtectedClassFromTs,
    instanceProtectedClass,
    ChildAbstractClass,
    createChildAbstractClassFromTs,
    instanceChildAbstractClass,
} from 'main_js';

function checkChildClassMethod(): boolean {
    const TSClass = new ChildClass();
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateChildClassFromTs(): boolean {
    const TSClass = createChildClassFromTs();
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkInstanceChildClass(): boolean {
    const TSClass = instanceChildClass;
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkChildClassMethodOwnMethod(): boolean {
    const TSClass = new ChildClass();
    const AnotherClass = new InterfaceClass();

    return TSClass.childMethodCompare(AnotherClass.get) as boolean == false;
}

function checkCreateChildClassFromTsOwnMethod(): boolean {
    const TSClass = createChildClassFromTs();
    const AnotherClass = new InterfaceClass();

    return TSClass.childMethodCompare(AnotherClass.get) as boolean == false;
}

function checkInstanceChildClassOwnMethod(): boolean {
    const TSClass = instanceChildClass;
    const AnotherClass = new InterfaceClass();

    return TSClass.childMethodCompare(AnotherClass.get) as boolean == false;
}

function checkUserClassMethod(): boolean {
    const TSClass = new UserClass();
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateUserClassFromTs(): boolean {
    const TSClass = createUserClassFromTs();
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkInstanceUserClass(): boolean {
    const TSClass = instanceUserClass;
    const AnotherClass = new InterfaceClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkInterfaceClassMethod(): boolean {
    const TSClass = new InterfaceClass();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateInterfaceClassFromTs(): boolean {
    const TSClass = createInterfaceClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkInstanceInterfaceClass(): boolean {
    const TSClass = instanceInterfaceClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkStaticClassMethod(): boolean {
    const TSClass = StaticClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateStaticClassFromTs(): boolean {
    const TSClass = createStaticClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkPrivateClassMethod(): boolean {
    const TSClass = new PrivateClass();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreatePrivateClassFromTs(): boolean {
    const TSClass = createPrivateClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}


function checkInstancePrivateClass(): boolean {
    const TSClass = instancePrivateClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkChildProtectedClassMethod(): boolean {
    const TSClass = new ChildProtectedClass();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateChildProtectedClassFromTs(): boolean {
    const TSClass = createChildProtectedClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}


function checkInstanceChildProtectedClass(): boolean {
    const TSClass = instanceChildProtectedClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkProtectedClassMethod(): boolean {
    const TSClass = new ProtectedClass();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateProtectedClassFromTs(): boolean {
    const TSClass = createProtectedClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}


function checkInstanceProtectedClass(): boolean {
    const TSClass = instanceProtectedClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkAbstractClassMethod(): boolean {
    const TSClass = new ChildAbstractClass();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkCreateAbstractClassFromTs(): boolean {
    const TSClass = createChildAbstractClassFromTs();
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}

function checkInstanceAbstractClass(): boolean {
    const TSClass = instanceChildAbstractClass;
    const AnotherClass = new UserClass();

    return TSClass.compare(AnotherClass.get) as boolean == false;
}