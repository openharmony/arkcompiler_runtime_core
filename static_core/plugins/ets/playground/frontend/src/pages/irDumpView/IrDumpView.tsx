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

import React, {useCallback, useEffect, useRef} from 'react';
import Editor, {loader, type OnMount} from '@monaco-editor/react';
import * as monaco from 'monaco-editor';
import styles from './styles.module.scss';
import {useTheme} from '../../components/theme/ThemeContext';
import {useSelector} from 'react-redux';
import {selectCompileLoading, selectRunLoading, selectRunRes, selectCompileRes} from '../../store/selectors/code';
import {IIrDumpFile} from '../../models/code';

loader.config({ monaco });

const IR_DUMP_LANGUAGE_ID = 'irdump';
let irdumpRegistered = false;
const ensureIrdumpLanguage = (): void => {
    if (irdumpRegistered) {
        return;
    }
    monaco.languages.register({ id: IR_DUMP_LANGUAGE_ID });
    irdumpRegistered = true;
};

const normalizeDumpFiles = (
    compilerDump: IIrDumpFile[] | null | undefined
): IIrDumpFile[] => {
    if (!compilerDump) {
        return [];
    }
    return compilerDump;
};

const SECTION_HEADER_PREFIX = '# === ';
const SECTION_HEADER_SUFFIX = ' ===';

const buildEditorContent = (files: IIrDumpFile[]): string => {
    if (files.length === 0) {
        return '';
    }
    if (files.length === 1) {
        return `${SECTION_HEADER_PREFIX}${files[0].name}${SECTION_HEADER_SUFFIX}\n${files[0].content}`;
    }
    return files
        .map(f => `${SECTION_HEADER_PREFIX}${f.name}${SECTION_HEADER_SUFFIX}\n${f.content}`)
        .join('\n\n');
};

const computeFoldingRanges = (text: string): { startLine: number; endLine: number }[] => {
    const lines = text.split('\n');
    const ranges: { startLine: number; endLine: number }[] = [];
    let currentStart: number | null = null;

    for (let i = 0; i < lines.length; i++) {
        if (lines[i].startsWith(SECTION_HEADER_PREFIX) && lines[i].endsWith(SECTION_HEADER_SUFFIX)) {
            if (currentStart !== null) {
                let endLine = i; // line before the next header (0-indexed)
                while (endLine > currentStart + 1 && lines[endLine - 1].trim() === '') {
                    endLine--;
                }
                ranges.push({ startLine: currentStart + 1, endLine });
            }
            currentStart = i;
        }
    }
    if (currentStart !== null) {
        ranges.push({ startLine: currentStart + 1, endLine: lines.length });
    }
    return ranges;
};

type DumpType = 'disasm' | 'compiler';

interface IrDumpViewProps {
    dumpType: DumpType;
}

const IrDumpViewBase: React.FC<IrDumpViewProps> = ({ dumpType }) => {
    const {theme} = useTheme();
    const runRes = useSelector(selectRunRes);
    const compileRes = useSelector(selectCompileRes);
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);

    const isCompiler = dumpType === 'compiler';

    const editorRef = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);
    const foldingProviderRef = useRef<monaco.IDisposable | null>(null);
    const multiFileRef = useRef(false);
    const codeRef = useRef('');

    const getCode = useCallback((): string => {
        if (isCompileLoading || isRunLoading) {
            return '';
        }
        if (isCompiler) {
            const dump = compileRes?.ir_dump?.compiler_dump ?? runRes?.ir_dump?.compiler_dump;
            const files = normalizeDumpFiles(dump);
            multiFileRef.current = files.length > 1;
            return buildEditorContent(files);
        }
        multiFileRef.current = false;
        return compileRes?.ir_dump?.disasm_dump ?? runRes?.ir_dump?.disasm_dump ?? '';
    }, [runRes, compileRes, isCompileLoading, isRunLoading, isCompiler]);

    const code = getCode();
    codeRef.current = code;

    const handleEditorMount: OnMount = useCallback((editor) => {
        editorRef.current = editor;

        if (!isCompiler) {
            return;
        }

        foldingProviderRef.current = monaco.languages.registerFoldingRangeProvider(IR_DUMP_LANGUAGE_ID, {
            provideFoldingRanges(): monaco.languages.FoldingRange[] {
                const text = codeRef.current;
                if (!text) {
                    return [];
                }
                const ranges = computeFoldingRanges(text);
                return ranges.map(r => ({
                    start: r.startLine,
                    end: r.endLine,
                    kind: monaco.languages.FoldingRangeKind.Region,
                }));
            }
        });

        if (multiFileRef.current) {
            setTimeout(() => {
                editor.getAction('editor.foldAll')?.run();
                editor.setPosition({ lineNumber: 1, column: 1 });
            }, 100);
        }
    }, [isCompiler]);

    useEffect(() => {
        if (!editorRef.current || !isCompiler || !multiFileRef.current) {
            return;
        }
        setTimeout(() => {
            editorRef.current?.getAction('editor.foldAll')?.run();
            editorRef.current?.setPosition({ lineNumber: 1, column: 1 });
        }, 100);
    }, [code, isCompiler]);

    useEffect(() => {
        return (): void => {
            foldingProviderRef.current?.dispose();
        };
    }, []);

    if (isCompiler) {
        ensureIrdumpLanguage();
    }

    return (
        <Editor
            defaultLanguage={isCompiler ? IR_DUMP_LANGUAGE_ID : 'plaintext'}
            value={isCompileLoading || isRunLoading ? 'Loading...' : code}
            theme={theme === 'dark' ? 'vs-dark' : 'light'}
            className={styles.editor}
            onMount={handleEditorMount}
            options={{
                readOnly: true,
                wordWrap: 'on',
                minimap: { enabled: false },
                automaticLayout: true,
                ...(isCompiler && {
                    folding: true,
                    showFoldingControls: 'always' as const,
                    scrollBeyondLastLine: false,
                }),
            }}
        />
    );
};

export const DisasmDumpView: React.FC = () => <IrDumpViewBase dumpType="disasm" />;

export const CompilerDumpView: React.FC = () => <IrDumpViewBase dumpType="compiler" />;
