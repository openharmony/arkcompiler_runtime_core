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

import { RootState } from '..';
import { Theme, Versions } from '../slices/appState';

export const theme = (state: RootState): Theme => state.appState.theme;
export const primaryColor = (state: RootState): string => state.appState.primaryColor;
export const withDisasm = (state: RootState): boolean => state.appState.disasm;
export const withVerifier = (state: RootState): boolean => state.appState.verifier;
export const withRuntimeVerify = (state: RootState): boolean => state.appState.runtimeVerify;
export const versions = (state: RootState): Versions => state.appState.versions;
export const isVersionsLoading = (state: RootState): boolean => state.appState.versionsLoading;
export const clearLogsEachRun = (state: RootState): boolean => state.appState.clearLogsEachRun;
