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

package std.core;

import { AbcFile } from "arkruntime"

/**
 * @class ABC files based RuntimeLinker.
 */
export class AbcRuntimeLinker extends RuntimeLinker {
    private parentLinker: RuntimeLinker
    private abcFiles: FixedArray<AbcFile | undefined>
    private readonly sharedLibraryLoaders: FixedArray<AbcRuntimeLinker | undefined>

    public constructor(parentLinker: RuntimeLinker | undefined, paths: string[]) {
        super()
        this.parentLinker = parentLinker ?? getBootRuntimeLinker()
        this.abcFiles = new FixedArray<AbcFile | undefined>(paths.length, undefined)
        this.sharedLibraryLoaders = []
        for (let i = 0; i < paths.length; i++) {
            this.abcFiles[i] = AbcFile.loadAbcFile(this, paths[i])
        }
    }

    public constructor(parentLinker: RuntimeLinker | undefined, paths: string[], sharedLibraryLoaders: AbcRuntimeLinker[]) {
        super()
        this.parentLinker = parentLinker ?? getBootRuntimeLinker()
        this.abcFiles = new FixedArray<AbcFile | undefined>(paths.length, undefined)
        this.sharedLibraryLoaders = new FixedArray<AbcRuntimeLinker | undefined>(sharedLibraryLoaders.length, undefined)
        for (let i = 0; i < sharedLibraryLoaders.length; i++) {
            this.sharedLibraryLoaders[i] = sharedLibraryLoaders[i]
        }
        for (let i = 0; i < paths.length; i++) {
            this.abcFiles[i] = AbcFile.loadAbcFile(this, paths[i])
        }
    }

    /**
     * Adds new ABC files in the linker's context.
     *
     * @param paths to new ABC files.
     *
     * @throws errors happened on loading ABC file.
     */
    public addAbcFiles(paths: string[]): void {
        const loadedFiles : FixedArray<AbcFile | undefined> =
            new FixedArray<AbcFile | undefined>(paths.length, undefined)
        let count = 0
        for (let i = 0; i < paths.length; i++) {
            const file = AbcFile.loadAbcFile(this, paths[i])
            if (file) {
                loadedFiles[count] = file
                count++
            } else {
                // NOTE: Skipping invalid path as a temporary workaround, should throw an exception instead
                console.error("Failed to load ABC file: " + paths[i])
            }
        }
        const newFiles : FixedArray<AbcFile | undefined> = 
            new FixedArray<AbcFile | undefined>(count, undefined)
        for (let i = 0; i < count; i++) {
            newFiles[i] = loadedFiles[i]
        }
        this.addAbcFiles(newFiles)
    }

    /**
     * Find class in parent RuntimeLinker or within unloaded classes from ABC files.
     * The queried class could be already loaded - in this case it will be returned by parent.
     *
     * @param clsName name of the class.
     *
     * @param init indicator whether class must be initialized.
     *
     * @returns Class if it was found, undefined otherwise.
     *
     * @throws errors happened during class initialization.
     */
    protected override findAndLoadClass(clsName: string, init: boolean): Class | undefined {
        let optClass = this.parentLinker.loadClassSafe(clsName, init)
        if (optClass) {
            return optClass
        }
        for (let sharedLibraryLoader of this.sharedLibraryLoaders) {
            let sharedClass = sharedLibraryLoader!.loadClassSafe(clsName, init)
            if (sharedClass) {
                return sharedClass
            }
        }
        return this.loadClassFromAbcFiles(clsName, init)
    }

    /**
     * Adds new ABC files in the linker's context.
     *
     * @param newFiles ABC files loaded into current context.
     */
    protected final native addAbcFiles(newFiles: FixedArray<AbcFile | undefined>): void

    private native loadClassFromAbcFiles(clsName: string, init: boolean): Class | undefined

    /**
     * Cold reload: prepend a patch abc to this linker's search order, so that
     * classes present in the patch are loaded from it and classes absent from
     * it fall back to the files already registered via addAbcFiles.
     *
     * Intended for application startup (before any user code has run), after
     * the process has been restarted. Structural changes are supported, since
     * there is no live state to preserve. The process must not have AOT files
     * loaded; such a call is rejected. A package without the abc entry (a
     * native-only patch) is a no-op success: class lookup has nothing to
     * reorder. Calling this more than once stacks the patches, newest first.
     *
     * @param patchPath path of the patch: a plain `.abc` file, or a `.hqf` /
     *                  `.hap` / `.hsp` package holding `ets/modules_static.abc`.
     *
     * @returns 0 on success, otherwise a `coldreload::Error` ordinal.
     *          Throws NullPointerException when patchPath is null; every
     *          other failure comes back as the return code.
     */
    protected final native coldReload(patchPath: string): int

    /**
     * Replaces an ABC file already loaded in this linker's context by a new version of the same
     * file (hot reload).
     *
     * This is the production entry point: it is addressed by the application framework from
     * native code, never called from arbitrary ArkTS, so it is not part of the public ArkTS
     * API. Bytecode-driven tests use the `std.debug.RuntimeDebug.hotReload` probe,
     * which shares all of the logic below the target resolution.
     *
     * The capability is disabled by default: the process must have been started with the
     * `enable-hotreload` runtime option (the framework passes it when it creates a debug
     * session), and the entry must not be reached from managed code outside the boot context.
     *
     * Only method bodies may differ between the two versions. Any structural change (fields,
     * method set, signatures, hierarchy, modifiers) is rejected, and so is a class that is loaded
     * from the target but missing from the patch. Objects, field values and running calls are all
     * preserved: only the code executed by subsequent calls changes.
     *
     * The whole call is a transaction. On any error nothing in the VM has been modified and the
     * call may simply be retried. A concurrent class load or an `addAbcFiles` writer racing the
     * transaction is not a patch problem: re-issue the SAME call once it completes.
     *
     * The process must run without JIT and without AOT files, otherwise the call is refused.
     *
     * @param targetPath path the ABC to be replaced was loaded with, exactly the string that was
     *                   passed to `addAbcFiles`. A `.hap` / `.hsp` package path is accepted.
     *
     * @param patchPath path of the new version. May be a plain `.abc` file or a `.hqf` / `.hap` /
     *                  `.hsp` package containing `ets/modules_static.abc`.
     *
     * @returns 0 on success, otherwise a non-zero error code.
     *          Throws NullPointerException when targetPath or patchPath is null; every
     *          other failure comes back as the return code, with the detail reported
     *          through the `hotreload` log component.
     */
    protected final native hotReload(targetPath: string, patchPath: string): int
}
