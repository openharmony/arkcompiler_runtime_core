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

import React, {useEffect, useRef, useState} from 'react';
import {Button, ButtonGroup, Checkbox, Icon, Popover, Tooltip} from '@blueprintjs/core';
import styles from './styles.module.scss';
import CompileOptions from '../../pages/compileOptions/CompileOptions';
import {useDispatch, useSelector} from 'react-redux';
import {withDisasm, withRuntimeVerify, withVerifier} from '../../store/selectors/appState';
import {AppDispatch} from '../../store';
import {setDisasmAction, setRuntimeVerifAction, setVerifAction} from '../../store/actions/appState';
import {fetchCompileCode, fetchRunCode} from '../../store/actions/code';
import {selectCompileLoading, selectRunLoading, selectShareLoading} from '../../store/selectors/code';
import {useClickOutside} from '../../utils/useClickOutside';
import cx from 'classnames';

const ControlPanel = (): JSX.Element => {
    const popoverRef = useRef<HTMLDivElement | null>(null);
    const [isOpen, setIsOpen] = useState<boolean>(false);
    const disasm = useSelector(withDisasm);
    const runtimeVerify = useSelector(withRuntimeVerify);
    const verifier = useSelector(withVerifier);
    const dispatch = useDispatch<AppDispatch>();
    const isCompileLoading = useSelector(selectCompileLoading);
    const isShareLoading = useSelector(selectShareLoading);
    const isRunLoading = useSelector(selectRunLoading);
    const [runVerify, setRunVerify] = useState(false);

    useEffect(() => {
        setRunVerify(runtimeVerify);
    }, [runtimeVerify]);


    const handleDisasmChange = (): void => {
        dispatch(setDisasmAction(!disasm));
    };
    const handleVerifChange = (): void => {
        dispatch(setVerifAction(!verifier));
    };
    const handleCompile = (): void => {
        dispatch(fetchCompileCode());
    };
    const handleRun = (): void => {
        dispatch(fetchRunCode());
    };
    const handleClosePopover = (): void => {
        setIsOpen(false);
        setRunVerify(runtimeVerify || false);
    };
    const handleOpenPopover = (): void => {
        setIsOpen(true);
    };
    const handleRunVerifReset = (): void => {
        dispatch(setRuntimeVerifAction(false));
        setIsOpen(false);
    };
    const handleRuntimeVerifChange = (): void => {
        dispatch(setRuntimeVerifAction(runVerify));
        handleClosePopover();
    };
    useClickOutside(popoverRef, handleClosePopover);
    return (
        <div className={styles.container}>
            <ButtonGroup>
                <Tooltip content="Run" placement="bottom">
                    <Button
                        icon={isRunLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11}/></div>
                            : <div className={styles.icon}><Icon icon="play" size={20}/></div>}
                        className={styles.btn}
                        onClick={handleRun}
                        disabled={isRunLoading}
                        data-testid="run-btn"
                    />
                </Tooltip>
                <Tooltip content="Compile" placement="bottom">
                    <Button
                        icon={isCompileLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11} /></div>
                            : <div className={styles.icon}><Icon icon="code-block" size={14}/></div>}
                        className={styles.btn}
                        onClick={handleCompile}
                        disabled={isCompileLoading}
                        data-testid="compile-btn"
                    />
                </Tooltip>
                <Tooltip content="Options" placement="bottom">
                    <Popover
                        content={<div className={styles.options}>
                            <CompileOptions onClose={handleClosePopover} />
                            <span className={styles.header}>Run options</span>
                            <Checkbox
                                checked={runVerify}
                                label="Runtime verify"
                                onChange={(e): void => setRunVerify(e.target.checked)}
                                className={styles.disasm}
                                data-testid="runtimeVerify-checkbox"
                            />
                            <div className={styles.btnContainer}>
                                <Button className={cx(styles.btn, styles.btnBorder)} onClick={handleRunVerifReset}>
                                    Reset
                                </Button>
                                <Button className={cx(styles.btn, styles.btnBorder)} onClick={handleRuntimeVerifChange}>
                                    Save
                                </Button>
                            </div>
                        </div>}
                        interactionKind="click"
                        popoverClassName={styles.pop}
                        isOpen={isOpen}
                        popoverRef={popoverRef}
                        position='bottom'
                    >
                        <Button
                            icon={<div className={styles.icon}><Icon icon="more" size={14} /></div>}
                            className={styles.btn}
                            onClick={handleOpenPopover}
                            data-testid='options'
                        />
                    </Popover>
                </Tooltip>
                {/* <Tooltip content="Share" placement="bottom">
                    <Button
                        icon={isShareLoading
                            ? <div className={styles.icon}><Icon icon="build" size={11} /></div>
                            : <div className={styles.icon}><Icon icon="share" size={14}/></div>}
                        className={styles.btn}
                        onClick={handleCompile}
                        disabled={isCompileLoading}
                    />
                        {/* Share
                    </Button>
                </Tooltip> */}
            </ButtonGroup>
            <Checkbox
                checked={disasm}
                label="Disasm"
                onChange={handleDisasmChange}
                className={styles.disasm}
                data-testid="disasm-checkbox"
            />
            <Checkbox
                checked={verifier}
                label="Verifier"
                onChange={handleVerifChange}
                className={styles.disasm}
                data-testid="verifier-checkbox"
            />
        </div>
    );
};

export default ControlPanel;
