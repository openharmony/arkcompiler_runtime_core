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
import {setSyntaxLoading, setSyntaxResponse} from '../slices/syntax';
import {syntaxService} from '../../services/syntax';

export const fetchSyntax = createAsyncThunk(
    '@syntax/fetch',
    async (_, thunkAPI) => {
        thunkAPI.dispatch(setSyntaxLoading(true));
        const response = await syntaxService.fetchGetSyntax();
        if (response.error) {
            thunkAPI.dispatch(setSyntaxLoading(false));
            return;
        }
        thunkAPI.dispatch(setSyntaxResponse(response.data));
        thunkAPI.dispatch(setSyntaxLoading(false));
    },
);
