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

import React, {useCallback, useEffect, useLayoutEffect, useState} from 'react';
import {Mosaic, MosaicWindow} from 'react-mosaic-component';
import 'react-mosaic-component/react-mosaic-component.css';
import '@blueprintjs/core/lib/css/blueprint.css';
import '@blueprintjs/icons/lib/css/blueprint-icons.css';
import ArkTSEditor from '../../pages/codeEditor/ArkTSEditor';
import cx from 'classnames';
import { Button, Icon, Tab, Tabs } from '@blueprintjs/core';
import styles from './styles.module.scss';
import ControlPanel from '../controlPanel/ControlPanel';
import {useDispatch, useSelector} from 'react-redux';
import {withAstView, withDisasm, selectActiveLogTab} from '../../store/selectors/appState';
import DisasmCode from '../../pages/disasmView/DisasmCode';
import {AppDispatch} from '../../store';
import {fetchOptions} from '../../store/actions/options';
import {fetchSyntax} from '../../store/actions/syntax';
import {
    selectCompilationLogs,
    selectRuntimeLogs,
    selectOutLogs,
    selectErrLogs
} from '../../store/selectors/logs';
import LogsView from '../../pages/logs/LogsView';
import {clearCompilationLogs, clearRuntimeLogs} from '../../store/actions/logs';
import AstView from '../../pages/astView/AstView';
import { fetchFeatures } from '../../store/actions/features';
import { selectASTLoading } from '../../store/selectors/ast';
import { fetchAst } from '../../store/actions/ast';
import { selectAstMode } from '../../store/selectors/features';
import { setActiveLogTab } from '../../store/slices/appState';

export type ViewId = 'code' | 'disasm' | 'logs' | 'ast';

const TITLE_MAP: Record<ViewId, string> = {
    code: 'Code editor',
    disasm: 'Disassembly',
    logs: 'Logs',
    ast: 'AST viewer'
};

