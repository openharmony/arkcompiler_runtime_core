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
import {setDisasm, setPrimaryColor, setTheme, Theme} from '../slices/appState';

export const setThemeAction = createAsyncThunk(
    '@theme/change',
    (data: Theme, thunkAPI) => {
        thunkAPI.dispatch(setTheme(data));
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
