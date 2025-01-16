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
    setClearLogsEachRun,
    setDisasm,
    setPrimaryColor,
    setRuntimeVerify,
    setTheme,
    setVerifier,
    setVersions,
    setVersionsLoading,
    Theme
} from '../slices/appState';
import { RootState } from '..';
import { versionService } from '../../services/versions';

export const setThemeAction = createAsyncThunk(
    '@theme/change',
    (data: Theme, thunkAPI) => {
        thunkAPI.dispatch(setTheme(data));
    },
);
export const setClearLogsAction = createAsyncThunk(
    '@clearLogs/change',
    (data: boolean, thunkAPI) => {
        thunkAPI.dispatch(setClearLogsEachRun(data));
    },
);
export const setPrimaryColorAction = createAsyncThunk(
    '@theme/changeColor',
    (data: string, thunkAPI) => {
        thunkAPI.dispatch(setPrimaryColor(data));
    },
);
export const setDisasmAction = createAsyncThunk(
    '@disasm/change',
    (data: boolean, thunkAPI) => {
        thunkAPI.dispatch(setDisasm(data));
    },
);
export const setVerifAction = createAsyncThunk(
    '@verifier/change',
    (data: boolean, thunkAPI) => {
        thunkAPI.dispatch(setVerifier(data));
    },
);
export const setRuntimeVerifAction = createAsyncThunk(
    '@runtimeVerify/change',
    (data: boolean, thunkAPI) => {
        thunkAPI.dispatch(setRuntimeVerify(data));
    },
);

export const fetchVersions = createAsyncThunk(
    '@versions/fetchVersions',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setVersionsLoading(true));
        const state: RootState = thunkAPI.getState() as RootState;
        try {
            const response = await versionService.fetchVersions();
            if (response.error) {
                console.error(response.error);
                thunkAPI.dispatch(setVersionsLoading(false));
                return;
            }
            thunkAPI.dispatch(setVersions(response.data));
        } catch (error) {
            console.error('Failed to fetch versions:', error);
        } finally {
            thunkAPI.dispatch(setVersionsLoading(false));
        }
    }
);

export const setVersionAction = createAsyncThunk(
    '@versions/setVersion',
    (data: { frontend: string; backend: string; arkts: string; es2panda: string }, thunkAPI) => {
        thunkAPI.dispatch(setVersions(data));
    }
);