const MosaicApp = (): JSX.Element => {
    const activeTab = useSelector(selectActiveLogTab);
    const withDisasmRender = useSelector(withDisasm);
    const withASTRender = useSelector(withAstView);
    const isAstLoading = useSelector(selectASTLoading);
    const astmode = useSelector(selectAstMode);
    const compilationLogs = useSelector(selectCompilationLogs);
    const runtimeLogs = useSelector(selectRuntimeLogs);
    const outLogs = useSelector(selectOutLogs);
    const errLogs = useSelector(selectErrLogs);

    const dispatch = useDispatch<AppDispatch>();
    const [compilationLogsCount, setCompilationLogsCount] = useState(0);
    const [runtimeLogsCount, setRuntimeLogsCount] = useState(0);

    useLayoutEffect(() => {
        dispatch(fetchOptions());
        dispatch(fetchSyntax());
        dispatch(fetchFeatures());
    }, []);

    const handleTabChange = (newTab: 'compilation' | 'runtime'): void => {
        dispatch(setActiveLogTab(newTab));
    };

    const handleASTView = (): void => {
        dispatch(fetchAst('manual'))
    };

    useEffect(() => {
        const compilationUnread = [
            ...(outLogs?.filter(el => !el?.isRead && (el.from === 'compileOut' || el.from === 'compileErr')) || []),
            ...(errLogs?.filter(el => !el?.isRead && (el.from === 'compileOut' || el.from === 'compileErr')) || [])
        ].length;

        const runtimeUnread = [
            ...(outLogs?.filter(el => !el?.isRead && (el.from === 'runOut' || el.from === 'runErr')) || []),
            ...(errLogs?.filter(el => !el?.isRead && (el.from === 'runOut' || el.from === 'runErr')) || [])
        ].length;

        setCompilationLogsCount(compilationUnread);
        setRuntimeLogsCount(runtimeUnread);
    }, [outLogs, errLogs]);

    const header = useCallback((id: string): JSX.Element => {
        switch (id) {
            case 'code':
                return <ControlPanel />;
            case 'disasm':
                return <div>disasm</div>;
            case 'ast':
                return <div className={styles.astHeader}>
                AST view
                {astmode === 'manual' && <Button
                        icon={isAstLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11}/></div>
                            : <div className={styles.icon}><Icon icon="play" size={20}/></div>}
                        className={styles.btn}
                        onClick={handleASTView}
                        disabled={isAstLoading}
                        data-testid="run-btn"
                    />}
                </div>;
            case 'logs':
                return (<Tabs
                    id="logs-tabs"
                    onChange={(tabId): void => handleTabChange(tabId as 'compilation' | 'runtime')}
                    selectedTabId={activeTab}
                >
                    <Tab id="compilation" className={styles.tab}>
                        Compile
                        {compilationLogsCount > 0 && <div className={styles.notif} />}
                    </Tab>
                    <Tab id="runtime" className={styles.tab}>
                        Execute
                        {runtimeLogsCount > 0 && <div className={styles.notif} />}
                    </Tab>
                </Tabs>);
            default:
                return <></>;
        }
    }, [activeTab, compilationLogsCount, runtimeLogsCount, astmode, isAstLoading]);

    const handleClearCompilationLogs = (): void => {
        dispatch(clearCompilationLogs());
    };
    const handleClearRuntimeLogs = (): void => {
        dispatch(clearRuntimeLogs());
    };

    const content = useCallback((title: string): JSX.Element => {
        switch (title) {
            case 'Code editor':
                return (
                    <div className={cx({
                        [styles.codeContainer]: true,
                        [styles.part]: withDisasmRender
                    })}>
                        <div className={cx(styles.code, 'monaco-editor')}>
                            <ArkTSEditor />
                        </div>
                        {withDisasmRender && <div className={cx(styles.disasm, 'monaco-editor')}>
                            <DisasmCode />
                        </div>}
                    </div>
                )
            case 'Logs':
                return (
                    <div className={styles.tabContent}>
                                {activeTab === 'compilation' ? (
                                    <LogsView
                                        logArr={compilationLogs}
                                        logType='compilation'
                                        clearFilters={handleClearCompilationLogs}
                                    />
                                ) : (
                                    <LogsView
                                        logArr={runtimeLogs}
                                        logType='runtime'
                                        clearFilters={handleClearRuntimeLogs}
                                    />
                                )}
                    </div>)
            case 'AST viewer':
                return (
                    <div className={cx({
                        [styles.codeContainer]: true,
                    })}>
                        <div className={cx(styles.code, 'monaco-editor')}>
                            <AstView />
                        </div>
                    </div>
                )
            default:
                return <h1>{title}</h1>
        }
    }, [withDisasmRender, compilationLogs, runtimeLogs, activeTab]);

    return (
        <div
            id='app'
            data-testid='mosaic-app-component'
        >
            <Mosaic<ViewId>
                className={cx('mosaic-blueprint-theme', styles.mosaicContainer)}
                blueprintNamespace="bp5"
                renderTile={(id, path): JSX.Element => (
                    <MosaicWindow<ViewId>
                        path={path}
                        title={TITLE_MAP[id]}
                        toolbarControls={<></>}
                        renderToolbar={(props, draggable): JSX.Element => (
                            <div className={styles.customHeader}>
                                {header(id)}
                            </div>
                        )}
                        className={cx(styles.window, {
                            [styles.windowCode]: TITLE_MAP[id] === 'Code editor'
                        })}
                    >
                        {content(TITLE_MAP[id])}
                    </MosaicWindow>
                )}
                initialValue={!withASTRender ?
                    {
                    direction: withDisasmRender ? 'column' : 'row',
                    first: 'code',
                    second: 'logs',
                    splitPercentage: withDisasmRender ? 70 : 50
                } : {
                    direction: 'column',
                    first: {
                        direction: 'row',
                        first: 'code',
                        second: 'ast',
                    },
                    second: 'logs',
                    splitPercentage: 70
                }}
            />
        </div>
    );
};

export default MosaicApp;
