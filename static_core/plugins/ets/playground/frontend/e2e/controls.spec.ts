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

test.describe('Control Panel E2E', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test('should show AST viewer panel when AST View is enabled', async ({ page }) => {
        await page.getByText('AST View').click();
        await expect(page.getByRole('heading', { name: 'AST viewer' })).toBeVisible();
    });

    test('should show disassembly panel when Disasm is enabled', async ({ page }) => {
        await page.getByText('Disasm').first().click();
        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await expect(page.getByText('.language eTS')).toBeVisible({ timeout: TIMEOUT.ui });
    });

    test('should show verifier output when Verifier is enabled', async ({ page }) => {
        await page.getByText('Verifier').first().click();
        await page.getByRole('button', { name: 'Compile' }).click();
        await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
        await page.getByRole('tab', { name: 'Compile' }).click();
        await expect(page.getByTestId('logs-container-compilation').getByText('Verifier')).toBeVisible();
    });
});
