/*
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

function cname(name: string) { return "runtime_linker." + name }

class Other {
    static getCallerOfItself() {
        return Class.current();
    }
}

class A { }
class B extends A { }
interface I { }

let LoadTarget_init = false;
class LoadTarget {
    private static s = (() => { LoadTarget_init = true; return 1; })();
}

let InitializeTarget_init = false;
class InitializeTarget {
    private static s = (() => { InitializeTarget_init = true; return 1; })();
}

let CreateInstanceTarget_init = false;
class CreateInstanceTarget {
    static s = (() => { CreateInstanceTarget_init = true; return 1; })();
}

let HasNoDefaultConstructor_init = false;
class HasNoDefaultConstructor {
    private static s = (() => { HasNoDefaultConstructor_init = true; return 1; })();
    constructor(x: int) { this.x = x }
    x: int
}

class Test {
    static testRoots() {
        let obj = new Object();
        let objCls = Class.of(obj);
        let clsCls = Class.of(objCls);
        assertEQ(Class.of(clsCls), clsCls);
        assertEQ(objCls.getDescriptor(), "Lstd/core/Object;");
        assertEQ(clsCls.getDescriptor(), "Lstd/core/Class;");
        assertEQ(objCls.getSuper(), undefined, "std.core.Object must have no super class");
        assertEQ(clsCls.getSuper(), objCls, "std.core.Class must extend std.core.Object");
    }

    static testClassProps() {
        const objcls = Class.of(new Object());
        const acls = Class.of(new A());
        const bcls = Class.of(new B());
        assertEQ(objcls.getName(), "std.core.Object");
        assertEQ(objcls.getSuper(), undefined);
        assertEQ(acls.getName(), cname("A"));
        assertEQ(acls.getSuper(), objcls);
        assertEQ(bcls.getName(), cname("B"));
        assertEQ(bcls.getSuper(), acls);
    }

    static testCaller() {
        let caller = Class.ofCaller();
        assertEQ(caller.getName(), cname("Test"));
        assertNE(Other.getCallerOfItself(), caller);
    }

    static testRuntimeLinkerProperties() {
        // Check all booted Class refer to the same RuntimeLinker
        let objectLinker = Class.of(new Object()).getLinker();
        assertNE(objectLinker, undefined);
        assertEQ(objectLinker, getBootRuntimeLinker());
        let doubleLinker = Class.of(new Double(1.0)).getLinker();
        assertEQ(objectLinker, doubleLinker);
        // Check idempotence
        let objectLinker2 = Class.of(new Object()).getLinker();
        assertEQ(objectLinker, objectLinker2);
    }

    static testLoadClass() {
        let linker = Class.ofCaller().getLinker();
        let clsName = cname("LoadTarget");

        assertFalse(LoadTarget_init, "LoadTarget class must be lazily initialized");
        let cls = linker.loadClass(clsName, false);
        assertEQ(cls.getName(), clsName);
        assertFalse(LoadTarget_init, "LoadTarget class must be loaded without initialization");

        let cls2 = linker.loadClass(clsName, true);
        assertTrue(LoadTarget_init, "already loaded LoadTarget class must be initialized");
        assertEQ(cls2, cls);

        expectThrow(() => { linker.loadClass(cname("does.not.exist"), false) },
            (e) => e instanceof LinkerClassNotFoundError
        );
    }

    static testInitialize() {
        let linker = Class.ofCaller().getLinker();
        let clsName = cname("InitializeTarget");

        assertFalse(InitializeTarget_init, "InitializeTarget class must be lazily initialized");
        let cls = linker.loadClass(clsName, false);
        assertFalse(InitializeTarget_init, "InitializeTarget class must be loaded without initialization");
        cls.initialize();
        assertTrue(InitializeTarget_init, "InitializeTarget class must be initialized explicitly");
    }

    static testCreateInstance() {
        let linker = Class.ofCaller().getLinker();

        let acls = Class.of(new A());
        assertEQ(Class.of(acls.createInstance()), acls);

        assertFalse(CreateInstanceTarget_init, "CreateInstanceTarget class must be lazily initialized");
        linker.loadClass(cname("CreateInstanceTarget"), false).createInstance();
        assertTrue(CreateInstanceTarget_init, "CreateInstanceTarget class must be initialized after first instantiation");

        expectThrow(() => { linker.loadClass(cname("I"), false).createInstance() },
            (e) => e instanceof Error
        );
        assertFalse(HasNoDefaultConstructor_init);
        expectThrow(() => { linker.loadClass(cname("HasNoDefaultConstructor"), false).createInstance() },
            (e) => e instanceof Error
        );
        assertFalse(HasNoDefaultConstructor_init);
    }

    static test() {
        Test.testClassProps();
        Test.testRoots();
        Test.testCaller();
        Test.testRuntimeLinkerProperties();
        Test.testLoadClass();
        Test.testInitialize();
        Test.testCreateInstance();
    }
}

function main() {
    Test.test();
}