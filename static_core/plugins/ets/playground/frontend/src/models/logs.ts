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
    setHighlightErrors,
    setOutLogs,
    setRunErrLogs,
    setRunOutLogs,
    setVerifierErrLogs,
    setVerifierOutLogs
} from '../store/slices/logs';
import { RootState } from '../store';

export enum ELogType {
    COMPILE_OUT = 'compileOut',
    COMPILE_ERR = 'compileErr',
    RUN_OUT = 'runOut',
    RUN_ERR = 'runErr',
    DISASM_OUT = 'disasmOut',
    DISASM_ERR = 'disasmErr',
    VERIFIER_OUT = 'verifierOut',
    VERIFIER_ERR = 'verifierErr',
}

export interface ILog {
    message: {message: string, line?: number, column?: number}[];
    isRead?: boolean;
    from?: ELogType;
}

export interface ICompileData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IRunData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IDisassemblyData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IVerifierData {
    exit_code?: number;
    output?: string;
    error?: string;
}

export interface IApiResponse {
    data: {
        compile?: ICompileData;
        run?: IRunData;
        disassembly?: IDisassemblyData;
        verifier?: IVerifierData;
    };
}

type HighlightItem = { message: string; line?: number; column?: number; type?: string };

const collectHighlights = (str?: string): HighlightItem[] => {
    if (!str) {
        return [];
    }

    const re =
    /(?<type>\w+(?:Error|Exception|Warning|Notice|Info|Debug|Fatal)?):\s+(?<msg>[\s\S]*?)\s+\[(?<file>[^\[\]]+?):(?<line>\d+):(?<col>\d+)\]/g;

    const out: HighlightItem[] = [];

    const pushPlainSegments = (text: string) => {
        const parts = text.match(/(?:[\s\S]*?\n|[^\n]+$)/g) ?? [];
        for (const p of parts) {
            if (p === '') {
                continue;
            }
            out.push({ message: p });
        }
    };

    let last = 0;
    for (const m of str.matchAll(re)) {
        const start = m.index ?? 0;
        const full = m[0] ?? '';
        const end = start + full.length;

        if (start > last) {
            pushPlainSegments(str.slice(last, start));
        }
        const { type, msg, file, line, col } = (m.groups ?? {}) as {
            type?: string; msg?: string; file?: string; line?: string; col?: string;
        };

        out.push({
            message: `${type}: ${(msg ?? '').trim()} [${file ?? ''}:${line ?? ''}:${col ?? ''}]`,
            line: line ? Number(line) : undefined,
            column: col ? Number(col) : undefined,
            type,
        });
        last = end;
    }

    if (last < str.length) {
        pushPlainSegments(str.slice(last));
    }
    return out;
};

export const handleResponseLogs = createAsyncThunk(
    'logs/compileLogs',
    async (response: IApiResponse, thunkAPI) => {

        const handleLog = (
            data: ICompileData | IRunData | IDisassemblyData | undefined,
            logTypeOut: ELogType,
            logTypeErr: ELogType,
            successMessage: string,
            setOutAction: (logs: ILog[]) => { payload: ILog[]; type: string },
            setErrAction: (logs: ILog[]) => { payload: ILog[]; type: string }
        ): void => {
            const state: RootState = thunkAPI.getState() as RootState;
            const logsState = state.logs;

            if (!data) {
                return;
            }
            if (data.output && data.error) {
                thunkAPI.dispatch(setOutAction(
                    logsState.out.concat([{
                        message: collectHighlights(data.output),
                        isRead: false,
                        from: logTypeOut
                    }])
                ));
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: collectHighlights(data.output),
                        isRead: false,
                        from: logTypeOut
                    })
                ));
                thunkAPI.dispatch(setErrAction(
                    logsState.err.concat([{
                        message: collectHighlights(data.error),
                        isRead: false,
                        from: logTypeErr
                    }])
                ));
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: collectHighlights(data.error),
                        isRead: false,
                        from: logTypeErr
                    })
                ));
            } else if (data.error) {
                thunkAPI.dispatch(setErrAction(
                    logsState.err.concat([{
                        message: collectHighlights(data.error),
                        isRead: false,
                        from: logTypeErr
                    }])
                ));
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: collectHighlights(data.error),
                        isRead: false,
                        from: logTypeErr
                    })
                ));
            } else if ((data.exit_code === 0 || data.exit_code === 254) && data.output && !data.error) {
                thunkAPI.dispatch(setOutAction(
                    logsState.out.concat({
                        message: collectHighlights(data.output),
                        isRead: false,
                        from: logTypeOut
                    })
                ));
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: collectHighlights(data.output),
                        isRead: false,
                        from: logTypeOut
                    })
                ));
            } else if (data.exit_code === 0 && !data.error && !data.output) {
                thunkAPI.dispatch(setOutAction(
                    logsState.out.concat({
                        message: [{message: successMessage}],
                        isRead: false,
                        from: logTypeOut
                    })
                ));
                thunkAPI.dispatch(setOutLogs(
                    logsState.out.concat({
                        message: [{message: successMessage}],
                        isRead: false,
                        from: logTypeOut
                    })
                ));
            } else if (data.exit_code && data.exit_code !== 0 && data.exit_code !== 254) {
                thunkAPI.dispatch(setErrAction(
                    logsState.err.concat({
                        message: collectHighlights(data.output) || `Exit code: ${data.exit_code}`,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
                thunkAPI.dispatch(setErrLogs(
                    logsState.err.concat({
                        message: collectHighlights(data.output) || `Exit code: ${data.exit_code}`,
                        isRead: false,
                        from: logTypeErr
                    })
                ));
            }
            const outHighlight = collectHighlights(data.output)
            const errHighlight = collectHighlights(data.error)
            thunkAPI.dispatch(setHighlightErrors([...outHighlight, ...errHighlight]));
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

        if (response.data.verifier?.error || response.data.verifier?.output) {
            handleLog(
                response.data.verifier,
                ELogType.VERIFIER_OUT,
                ELogType.VERIFIER_ERR,
                '',
                setVerifierOutLogs,
                setVerifierErrLogs
            );
        }
    }
);
