/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { IObj } from '../store/slices/options';

export interface ICodeFetch {
    code: string
    disassemble: boolean
    options: IObj | null
}
export interface ICodeReq {
    compile: {
        output: string,
        error: string,
        exit_code: number
    },
    disassembly: {
        output: string,
        code: string,
        error: string,
        exit_code: number
    }
}
export interface IRunReq {
    compile: {
        output: string,
        error: string,
        exit_code: number
    },
    disassembly: {
        output: string,
        code: string,
        error: string,
        exit_code: number
    },
    run: {
        output: string,
        error: string,
        exit_code: number
    },
}

export const codeModel = {
    fillDefaults: <T extends Record<keyof T, string | number>>(
        data: Partial<T>,
        defaults: T
    ): T => {
        return Object.keys(defaults).reduce((acc, key) => {
            const typedKey = key as keyof T;
            acc[typedKey] = data[typedKey] ?? defaults[typedKey];
            return acc;
        }, { ...defaults });
    },
    fromApiCompile: (data: ICodeReq): ICodeReq => ({
        compile: codeModel.fillDefaults(data.compile || {}, { output: '', error: '', exit_code: -1 }),
        disassembly: codeModel.fillDefaults(data.disassembly || {}, { output: '', code: '', error: '', exit_code: -1 }),
    }),
    fromApiRun: (data: IRunReq): IRunReq => ({
        compile: codeModel.fillDefaults(data.compile || {}, { output: '', error: '', exit_code: -1 }),
        disassembly: codeModel.fillDefaults(data.disassembly || {}, { output: '', code: '', error: '', exit_code: -1 }),
        run: codeModel.fillDefaults(data.run || {}, { output: '', error: '', exit_code: -1 }),
    }),
};
