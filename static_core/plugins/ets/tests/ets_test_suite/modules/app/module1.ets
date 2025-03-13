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

import * as foo from "@foo/foo"
import * as bar from "@bar/bar"
import * as module2 from "./module2"
import * as module3 from "./module3"
import { Ability } from "@spawn/ability";

export class Dummy { }

export function dummy(): Dummy { return new Dummy() }

function getCurrentLinker() {
    return Class.ofCaller().getLinker();
}

function testLazyLoading() {
    const appLinker = getCurrentLinker();

    // disabled as runtime verication is enabled
    const verifierEnabled: boolean = true;

    if (!verifierEnabled) {
        assertEQ(appLinker.findLoadedClass("@bar.bar.ETSGLOBAL"), undefined, "module must be loaded lazily");
    }

    assertFalse(bar.ClassLoadingChecker_init, "`@bar.bar.ClassLoadingChecker` must be not initialized");
    assertNE(appLinker.findLoadedClass("@bar.bar.ETSGLOBAL"), undefined,
        "module must be loaded after accessing `bar.ClassLoadingChecker_init`");

    if (!verifierEnabled) {
        assertEQ(appLinker.findLoadedClass("@bar.bar.ClassLoadingChecker"), undefined,
            "`@bar.bar.ClassLoadingChecker` must not be loaded yet");
    }

    const instance = new bar.ClassLoadingChecker();
    assertEQ(appLinker.findLoadedClass("@bar.bar.ClassLoadingChecker"), Class.of(instance));
    assertTrue(bar.ClassLoadingChecker_init, "`@bar.bar.ClassLoadingChecker` must be initialized");
}

function triggerGC() {
    // Check that garbage collection does not break user-defined RuntimeLinker
    expectNoThrow((): void throws => {
        const gcId = GC.startGC(GC.FULL_CAUSE, GC.IN_PLACE_MODE);
        if(gcId > 0) {
        GC.waitForFinishGC(gcId);
    }
});
}

function getNameOfClass(o: Object): string {
    return Class.of(o).getName();
}

function getMethodByName(classType: ClassType, name: string): Method | undefined {
    const numOfMethods = classType.getMethodsNum();
    for (let i = 0; i < numOfMethods; i++) {
        const method = classType.getMethod(i);
        if (method.getName() == name) {
            return method;
        }
    }
    return undefined;
}

function testStdCoreType() {
    const obj = foo.dummy();
    assertTrue(obj instanceof foo.Dummy);
    const objName = getNameOfClass(obj);
    assertEQ(objName, "@foo.foo.Dummy");

    const objType = Type.of(obj);
    assertEQ(objType.getName(), objName, "names obtained via std.core.Type and std.core.Class must match");

    const classType = objType as ClassType;
    const objClass = Class.of(obj);
    assertEQ(classType.getName(), objClass.getName(), "Class objects must match");
    const classTypeFromLinker = objClass.getLinker().getType(objClass.getName());
    assertNE(classTypeFromLinker, undefined, "RuntimeLinker must correctly resolve class type '" + objClass.getName() + "'");
    assertTrue(classType.equals(classTypeFromLinker!), "RuntimeLinker must correctly resolve class type '" + objClass.getName() + "'");

    const method = getMethodByName(classType, "foo");
    assertNE(method, undefined, "method 'foo' must be in class @foo.foo.Dummy");
    assertTrue(method!.getType().hasReceiver());
    assertEQ((method!.getType().getReceiverType() as ClassType).getName(), classType.getName(), "method 'foo' must be declared in interface 'Base'");
}

function testStdCoreProxy() {
    const map = new Map<string, string>();

    class Handler extends DefaultProxyHandler<Map<string, string>> { };
    const mapProxy = Proxy.create(map, new Handler());

    assertTrue(map === mapProxy, "std.core.Proxy must work correctly in application context");
}

function testArrayClass() {
    const arr = new foo.Dummy[0]
    const arrClass = Class.of(arr);
    assertNE(arrClass, undefined);

    const element = foo.dummy();
    const elementClass = Class.of(element);
    assertNE(elementClass, undefined);
    assertEQ(arrClass.getLinker(), elementClass.getLinker());
}

function testNames() {
    triggerGC();

    assertEQ(Class.current().getName(), "@app.module1.ETSGLOBAL")

    testLazyLoading();

    triggerGC();

    assertTrue(bar.dummy() instanceof bar.Dummy);
    assertEQ(getNameOfClass(bar.dummy()), "@bar.bar.Dummy");

    assertTrue(foo.dummy() instanceof foo.Dummy);
    assertEQ(getNameOfClass(foo.dummy()), "@foo.foo.Dummy");

    assertTrue(dummy() instanceof Dummy);
    assertEQ(getNameOfClass(dummy()), "@app.module1.Dummy");

    assertTrue(module2.dummy() instanceof module2.Dummy);
    assertEQ(getNameOfClass(module2.dummy()), "@app.module2.Dummy");

    assertTrue(module3.dummy() instanceof module3.Dummy);
    assertEQ(getNameOfClass(module3.dummy()), "@app.module3.Dummy");

    testStdCoreType();
    testStdCoreProxy();

    testArrayClass();

    triggerGC();
}

class MainAbility implements Ability {
    override invoke() {
        testNames();
    }
}
