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

import { createAsyncThunk } from '@reduxjs/toolkit';
import {
    setCompileErrLogs,
    setCompileOutLogs, setDisasmErrLogs, setDisasmOutLogs,
    setErrLogs,
    setOutLogs,
    setRunErrLogs,
    setRunOutLogs
} from '../store/slices/logs';
import { RootState } from '../store';

export enum ELogType {
    COMPILE_OUT = 'compileOut',
    COMPILE_ERR = 'compileErr',
    RUN_OUT = 'runOut',
    RUN_ERR = 'runErr',
    DISASM_OUT = 'disasmOut',
    DISASM_ERR = 'disasmErr',
}

export interface ILog {
    message: string;
    isRead?: boolean;
    from?: ELogType;
}

export interface ICompileData {
    exit_code: number;
    output?: string;
    error?: string;
}

export interface IRunData {
    exit_code: number;
    output?: string;
    error?: string;
}

export interface IDisassemblyData {
    exit_code: number;
    output?: string;
    error?: string;
}

export interface IApiResponse {
    data: {
        compile?: ICompileData;
        run?: IRunData;
        disassembly?: IDisassemblyData;
    };
}

export const handleResponseLogs = createAsyncThunk(
    'logs/compileLogs',
    async (response: IApiResponse, thunkAPI) => {
        const state: RootState = thunkAPI.getState() as RootState;
        const logsState = state?.logs;
    if (response?.data?.compile) {
        if (response?.data?.compile.exit_code !== 0 && response?.data?.compile.error) {
            thunkAPI.dispatch(setCompileErrLogs(logsState?.compileErr?.concat({
                message: response?.data?.compile.error,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.compile.error,
                isRead: false,
                from: ELogType.COMPILE_ERR
            })));
        } else if (response?.data?.compile.exit_code === 0) {
            thunkAPI.dispatch(setCompileOutLogs(logsState?.compileOut?.concat({
                message: response?.data?.compile?.output || 'Compile successful!',
                isRead: false
            })));
            thunkAPI.dispatch(setOutLogs(logsState?.out?.concat({
                message: response?.data?.compile?.output || 'Compile successful!',
                isRead: false,
                from: ELogType.COMPILE_OUT
            })));
        } else if (response?.data?.compile.exit_code !== 0 && response?.data?.compile.output) {
            thunkAPI.dispatch(setCompileErrLogs(logsState?.compileErr?.concat({
                message: response?.data?.compile.output,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.compile.output,
                isRead: false,
                from: ELogType.COMPILE_ERR
            })));
        }
    }

    if ('run' in response?.data) {
        if (response?.data?.run?.exit_code !== 0 && response?.data?.run?.error) {
            thunkAPI.dispatch(setRunErrLogs(logsState?.runErr?.concat({
                message: response?.data?.run.error,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.run.error,
                isRead: false,
                from: ELogType.RUN_ERR
            })));
        } else if (response?.data?.run?.exit_code === 0 && response?.data?.run.output) {
            thunkAPI.dispatch(setRunOutLogs(logsState?.runOut?.concat({
                message: response?.data?.run.output,
                isRead: false
            })));
            thunkAPI.dispatch(setOutLogs(logsState?.out?.concat({
                message: response?.data?.run.output,
                isRead: false,
                from: ELogType.RUN_OUT
            })));
        } else if (response?.data?.run?.exit_code !== 0 && response?.data?.run?.output) {
            thunkAPI.dispatch(setRunErrLogs(logsState?.runErr?.concat({
                message: response?.data?.run.output,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.run.output,
                isRead: false,
                from: ELogType.RUN_ERR
            })));
        }
    }

    if (response?.data?.disassembly) {
        if (response?.data?.disassembly?.exit_code !== 0 && response?.data?.disassembly?.error) {
            thunkAPI.dispatch(setDisasmErrLogs(logsState?.disasmErr?.concat({
                message: response?.data?.disassembly.error,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.disassembly.error,
                isRead: false,
                from: ELogType.DISASM_ERR
            })));
        } else if (response?.data?.disassembly?.exit_code === 0 && response?.data?.disassembly?.output) {
            thunkAPI.dispatch(setDisasmOutLogs(logsState?.disasmOut?.concat({
                message: response?.data?.disassembly.output,
                isRead: false
            })));
            thunkAPI.dispatch(setOutLogs(logsState?.out?.concat({
                message: response?.data?.disassembly.output,
                isRead: false,
                from: ELogType.DISASM_OUT
            })));
        } else if (response?.data?.disassembly?.exit_code !== 0 && response?.data?.disassembly?.output) {
            thunkAPI.dispatch(setDisasmErrLogs(logsState?.disasmErr?.concat({
                message: response?.data?.disassembly.output,
                isRead: false
            })));
            thunkAPI.dispatch(setErrLogs(logsState?.err?.concat({
                message: response?.data?.disassembly.output,
                isRead: false,
                from: ELogType.DISASM_ERR
            })));
        }
    }}
);
