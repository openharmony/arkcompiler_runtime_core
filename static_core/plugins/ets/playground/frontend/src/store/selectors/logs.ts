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

import { RootState } from '..';
import { ILog } from '../../models/logs';

export const selectCompileOutLogs = (state: RootState): ILog[] => state.logs.compileOut;
export const selectCompileErrLogs = (state: RootState): ILog[] => state.logs.compileErr;
export const selectRunOutLogs = (state: RootState): ILog[] => state.logs.runOut;
export const selectRunErrLogs = (state: RootState): ILog[] => state.logs.runErr;
export const selectDisasmOutLogs = (state: RootState): ILog[] => state.logs.disasmOut;
export const selectDisasmErrLogs = (state: RootState): ILog[] => state.logs.disasmErr;
export const selectOutLogs = (state: RootState): ILog[] => state.logs.out;
export const selectErrLogs = (state: RootState): ILog[] => state.logs.err;
