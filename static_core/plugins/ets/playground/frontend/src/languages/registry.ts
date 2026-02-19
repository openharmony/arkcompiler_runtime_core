/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import * as monaco from 'monaco-editor';
import type { LanguagePlugin } from './types';

const registered = new Set<string>();

export const registerLanguage = (plugin: LanguagePlugin): void => {
    if (registered.has(plugin.id)) {
        return;
    }
    monaco.languages.register({ id: plugin.id });
    monaco.languages.setMonarchTokensProvider(plugin.id, plugin.monarchTokens);
    if (plugin.configuration) {
        monaco.languages.setLanguageConfiguration(plugin.id, plugin.configuration);
    }
    registered.add(plugin.id);
};
