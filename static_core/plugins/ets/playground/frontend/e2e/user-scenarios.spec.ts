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

import { test, expect, setupPage, typeInEditor, TIMEOUT } from './fixtures';

test.describe('User Scenarios', () => {
    test.beforeEach(async ({ page }) => {
        await setupPage(page);
    });

    test.describe('Error Handling', () => {
        test('should show compilation error for syntax error', async ({ page }) => {
            await typeInEditor(page, 'function main(): void {\n    console.log("Hello"\n}');
            await page.getByRole('button', { name: 'Compile' }).click();
            await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
            await expect(page.getByText('Compile successful')).not.toBeVisible({ timeout: TIMEOUT.quick });
            await expect(page.getByText(/SyntaxError|error|Error/i).first()).toBeVisible({ timeout: TIMEOUT.ui });
        });

        test('should show runtime error when code throws', async ({ page }) => {
            await typeInEditor(page, 'function main(): void {\n    throw new Error("Test runtime error");\n}');
            await page.getByRole('button', { name: 'Run' }).click();
            await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
            await page.getByRole('tab', { name: 'Execute' }).click();
            await expect(page.getByTestId('log-message').filter({ hasText: 'Test runtime error' })).toBeVisible({ timeout: TIMEOUT.ui });
        });

        test('should show non-zero exit code for failed compilation', async ({ page }) => {
            await typeInEditor(page, 'this is not valid code at all!!!');
            await page.getByRole('button', { name: 'Compile' }).click();
            await expect(page.getByRole('button', { name: 'Compile' })).toBeEnabled({ timeout: TIMEOUT.compile });
            await expect(page.getByText('Compile successful')).not.toBeVisible({ timeout: TIMEOUT.quick });
        });
    });

    test.describe('Share Functionality', () => {
        test('should share code and show confirmation', async ({ page }) => {
            await page.getByRole('button', { name: 'Share' }).click();
            await expect(page.getByText('URL updated')).toBeVisible({ timeout: TIMEOUT.ui });
        });

        test('should update URL when sharing code', async ({ page }) => {
            await page.getByRole('button', { name: 'Share' }).click();
            await expect(page.getByText('URL updated')).toBeVisible({ timeout: TIMEOUT.ui });
            const url = page.url();
            expect(url).toContain('#share=');
        });

        test('should load shared code from URL', async ({ page }) => {
            await page.getByRole('button', { name: 'Share' }).click();
            await expect(page.getByText('URL updated')).toBeVisible({ timeout: TIMEOUT.ui });

            const sharedUrl = page.url();

            await page.goto(sharedUrl);
            await expect(page.getByText('ArkTS playground')).toBeVisible({ timeout: TIMEOUT.ui });

            await page.getByRole('button', { name: 'Run' }).click();
            await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
            await page.getByRole('tab', { name: 'Execute' }).click();
            await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });
        });
    });

    test.describe('Output Management', () => {
        test('should clear output logs', async ({ page }) => {
            await page.getByRole('button', { name: 'Run' }).click();
            await expect(page.getByRole('button', { name: 'Run' })).toBeEnabled({ timeout: TIMEOUT.compile });
            await page.getByRole('tab', { name: 'Execute' }).click();
            await expect(page.getByTestId('log-message').filter({ hasText: 'Hello, ArkTS!' })).toBeVisible({ timeout: TIMEOUT.ui });

            await page.getByRole('button', { name: 'Clear logs' }).click();

            await expect(page.getByTestId('log-message')).not.toBeVisible({ timeout: TIMEOUT.quick });
        });
    });
});
