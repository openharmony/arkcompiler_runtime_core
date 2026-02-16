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

import { test, expect, setupPage, TIMEOUT } from './fixtures';

test.describe('Compile Options', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test('should open options popover and change opt-level', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();
        await expect(page.getByText('Compile options')).toBeVisible();

        await page.getByTestId('radio-2').click({ force: true });
        await page.getByTestId('compile-options-save-btn').click();

        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByText('Compile successful')).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should reset compile options', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();
        await expect(page.getByText('Compile options')).toBeVisible();

        await page.getByTestId('radio-2').click({ force: true });
        await page.getByTestId('compile-options-save-btn').click();

        await page.getByRole('button', { name: 'Options' }).click();
        await page.getByRole('button', { name: 'Reset' }).first().click();

        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByText('Compile successful')).toBeVisible({ timeout: TIMEOUT.ui });
    });
});

test.describe('Run Options', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test('should enable AOT mode and run code', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();
        await expect(page.getByText('Run options')).toBeVisible();

        await page.getByText('AOT mode').click();
        await page.getByTestId('run-options-save-btn').click();

        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });

        await page.getByRole('tab', { name: 'Execute' }).click();
        await expect(page.getByText('[AOT]')).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should change verification mode to Ahead of Time', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();
        await expect(page.getByText('Verification mode:')).toBeVisible();

        await page.getByRole('radio', { name: 'Ahead of Time' }).click({ force: true });
        await page.getByTestId('run-options-save-btn').click();

        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await page.getByRole('tab', { name: 'Execute' }).click();
        await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should enable IR Dump Compiler option and show panel', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();

        await page.getByText('Compiler Dump').click();
        await page.getByTestId('run-options-save-btn').click();

        await expect(page.getByText('Compiler IR Dump').first()).toBeVisible({ timeout: TIMEOUT.quick });

        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
    });

    test('should enable IR Dump Disasm option and show panel', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();

        await page.getByText('Disasm Dump').click();
        await page.getByTestId('run-options-save-btn').click();

        await expect(page.getByRole('heading', { name: 'Disasm Dump' })).toBeVisible({ timeout: TIMEOUT.quick });

        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
    });

    test('should reset run options', async ({ page }) => {
        await page.getByRole('button', { name: 'Options' }).click();
        await page.getByText('AOT mode').click();
        await page.getByTestId('run-options-save-btn').click();

        await page.getByRole('button', { name: 'Options' }).click();
        await page.getByRole('button', { name: 'Reset' }).last().click();

        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await page.getByRole('tab', { name: 'Execute' }).click();
        await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });
    });
});
