/*
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

class A { }

class DummyRuntimeLinker extends RuntimeLinker {
    private parent: RuntimeLinker | undefined = undefined;
    private placeholderClass: Class;

    public constructor(placeholderClass: Class, parent: RuntimeLinker | undefined = undefined) {
        this.parent = parent;
        this.placeholderClass = placeholderClass;
    }

    public override findAndLoadClass(clsName: string, init: boolean): Class {
        if (this.parent) {
            const optClass = this.parent!.findAndLoadClass(clsName, init);
            if (optClass) {
                return optClass;
            }
        }
        return this.placeholderClass;
    }
}

class Test {
    static stdCoreObjectName = "std.core.Object";
    static additionalClass1Name = "additional1.AdditionalClass1";
    static additionalClass2Name = "additional2.AdditionalClass2";

    static testDummyLinker() {
        const placeholderClass = Class.of(new A());
        const testLoadObject = (runtimeLinker: RuntimeLinker) => {
            const cls = runtimeLinker.loadClass(Test.stdCoreObjectName);
            assertEQ(cls, placeholderClass, "dummy linker must always return class A");
        };

        const dummyLinker = new DummyRuntimeLinker(placeholderClass);
        testLoadObject(dummyLinker);

        const abcLinker = new AbcRuntimeLinker(dummyLinker, []);
        testLoadObject(abcLinker);
    }

    private static testEmptyLinker(linker: RuntimeLinker) {
        const cls1 = linker.loadClass(Test.stdCoreObjectName);
        assertEQ(cls1, Class.of(new Object()));
        // Object must be returned from BootRuntimeLinker
        assertEQ(cls1.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");
        const cls2 = getBootRuntimeLinker().loadClass(Test.stdCoreObjectName);
        assertEQ(cls1, cls2, "loadClass must be idempotent");
    }

    static testEmptyAbcRuntimeLinker() {
        const linker = new AbcRuntimeLinker(undefined, []);
        Test.testEmptyLinker(linker);
    }

    static testEmptyMemoryRuntimeLinker() {
        const linker = new MemoryRuntimeLinker(undefined, []);
        Test.testEmptyLinker(linker);
    }

    private static native loadFile(path: string): byte[] | undefined;

    private static getAdditionalFilesList(): string[] {
        const abcFiles = ProcessManager.getEnvironmentVar("ADDITIONAL_ABC_FILES");
        assertLT(0, abcFiles.length, "this test must have additional abc files")
        return abcFiles.split(":");
    }

    private static loadRawFiles(paths: string[]): byte[][] {
        const rawFiles = new byte[paths.length as int][0];
        for (let i = 0; i < rawFiles.length; i++) {
            const raw = Test.loadFile(paths[i]);
            assertNE(raw, undefined, "failed to load file " + paths[i]);
            rawFiles[i] = raw!;
        }
        return rawFiles;
    }

    private static testOnAdditionalFiles(linker: RuntimeLinker) {
        const objectClass = linker.loadClass(Test.stdCoreObjectName);
        assertEQ(objectClass, Class.of(new Object()));
        assertEQ(objectClass.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");

        // Load class from "additional1.ets"
        const cls1 = linker.loadClass(Test.additionalClass1Name, true);
        assertNE(cls1, undefined, "AdditionalClass1 must be found");
        assertEQ(cls1.getLinker(), linker, "AdditionalClass1 must be loaded by custom linker");

        // Load class from "additional2.ets"
        // As side effect, this must load "additional3.ets" and execute code from it
        const cls2 = linker.loadClass(Test.additionalClass2Name, true);
        assertNE(cls2, undefined, "AdditionalClass2 must be found");
        assertEQ(cls2.getLinker(), linker, "AdditionalClass2 must be loaded by custom linker");

        // Check that RuntimeLinker loads into an isolated context
        const abcLinker2 = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());
        // Load firstly "AdditionalClass2", which must trigger load of "AdditionalClass1"
        const otherLinkerClass2 = abcLinker2.loadClass(Test.additionalClass2Name, true);
        assertNE(otherLinkerClass2, undefined);
        assertEQ(otherLinkerClass2.getLinker(), abcLinker2, "linker contexts must be isolated");
        assertNE(cls2, otherLinkerClass2, "classes from different contexts must not conflict");
        const otherLinkerClass1 = abcLinker2.loadClass(Test.additionalClass1Name, true);
        assertNE(otherLinkerClass1, undefined);
        assertEQ(otherLinkerClass1.getLinker(), abcLinker2, "linker contexts must be isolated");
        assertNE(cls1, otherLinkerClass1, "classes from different contexts must not conflict");
    }

    static testAbcRuntimeLinker() {
        const linker = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());
        Test.testOnAdditionalFiles(linker);
    }

    static testMemoryRuntimeLinker() {
        const linker = new MemoryRuntimeLinker(undefined, Test.loadRawFiles(Test.getAdditionalFilesList()));
        Test.testOnAdditionalFiles(linker);
    }

    static testCustomAbcRuntimeLinker() {
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Array.from(Test.getAdditionalFilesList()).slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path: string, idx: number) => { abcFiles[idx as int] = path; });
        // Create linkers
        const placeholderClass = Class.of(new A());
        const baseLinker = new AbcRuntimeLinker(undefined, abcFiles);
        const linker = new DummyRuntimeLinker(placeholderClass, baseLinker);

        // Load firstly "AdditionalClass2", which must trigger load of "AdditionalClass1"
        // - "A" must be loaded instead of "AdditionalClass1"
        const cls2 = linker.loadClass(Test.additionalClass2Name, true);
        assertNE(cls2, undefined);
        assertEQ(cls2.getLinker(), baseLinker);
        const cls1 = linker.loadClass(Test.additionalClass1Name, true);
        assertEQ(cls1, placeholderClass);
        assertEQ(cls1.getLinker(), getBootRuntimeLinker());
    }

    static testClassInitializationFail() {
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Array.from(Test.getAdditionalFilesList()).slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path: string, idx: number) => { abcFiles[idx as int] = path; });
        // Create linker
        const linker = new AbcRuntimeLinker(undefined, abcFiles);

        // Load "AdditionalClass2" without initialization
        const cls2 = linker.loadClass(Test.additionalClass2Name, false);
        assertNE(cls2, undefined);
        assertEQ(cls2.getLinker(), linker);
        // Trigger class initialization, which will not find "AdditionalClass1"
        expectThrow(() => { linker.loadClass(Test.additionalClass2Name, true); },
            (e) => e instanceof LinkerUnresolvedClassError
        );
    }

    private static triggerGC() {
        expectNoThrow((): void throws => {
            GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE, GC.IN_PLACE_MODE));
        });
    }

    // Do not inline this method, otherwise `AbcRuntimeLinker` will survive and test will fail
    private static createWeakRef(): WeakRef<AbcRuntimeLinker> {
        const abcLinker = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());

        const objectClass = abcLinker.loadClass(Test.stdCoreObjectName);
        assertEQ(objectClass, Class.of(new Object()));
        assertEQ(objectClass.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");

        // Load class from "additional1.ets"
        const cls1 = abcLinker.loadClass(Test.additionalClass1Name, true);
        assertNE(cls1, undefined, "AdditionalClass1 must be found");
        assertEQ(cls1.getLinker(), abcLinker, "AdditionalClass1 must be loaded by custom linker");

        Test.triggerGC();

        // Load class from "additional2.ets"
        // As side effect, this must load "additional3.ets" and execute code from it
        const cls2 = abcLinker.loadClass(Test.additionalClass2Name, true);
        assertNE(cls2, undefined, "AdditionalClass2 must be found");
        assertEQ(cls2.getLinker(), abcLinker, "AdditionalClass2 must be loaded by custom linker");

        return new WeakRef<AbcRuntimeLinker>(abcLinker);
    }

    static testAbcRuntimeLinkerUnload() {
        const abcLinkerRef = Test.createWeakRef();
        Test.triggerGC();
        assertEQ(abcLinkerRef.deref(), undefined, "AbcRuntimeLinker must be collected");
    }

    private static testAddAbcFiles(linker: AbcRuntimeLinker, newFiles: string[]) {
        // Load "AdditionalClass2" without initialization
        const cls2 = linker.loadClass(Test.additionalClass2Name, false);
        assertNE(cls2, undefined);
        assertEQ(cls2.getLinker(), linker);

        // Add the missing file
        linker.addAbcFiles(newFiles);

        // Trigger class initialization, which must not fail
        linker.loadClass(Test.additionalClass2Name, true);
    }

    static testRuntimeLinkerAddAbcFiles() {
        const allAbcFiles = Test.getAdditionalFilesList();
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Array.from(allAbcFiles).slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path: string, idx: number) => { abcFiles[idx as int] = path; });
        // Create linker
        const linker = new AbcRuntimeLinker(undefined, abcFiles);
        Test.testAddAbcFiles(linker, [allAbcFiles[0]]);
    }

    static testMemoryRuntimeLinkerAddAbcFiles() {
        const allAbcFiles = Test.getAdditionalFilesList();
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Array.from(allAbcFiles).slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path: string, idx: number) => { abcFiles[idx as int] = path; });
        // Create linker
        const linker = new MemoryRuntimeLinker(undefined, Test.loadRawFiles(abcFiles));
        Test.testAddAbcFiles(linker, [allAbcFiles[0]]);
    }

    private static addSingleAbcFile(linker: AbcRuntimeLinker, path: string): void {
        linker.addAbcFiles([path]);
    }

    static testConcurrentAddAbcFiles() {
        const linker = new AbcRuntimeLinker(undefined, []);
        const allAbcFiles = Test.getAdditionalFilesList();
        const promises = new Array<Promise<void>>();
        // Add all files except "additional1.ets"
        for (let i = 1; i < allAbcFiles.length; i++) {
            promises.push(launch Test.addSingleAbcFile(linker, allAbcFiles[i]));
        }
        // Try to load a class from "additional1.ets"
        expectThrow(() => { linker.loadClass(Test.additionalClass1Name, true); },
            (e) => e instanceof LinkerClassNotFoundError
        );
        promises.forEach((p: Promise<void>): void => { await p; });
        Test.addSingleAbcFile(linker, allAbcFiles[0]);
        // Now class must be found
        expectNoThrow(() => { linker.loadClass(Test.additionalClass1Name, true); });
    }

    static testAbcRuntimeLinkerLoadErrorPath() {
        const path: string[] = ["null"];
        expectThrow(() => { new AbcRuntimeLinker(undefined, path) },
            (e) => e instanceof AbcFileNotFoundError
        );
    }

    static test() {
        Test.testDummyLinker();
        Test.testEmptyAbcRuntimeLinker();
        Test.testEmptyMemoryRuntimeLinker();
        Test.testAbcRuntimeLinker();
        Test.testMemoryRuntimeLinker();
        Test.testClassInitializationFail();
        Test.testAbcRuntimeLinkerUnload();
        Test.testAbcRuntimeLinkerLoadErrorPath();
        Test.testRuntimeLinkerAddAbcFiles();
        Test.testMemoryRuntimeLinkerAddAbcFiles();
        Test.testConcurrentAddAbcFiles();
    }
}

function main() {
    loadLibrary("runtime_linker_extensions-shared-lib");
    Test.test();
}
