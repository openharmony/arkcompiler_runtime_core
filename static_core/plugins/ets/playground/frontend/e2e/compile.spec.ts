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

test.describe('Compilation', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test('should compile code successfully and show output', async ({ page }) => {
        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByText('Compile successful')).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should run code successfully and show Hello ArkTS output', async ({ page }) => {
        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await page.getByRole('tab', { name: 'Execute' }).click();
        await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should compile code with exit code 0', async ({ page }) => {
        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByText('exit code: 0')).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should auto-switch to Execute tab when Run is clicked', async ({ page }) => {
        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByRole('tab', { name: 'Execute' })).toHaveAttribute('aria-selected', 'true');
        await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should auto-switch to Compile tab when Compile is clicked', async ({ page }) => {
        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByRole('tab', { name: 'Execute' })).toHaveAttribute('aria-selected', 'true');

        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByRole('tab', { name: 'Compile' })).toHaveAttribute('aria-selected', 'true');
        await expect(page.getByText('Compile successful')).toBeVisible({ timeout: TIMEOUT.ui });
    });
});
