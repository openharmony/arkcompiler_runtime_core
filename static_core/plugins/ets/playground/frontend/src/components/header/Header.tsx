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

import React, { useRef, useState } from 'react';
import {Icon, Menu, MenuItem, Popover, Switch} from '@blueprintjs/core';
import {useTheme} from '../theme/ThemeContext';
import cx from 'classnames';
import styles from './styles.module.scss';
import { useClickOutside } from '../../utils/useClickOutside';

export const primaryColors = ['#e32b49', '#da70d6', '#0F6894', '#1C6E42', '#935610', '#634DBF', '#00A396', '#D1980B'];

const exampleMenu = (theme: string, toggleTheme: () => void, togglePrimaryColor: (val: string) => void): JSX.Element => (
    <Menu className={styles.menu}>
        <MenuItem
            icon="moon"
            text="Dark mode"
            className={styles.menuItem}
            labelElement={
                <Switch
                    onChange={(e: React.ChangeEvent<HTMLInputElement>): void => {
                        e.stopPropagation();
                    }}
                    onClick={(e: React.MouseEvent<HTMLInputElement, MouseEvent>): void => {
                        e.stopPropagation();
                        e.preventDefault();
                    }}
                    checked={theme === 'dark'}
                    className={styles.switch}
                />
            }
            onClick={toggleTheme}
        />
        <div className={styles.colorContainer}>
            {primaryColors.map(color => (
                <div
                    data-testid={color}
                    className={styles.color}
                    style={{backgroundColor: color}}
                    key={color}
                    onClick={(): void => togglePrimaryColor(color)}
                />))}
        </div>
    </Menu>
);

const Header = (): JSX.Element => {
    const popoverRef = useRef<HTMLDivElement | null>(null);
    const { theme, toggleTheme, togglePrimaryColor } = useTheme();

    const [isOpen, setIsOpen] = useState<boolean>(false);

    const handleClosePopover = (): void => {
        setIsOpen(false);
    };
    const handleOpenPopover = (): void => {
        setIsOpen(true);
    };

    useClickOutside(popoverRef, handleClosePopover);

    return (
        <div
            className={cx({
                [styles.dark]: theme === 'dark',
                [styles.container]: true,
            })}
            data-testid="header-component"
        >
            <span className={cx({[styles.title]: true})}>ArkTS playground</span>
            <Popover
                content={exampleMenu(theme, toggleTheme, togglePrimaryColor)}
                placement="bottom"
                className={styles.settings}
                popoverClassName={styles.pop}
                isOpen={isOpen}
                popoverRef={popoverRef}
            >
                <div
                    data-testid="settings-icon"
                    onClick={handleOpenPopover}
                >
                    <Icon icon={'cog'} />
                </div>
            </Popover>
        </div>);
};

export default Header;
