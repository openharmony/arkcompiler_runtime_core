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
        const logsState = state.logs;

        const handleLog = (
            data: ICompileData | IRunData | IDisassemblyData | undefined,
            logTypeOut: ELogType,
            logTypeErr: ELogType,
            successMessage: string,
            setOutAction: (logs: ILog[]) => { payload: ILog[]; type: string },
            setErrAction: (logs: ILog[]) => { payload: ILog[]; type: string }
        ): void => {
            if (!data) {
                return;
            }
            if (data.exit_code === 0 && data.output) {
                thunkAPI.dispatch(setOutAction(
                    logsState.out.concat({
                        message: data.output,
                        isRead: false,
                        from: logTypeOut
                    })
                ));
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: data.output,
                        isRead: false,
                        from: logTypeOut
                    })
                ));
            } else if (data.exit_code === 0) {
                thunkAPI.dispatch(setOutAction(
                    logsState.out.concat({
                        message: successMessage,
                        isRead: false,
                        from: logTypeOut
                    })
                ));
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: successMessage,
                        isRead: false,
                        from: logTypeOut
                    })
                ));
            } else if (data.exit_code !== 0 && data.error) {
                thunkAPI.dispatch(setErrAction(
                    logsState.err.concat({
                        message: data.error,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: data.error,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
            } else if (data.exit_code !== 0 && data.output) {
                thunkAPI.dispatch(setErrAction(
                    logsState.err.concat({
                        message: data.output,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: data.output,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
            }
        };

        handleLog(
            response.data.compile,
            ELogType.COMPILE_OUT,
            ELogType.COMPILE_ERR,
            'Compile successful!',
            setCompileOutLogs,
            setCompileErrLogs
        );

        handleLog(
            response.data.run,
            ELogType.RUN_OUT,
            ELogType.RUN_ERR,
            'Run successful!',
            setRunOutLogs,
            setRunErrLogs
        );

        handleLog(
            response.data.disassembly,
            ELogType.DISASM_OUT,
            ELogType.DISASM_ERR,
            'Disassembly successful!',
            setDisasmOutLogs,
            setDisasmErrLogs
        );
    }
);
