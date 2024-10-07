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
import {useDispatch, useSelector} from 'react-redux';
import {selectSyntax} from '../../store/selectors/syntax';
import {setCodeAction} from '../../store/actions/code';
import {AppDispatch} from '../../store';
import debounce from 'lodash.debounce';

const ArkTSEditor: React.FC = () => {
    let backendSyntax = useSelector(selectSyntax);
    const monaco = useMonaco();
    const dispatch = useDispatch<AppDispatch>();
    const {theme} = useTheme();
    const [syn, setSyn] = useState(backendSyntax);

    useEffect(() => {
        setSyn(backendSyntax);
    }, [backendSyntax]);

    useEffect(() => {
        if (monaco && backendSyntax) {
            const cloneSyntax = Object.assign({}, backendSyntax);
            const existingLang = monaco.languages.getLanguages()
                // @ts-ignore
                .find(lang => lang.id === 'arkts');
            if (!existingLang) {
                monaco.languages.register({ id: 'arkts' });
            }
            // @ts-ignore
            monaco.languages.setMonarchTokensProvider('arkts', cloneSyntax);
            monaco.languages.setLanguageConfiguration('arkts', {
            brackets: [
                ['{', '}'],
                ['[', ']'],
                ['(', ')'],
            ],
            autoClosingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: "'", close: "'" },
            ],
            surroundingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: "'", close: "'" },
            ],
        });
        }
    }, [monaco, syn]);

    const handleEditorChange = debounce((value: string = '') => {
        dispatch(setCodeAction(value));
    }, 300);

    return (
        <Editor
            height="90vh"
            defaultLanguage="arkts"
            defaultValue="function main(): void {}"
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            onChange={handleEditorChange}
            className={styles.editor}
        />
    );
};

export default ArkTSEditor;
