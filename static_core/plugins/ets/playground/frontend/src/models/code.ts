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
    fromApiCompile: (data: ICodeReq): ICodeReq => {
        return ({
            compile: {
                output: data?.compile?.output || '',
                error: data?.compile?.error || '',
                exit_code: data?.compile?.exit_code || 0
            },
            disassembly: {
                output: data?.disassembly?.output || '',
                code: data?.disassembly?.code || '',
                error: data?.disassembly?.error || '',
                exit_code: data?.disassembly?.exit_code || 0
            }
        });
    },
    fromApiRun: (data: IRunReq): IRunReq => ({
        compile: {
            output: data?.compile?.output || '',
            error: data?.compile?.error || '',
            exit_code: data?.compile?.exit_code || 0
        },
        disassembly: {
            output: data?.disassembly?.output || '',
            code: data?.disassembly?.code || '',
            error: data?.disassembly?.error || '',
            exit_code: data?.disassembly?.exit_code || 0
        },
        run: {
            output: data?.run?.output || '',
            error: data?.run?.error || '',
            exit_code: data?.run?.exit_code || 0
        }
    }),
};
