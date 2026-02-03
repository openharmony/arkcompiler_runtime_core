/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    setAstView,
    setClearLogsEachRun,
    setDisasm,
    setPrimaryColor,
    setVerificationMode,
    setAotMode,
    setTheme,
    setVerifier,
    setVersions,
    setVersionsLoading,
    setIrDumpCompilerDump,
    setIrDumpDisasmDump,
} from '../slices/appState';
import { versionService } from '../../services/versions';

export const setThemeAction = setTheme;
export const setClearLogsAction = setClearLogsEachRun;
export const setPrimaryColorAction = setPrimaryColor;
export const setDisasmAction = setDisasm;
export const setVerifAction = setVerifier;
export const setAstViewAction = setAstView;
export const setVerificationModeAction = setVerificationMode;
export const setAotModeAction = setAotMode;
export const setVersionAction = setVersions;
export const setIrDumpCompilerDumpAction = setIrDumpCompilerDump;
export const setIrDumpDisasmDumpAction = setIrDumpDisasmDump;

export const fetchVersions = createAsyncThunk(
    '@versions/fetchVersions',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setVersionsLoading(true));
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
