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
import {setCode, setCompileLoading, setCompileRes, setRunLoading, setRunRes} from '../slices/code';
import {codeService} from '../../services/code';
import {handleResponseLogs} from '../../models/logs';
import { RootState } from '..';

export const fetchCompileCode = createAsyncThunk(
    '@code/compileCode',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setCompileLoading(true));
        thunkAPI.dispatch(setCompileRes(null));
        thunkAPI.dispatch(setRunRes(null));
        const state: RootState = thunkAPI.getState() as RootState;
        const codeState = state.code || '';
        const appState = state?.appState || false;
        const logsState = state?.logs || [];
        const optionsState = state.options || {};
        const response = await codeService.fetchCompile({
            code: codeState?.code,
            options: optionsState?.pickedOptions,
            disassemble: appState.disasm});
        if (response.error) {
            console.error(response.error);
            thunkAPI.dispatch(setCompileLoading(false));
            return;
        }
        thunkAPI.dispatch(handleResponseLogs(response));
        thunkAPI.dispatch(setCompileRes(response.data));
        thunkAPI.dispatch(setCompileLoading(false));
    },
);
export const fetchRunCode = createAsyncThunk(
    '@code/runCode',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setRunLoading(true));
        thunkAPI.dispatch(setCompileRes(null));
        thunkAPI.dispatch(setRunRes(null));
        const state: RootState = thunkAPI.getState() as RootState;
        const codeState = state.code || '';
        const appState = state?.appState || false;
        const logsState = state?.logs;
        const optionsState = state?.options || {};
        const response = await codeService.fetchRun({
            code: codeState?.code,
            options: optionsState?.pickedOptions,
            disassemble: appState.disasm
        });
        if (response.error) {
            console.error(response.error);
            thunkAPI.dispatch(setRunLoading(false));
            return;
        }
        thunkAPI.dispatch(handleResponseLogs(response));
        thunkAPI.dispatch(setRunRes(response.data));
        thunkAPI.dispatch(setRunLoading(false));
    },
);

export const setCodeAction = createAsyncThunk(
    '@code/setCode',
    (data: string, thunkAPI) => {
        thunkAPI.dispatch(setCode(data));
    },
);
