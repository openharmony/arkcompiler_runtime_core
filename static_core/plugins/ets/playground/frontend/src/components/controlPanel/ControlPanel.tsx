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

import React, {useRef, useState} from 'react';
import {Button, Checkbox, Icon, Popover} from '@blueprintjs/core';
import styles from './styles.module.scss';
import CompileOptions from '../../pages/compileOptions/CompileOptions';
import {useDispatch, useSelector} from 'react-redux';
import {withDisasm} from '../../store/selectors/appState';
import {AppDispatch} from '../../store';
import {setDisasmAction} from '../../store/actions/appState';
import {fetchCompileCode, fetchRunCode} from '../../store/actions/code';
import {selectCompileLoading, selectRunLoading} from '../../store/selectors/code';
import {useClickOutside} from '../../utils/useClickOutside';

const ControlPanel = (): JSX.Element => {
    const popoverRef = useRef<HTMLDivElement | null>(null);
    const [isOpen, setIsOpen] = useState<boolean>(false);
    const disasm = useSelector(withDisasm);
    const dispatch = useDispatch<AppDispatch>();
    const isCompileLoading = useSelector(selectCompileLoading);
    const isRunLoading = useSelector(selectRunLoading);

    const handleDisasmChange = (): void => {
        dispatch(setDisasmAction(!disasm));
    };
    const handleCompile = (): void => {
        dispatch(fetchCompileCode());
    };
    const handleRun = (): void => {
        dispatch(fetchRunCode());
    };
    const handleClosePopover = (): void => {
        setIsOpen(false);
    };
    const handleOpenPopover = (): void => {
        setIsOpen(true);
    };
    useClickOutside(popoverRef, handleClosePopover);
    return (
        <div className={styles.container}>
            <Button
                icon={isCompileLoading
                    ? <div className={styles.icon}><Icon icon="build" size={11} /></div>
                    : <div className={styles.icon}><Icon icon="code-block" size={11}/></div>}
                className={styles.btn}
                onClick={handleCompile}
                disabled={isCompileLoading}
            >
                Compile
            </Button>
            <Button
                icon={isRunLoading
                    ? <div className={styles.icon}><Icon icon="build" size={11}/></div>
                    : <div className={styles.icon}><Icon icon="play" size={16}/></div>}
                className={styles.btn}
                onClick={handleRun}
                disabled={isRunLoading}
            >
                Run
            </Button>
            <Checkbox
                checked={disasm}
                label="Disasm"
                onChange={handleDisasmChange}
                className={styles.disasm}
                data-testid="disasm-checkbox"
            />
            <Popover
                content={<CompileOptions onClose={handleClosePopover} />}
                interactionKind="click"
                popoverClassName={styles.pop}
                isOpen={isOpen}
                popoverRef={popoverRef}
            >
                <div className={styles.popoverContainer} onClick={handleOpenPopover}>
                    Compile options
                    <Icon icon='chevron-down' />
                </div>
            </Popover>
        </div>
    );
};

export default ControlPanel;
