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

package std.core;

/**
 * @class Represents ABC file.
 * Intended to be used by AbcRuntimeLinker and other platform extensions.
 */
final class AbcFile {
    private fileHandlePtr: long;

    private constructor() {
        throw new Error("Constructor called");
    }

    /**
     * Loads ABC file.
     *
     * @param runtimeLinker which the opened file will correspond to.
     *
     * @param path to ABC file.
     *
     * @returns instance of loaded AbcFile.
     *
     * @throws errors happened on loading ABC file.
     */
    public static native loadAbcFile(runtimeLinker: RuntimeLinker, path: string): AbcFile;

    /**
     * Loads ABC file from memory.
     *
     * @param runtimeLinker which the opened file will correspond to.
     *
     * @param rawFile byte array containing the ABC file contents.
     *
     * @returns instance of loaded AbcFile.
     *
     * @throws errors happened on loading ABC file.
     */
    public static native loadFromMemoryImpl(runtimeLinker: RuntimeLinker, rawFile: FixedArray<byte>): AbcFile;
    public static loadFromMemory(runtimeLinker: RuntimeLinker, rawFile: byte[]): AbcFile {
        let localFixed: FixedArray<byte> = new byte[rawFile.length]
        for (let i = 0; i < rawFile.length; i++) {
            localFixed[i] = rawFile[i];
        }
        return AbcFile.loadFromMemoryImpl(runtimeLinker, localFixed);
    }

    /**
     * Loads class from ABC file.
     *
     * @param runtimeLinker which the opened file will correspond to.
     *
     * @param path to ABC file.
     *
     * @returns loaded Class if it was found in the file, undefined otherwise.
     *
     * @throws errors happened during class initialization.
     */
    public native loadClass(runtimeLinker: RuntimeLinker, clsName: string, init: boolean): Class | undefined;

    public native getFilename(): string;
}

export class AbcFileNotFoundError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("AbcFileNotFoundError", message, options)
    }
}