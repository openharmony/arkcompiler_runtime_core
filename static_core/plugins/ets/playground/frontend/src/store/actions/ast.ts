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

import { createAsyncThunk } from '@reduxjs/toolkit';
import { RootState } from '..';
import { astService } from '../../services/ast';
import { handleASTLogs } from '../../models/logs';
import { clearErrLogs, clearOutLogs } from './logs';
import { setClearHighLightErrs } from '../slices/logs';
import { setAstLoading, setAstRes } from '../slices/ast';

type AstTrigger = 'auto' | 'manual';

export const fetchAst = createAsyncThunk(
    '@ast/fetchAst',
    async (triggerArg: AstTrigger | undefined, thunkAPI) => {
        thunkAPI.dispatch(setAstLoading(true));
        thunkAPI.dispatch(setAstRes(null));
    
        const state = thunkAPI.getState() as RootState;
        const code = state.code?.code ?? '';
        const options = state.options?.pickedOptions ?? {};
        const clearLogs = state.appState?.clearLogsEachRun;
        const modeFromState: AstTrigger = (state.features?.astMode ?? 'auto') as AstTrigger;
        const trigger: AstTrigger = triggerArg ?? modeFromState;
    
        thunkAPI.dispatch(setClearHighLightErrs([]));
        if (clearLogs) {
          thunkAPI.dispatch(clearOutLogs());
          thunkAPI.dispatch(clearErrLogs());
        }
    
        const response = await astService.fetchAst({ code, options }, trigger);
    
        if (response.error) {
          console.error(response.error);
          thunkAPI.dispatch(setAstLoading(false));
          return;
        }
        thunkAPI.dispatch(handleASTLogs(response.data));
        thunkAPI.dispatch(setAstRes(response.data));
        thunkAPI.dispatch(setAstLoading(false));
      }
);
