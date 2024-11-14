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

import React, {useEffect, useState} from 'react';
import Editor, {useMonaco} from '@monaco-editor/react';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useSelector} from 'react-redux';
import {selectCompileLoading, selectRunLoading, selectRunRes, selectCompileRes} from '../../store/selectors/code';
import {selectSyntax} from '../../store/selectors/syntax';
import {asmSyntax} from './syntax';

export const DISASM = 'arkasm';

const DisasmEditor: React.FC = () => {
    let backendSyntax = useSelector(selectSyntax);
    const monaco = useMonaco();
    const {theme} = useTheme();
    const [code, setCode] = React.useState('');
    const runRes = useSelector(selectRunRes);
    const compileRes = useSelector(selectCompileRes);
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);
    const [syn, setSyn] = useState(backendSyntax);

    useEffect(() => {
        setSyn(backendSyntax);
    }, [backendSyntax]);

    useEffect(() => {
        if (compileRes?.disassembly?.code) {
            setCode(compileRes.disassembly.code);
        } else if (runRes?.disassembly?.code) {
            setCode(runRes.disassembly.code);
        }
    }, [runRes, compileRes]);

    useEffect(() => {
        if (monaco) {
            const existingLang = monaco.languages.getLanguages()
                // @ts-ignore
                .find(lang => lang.id === DISASM);
            if (!existingLang) {
                monaco.languages.register({ id: DISASM });
            }
            // @ts-ignore
            monaco.languages.setMonarchTokensProvider(DISASM, asmSyntax);

        monaco.languages.setLanguageConfiguration(DISASM, {
            brackets: [
                ['{', '}'],
                ['[', ']'],
                ['(', ')'],
                ['<', '>']
            ],
            autoClosingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: '<', close: '>' },
            ],
            surroundingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: '<', close: '>' },
            ],
        });
        }
    }, [monaco, syn]);

    return (
        <Editor
            height="90vh"
            defaultLanguage={DISASM}
            defaultValue={code}
            value={isCompileLoading || isRunLoading ? 'Loading...' : code}
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            className={styles.container}
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
            }}
        />
    );
};

export default DisasmEditor;
