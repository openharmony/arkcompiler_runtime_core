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

import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import {ILog} from '../../models/logs';

interface IState {
    compileOut: ILog[]
    compileErr: ILog[]
    runOut: ILog[]
    runErr: ILog[]
    disasmOut: ILog[]
    disasmErr: ILog[]
    verifierOut: ILog[]
    verifierErr: ILog[]
    out: ILog[]
    err: ILog[]
    highlightErrors: HighlightError[];
    jumpTo: JumpTo | null;
}
export type JumpTo = { line: number; column: number; nonce?: number };

export interface HighlightError {
    message: string;
    line: number;
    column: number;
    type?: string
}

const initialState: IState = {
    compileOut: [],
    compileErr: [],
    runOut: [],
    runErr: [],
    disasmOut: [],
    disasmErr: [],
    verifierOut: [],
    verifierErr: [],
    out: [],
    err: [],
    highlightErrors: [],
    jumpTo: null
};

const logsSlice = createSlice({
    name: 'logsState',
    initialState,
    reducers: {
        setCompileOutLogs(state, action: PayloadAction<ILog[]>) {
            state.compileOut = action.payload;
        },
        setCompileErrLogs(state, action: PayloadAction<ILog[]>) {
            state.compileErr = action.payload;
        },
        setRunOutLogs(state, action: PayloadAction<ILog[]>) {
            state.runOut = action.payload;
        },
        setRunErrLogs(state, action: PayloadAction<ILog[]>) {
            state.runErr = action.payload;
        },
        setVerifierOutLogs(state, action: PayloadAction<ILog[]>) {
            state.runOut = action.payload;
        },
        setVerifierErrLogs(state, action: PayloadAction<ILog[]>) {
            state.runErr = action.payload;
        },
        setDisasmOutLogs(state, action: PayloadAction<ILog[]>) {
            state.disasmOut = action.payload;
        },
        setDisasmErrLogs(state, action: PayloadAction<ILog[]>) {
            state.disasmErr = action.payload;
        },
        setOutLogs(state, action: PayloadAction<ILog[]>) {
            state.out = action.payload;
        },
        setErrLogs(state, action: PayloadAction<ILog[]>) {
            state.err = action.payload;
        },
        setHighlightErrors: (state, action) => {
            state.highlightErrors = state.highlightErrors.concat(action.payload);
        },
        setClearHighLightErrs: (state, action) => {
            state.highlightErrors = [];
        },
        setJumpTo(state, action: PayloadAction<JumpTo | null>) {
            state.jumpTo = action.payload;
        },
    }
});

export const {
    setCompileOutLogs,
    setCompileErrLogs,
    setRunOutLogs,
    setRunErrLogs,
    setDisasmOutLogs,
    setDisasmErrLogs,
    setVerifierOutLogs,
    setVerifierErrLogs,
    setOutLogs,
    setErrLogs,
    setHighlightErrors,
    setClearHighLightErrs,
    setJumpTo,
} = logsSlice.actions;

export default logsSlice.reducer;
