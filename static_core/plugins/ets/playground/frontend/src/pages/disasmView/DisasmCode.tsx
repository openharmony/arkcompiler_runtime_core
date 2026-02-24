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

import React, {useEffect} from 'react';
import Editor, {loader} from '@monaco-editor/react';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useSelector} from 'react-redux';
import {selectCompileLoading, selectRunLoading, selectRunRes, selectCompileRes} from '../../store/selectors/code';
import * as monaco from 'monaco-editor';
import {registerLanguage, arkasmPlugin, ensureThemes, getThemeName} from '../../languages';

loader.config({ monaco });
registerLanguage(arkasmPlugin);
ensureThemes();

const DisasmEditor: React.FC = () => {
    const {theme} = useTheme();
    const [code, setCode] = React.useState('');
    const runRes = useSelector(selectRunRes);
    const compileRes = useSelector(selectCompileRes);
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);

    useEffect(() => {
        if (isCompileLoading || isRunLoading) {
            setCode('');
            return;
        }
        if (compileRes?.disassembly?.code) {
            setCode(compileRes.disassembly.code);
        } else if (runRes?.disassembly?.code) {
            setCode(runRes.disassembly.code);
        } else if (compileRes !== null || runRes !== null) {
            setCode('');
        }
    }, [runRes, compileRes, isCompileLoading, isRunLoading]);

    return (
        <Editor
            defaultValue={code}
            language={isCompileLoading || isRunLoading ? 'plaintext' : arkasmPlugin.id}
            value={isCompileLoading || isRunLoading ? 'Loading...' : code}
            theme={getThemeName(theme)}
            className={styles.editor}
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
                automaticLayout: true
            }}
        />
    );
};

export default DisasmEditor;
