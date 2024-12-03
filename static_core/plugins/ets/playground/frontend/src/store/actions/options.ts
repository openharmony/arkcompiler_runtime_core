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
import {setOptionsLoading, setOptionsPicked, setOptionsResponse} from '../slices/options';
import {optionsService} from '../../services/options';
import {TObj} from '../../models/options';

export const fetchOptions = createAsyncThunk(
    '@options/fetch',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setOptionsLoading(true));
        const response = await optionsService.fetchGetOptions();
        if (response.error) {
            thunkAPI.dispatch(setOptionsLoading(false));
            return;
        }
        thunkAPI.dispatch(setOptionsResponse(response.data));
        thunkAPI.dispatch(setOptionsLoading(false));
    },
);

export const pickOptions = createAsyncThunk(
    '@options/select',
    async (opt: TObj, thunkAPI) => {
        thunkAPI.dispatch(setOptionsPicked(opt));
    },
);
