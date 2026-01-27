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

import { test as base, expect, Page } from '@playwright/test';

// API URL for E2E tests - points to local backend
export const TEST_API_URL = 'http://localhost:8000';

// Timeouts for E2E tests
export const TIMEOUT = {
    compile: 30000,  // Compilation/run operations
    ui: 10000,       // UI updates
    quick: 5000,     // Quick checks
};

// Setup page with mocked API URL
export async function setupPage(page: Page): Promise<void> {
    await page.route('**/env.js', async (route) => {
        await route.fulfill({
            contentType: 'application/javascript',
            body: `window.runEnv = { apiUrl: '${TEST_API_URL}' };`,
        });
    });

    await page.goto('/');
    await expect(page.getByText('ArkTS playground')).toBeVisible({ timeout: TIMEOUT.ui });
}

// Helper to type code in Monaco editor
export async function typeInEditor(page: Page, code: string): Promise<void> {
    const editor = page.locator('.monaco-editor textarea').first();
    await editor.focus();
    await page.waitForTimeout(100);
    // Select all and delete, then type new code
    await page.keyboard.press('ControlOrMeta+a');
    await page.waitForTimeout(50);
    await page.keyboard.press('Backspace');
    await page.waitForTimeout(50);
    await page.keyboard.type(code, { delay: 5 });
}

// Re-export test and expect from Playwright
export const test = base;
export { expect };
