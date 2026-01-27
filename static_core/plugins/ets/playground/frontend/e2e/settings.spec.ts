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

test.describe('Settings', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test('should toggle dark mode', async ({ page }) => {
        await page.getByTestId('settings-icon').click();
        await expect(page.getByText('Dark mode')).toBeVisible();

        await page.getByText('Dark mode').click();

        await expect(page.locator('body')).toHaveClass(/dark-theme/);
    });

    test('should toggle auto clear logs', async ({ page }) => {
        await page.getByTestId('settings-icon').click();
        await expect(page.getByText('Auto clear logs')).toBeVisible();

        await page.getByText('Auto clear logs').click();
        await page.keyboard.press('Escape');

        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await page.getByRole('tab', { name: 'Execute' }).click();
        await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });

        await page.getByRole('button', { name: 'Run' }).click();
        await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });

        const logMessages = page.getByTestId('log-message');
        await expect(logMessages).toHaveCount(2, { timeout: TIMEOUT.ui });
    });

    test('should display version information', async ({ page }) => {
        await page.getByTestId('settings-icon').click();

        await expect(page.getByText('frontend:')).toBeVisible();
        await expect(page.getByText('backend:')).toBeVisible();
        await expect(page.getByText('ArkTS:')).toBeVisible();
        await expect(page.getByText('es2panda:')).toBeVisible();
    });

    test('should have docs link', async ({ page }) => {
        const docsLink = page.getByTestId('docs-link');
        await expect(docsLink).toBeVisible();
        await expect(docsLink).toHaveAttribute('href', /gitcode\.com/);
    });
});
