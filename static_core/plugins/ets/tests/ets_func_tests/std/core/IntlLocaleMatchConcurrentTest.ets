/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

const enUS: string = 'en-US'
const zhCN: string = 'zh-CN'
const ruRU: string = 'ru-RU'
const jaJP: string = 'ja-JP'
const deDE: string = 'de-DE'
const frFR: string = 'fr-FR'
const koKR: string = 'ko-KR'
const esES: string = 'es-ES'
const itIT: string = 'it-IT'
const ptBR: string = 'pt-BR'
const localeList: string[] = [ruRU, zhCN, enUS]

const localeSets: string[][] = [
    [enUS, zhCN, ruRU],
    [jaJP, deDE, frFR],
    [koKR, esES, itIT],
    [enUS, jaJP, deDE],
    [zhCN, frFR, ptBR],
    [ruRU, koKR, esES],
    [jaJP, frFR, itIT],
    [deDE, enUS, ptBR],
    [zhCN, jaJP, koKR],
    [ruRU, deDE, esES],
    [frFR, itIT, enUS],
    [koKR, ptBR, zhCN],
    [esES, enUS, jaJP],
    [itIT, ruRU, deDE],
    [ptBR, frFR, koKR],
    [enUS, esES, itIT]
]

// Each worker writes only to its own slot — no shared mutable state, no lock needed
function collectErrors(errorSlots: string[], count: int): string[] {
    let errors: string[] = []
    for (let i = 0; i < count; i++) {
        if (errorSlots[i].length > 0) {
            errors.push(errorSlots[i])
        }
    }
    return errors
}

function verifyLookupResults(threadId: int): void {
    let single = Intl.intlLookUpLocale(enUS)
    if (single != enUS) {
        throw new Error('thread ' + threadId.toString() + ' lookupLocale expected ' + enUS + ' got ' + single)
    }

    let result = Intl.intlLookUpLocales(localeList)
    if (result.length != 3) {
        throw new Error('thread ' + threadId.toString() + ' lookupLocales length expected 3 got ' + result.length.toString())
    }
    if (result[0] != ruRU) {
        throw new Error('thread ' + threadId.toString() + ' lookupLocales[0] expected ' + ruRU + ' got ' + result[0])
    }
    if (result[1] != zhCN) {
        throw new Error('thread ' + threadId.toString() + ' lookupLocales[1] expected ' + zhCN + ' got ' + result[1])
    }
    if (result[2] != enUS) {
        throw new Error('thread ' + threadId.toString() + ' lookupLocales[2] expected ' + enUS + ' got ' + result[2])
    }
}

function verifyBestFitResults(threadId: int): void {
    let result = Intl.intlBestFitLocales(localeList)
    if (result.length != 3) {
        throw new Error('thread ' + threadId.toString() + ' bestFitLocales length expected 3 got ' + result.length.toString())
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite('Intl LocaleMatch Concurrent Test')

    suite.addTest('Concurrent lookup from multiple EAWorkers', testConcurrentLookupFromWorkers)
    suite.addTest('Concurrent lookup from main and workers', testConcurrentLookupMixed)
    suite.addTest('Concurrent first-call initialization', testConcurrentInit)
    suite.addTest('Concurrent different locales per thread', testConcurrentDifferentLocalesPerThread)
    suite.addTest('Concurrent mixed lookup and bestfit paths', testConcurrentMixedLookupBestFit)
    suite.addTest('Concurrent cache consistency across threads', testConcurrentCacheConsistency)

    return suite.run()
}

function testConcurrentLookupFromWorkers(): void {
    const workerCount = 16
    const iterations = 50
    let workers: EAWorker[] = []
    let errorSlots: string[] = []

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let w = new EAWorker(() => {
            try {
                for (let j = 0; j < iterations; j++) {
                    verifyLookupResults(idx)
                    verifyBestFitResults(idx)
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    // quit() = join().Await() — blocks until worker fully exits and releases coroutine quota
    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}

function testConcurrentLookupMixed(): void {
    const workerCount = 16
    const iterations = 30
    let workers: EAWorker[] = []
    let errorSlots: string[] = []

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let w = new EAWorker(() => {
            try {
                for (let j = 0; j < iterations; j++) {
                    let r = Intl.intlLookUpLocales([jaJP, deDE, frFR])
                    if (r.length != 3) {
                        throw new Error('worker ' + idx.toString() + ' length expected 3 got ' + r.length.toString())
                    }
                    if (r[0] != jaJP) {
                        throw new Error('worker ' + idx.toString() + ' [0] expected ' + jaJP + ' got ' + r[0])
                    }
                    if (r[1] != deDE) {
                        throw new Error('worker ' + idx.toString() + ' [1] expected ' + deDE + ' got ' + r[1])
                    }
                    if (r[2] != frFR) {
                        throw new Error('worker ' + idx.toString() + ' [2] expected ' + frFR + ' got ' + r[2])
                    }
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    // Main thread also does work concurrently
    for (let j = 0; j < iterations; j++) {
        let r = Intl.intlLookUpLocales([jaJP, deDE, frFR])
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], jaJP)
        arktest.assertEQ(r[1], deDE)
        arktest.assertEQ(r[2], frFR)
    }

    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}

function testConcurrentInit(): void {
    // Test concurrent first-call to trigger static initialization
    const workerCount = 16
    let workers: EAWorker[] = []
    let errorSlots: string[] = []

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let w = new EAWorker(() => {
            try {
                let r = Intl.intlLookUpLocales([enUS, zhCN])
                if (r.length != 2) {
                    throw new Error('worker ' + idx.toString() + ' length expected 2 got ' + r.length.toString())
                }
                if (r[0] != enUS) {
                    throw new Error('worker ' + idx.toString() + ' [0] expected ' + enUS + ' got ' + r[0])
                }
                if (r[1] != zhCN) {
                    throw new Error('worker ' + idx.toString() + ' [1] expected ' + zhCN + ' got ' + r[1])
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}

function testConcurrentDifferentLocalesPerThread(): void {
    // Each worker uses a different locale set — verify no cross-thread contamination from cache
    const workerCount = 16
    const iterations = 40
    let workers: EAWorker[] = []
    let errorSlots: string[] = []

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let myLocales = localeSets[idx]
        let w = new EAWorker(() => {
            try {
                for (let j = 0; j < iterations; j++) {
                    let lookupResult = Intl.intlLookUpLocales(myLocales)
                    if (lookupResult.length != myLocales.length) {
                        throw new Error('worker ' + idx.toString() + ' lookup length expected ' +
                                        myLocales.length.toString() + ' got ' + lookupResult.length.toString())
                    }
                    for (let k = 0; k < myLocales.length; k++) {
                        if (lookupResult[k] != myLocales[k]) {
                            throw new Error('worker ' + idx.toString() + ' lookup[' + k.toString() +
                                            '] expected ' + myLocales[k] + ' got ' + lookupResult[k])
                        }
                    }
                    let bestFitResult = Intl.intlBestFitLocales(myLocales)
                    if (bestFitResult.length != myLocales.length) {
                        throw new Error('worker ' + idx.toString() + ' bestFit length expected ' +
                                        myLocales.length.toString() + ' got ' + bestFitResult.length.toString())
                    }
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}

function testConcurrentMixedLookupBestFit(): void {
    // Odd workers do lookup, even workers do bestfit — verify both paths use cache correctly
    const workerCount = 16
    const iterations = 50
    let workers: EAWorker[] = []
    let errorSlots: string[] = []
    let myLocales: string[] = [enUS, zhCN, ruRU, jaJP, deDE]

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let doLookup = (idx % 2 == 0)
        let w = new EAWorker(() => {
            try {
                for (let j = 0; j < iterations; j++) {
                    if (doLookup) {
                        let r = Intl.intlLookUpLocales(myLocales)
                        if (r.length != myLocales.length) {
                            throw new Error('worker ' + idx.toString() + ' lookup length expected ' +
                                            myLocales.length.toString() + ' got ' + r.length.toString())
                        }
                    } else {
                        let r = Intl.intlBestFitLocales(myLocales)
                        if (r.length != myLocales.length) {
                            throw new Error('worker ' + idx.toString() + ' bestFit length expected ' +
                                            myLocales.length.toString() + ' got ' + r.length.toString())
                        }
                    }
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}

function testConcurrentCacheConsistency(): void {
    // Verify repeated calls from different threads always return consistent results
    const workerCount = 16
    const iterations = 60
    let workers: EAWorker[] = []
    let errorSlots: string[] = []
    let myLocales: string[] = [enUS, zhCN, ruRU, jaJP, deDE, frFR, koKR, esES]

    for (let i = 0; i < workerCount; i++) {
        errorSlots.push('')
        let idx = i
        let w = new EAWorker(() => {
            try {
                // First call — may trigger cache initialization
                let firstLookup = Intl.intlLookUpLocales(myLocales)
                let firstBestFit = Intl.intlBestFitLocales(myLocales)

                for (let j = 0; j < iterations; j++) {
                    let lookupResult = Intl.intlLookUpLocales(myLocales)
                    let bestFitResult = Intl.intlBestFitLocales(myLocales)

                    // Results must be identical across all iterations
                    if (lookupResult.length != firstLookup.length) {
                        throw new Error('worker ' + idx.toString() + ' lookup length changed: expected ' +
                                        firstLookup.length.toString() + ' got ' + lookupResult.length.toString())
                    }
                    for (let k = 0; k < firstLookup.length; k++) {
                        if (lookupResult[k] != firstLookup[k]) {
                            throw new Error('worker ' + idx.toString() + ' lookup[' + k.toString() +
                                            '] changed: expected ' + firstLookup[k] + ' got ' + lookupResult[k])
                        }
                    }
                    if (bestFitResult.length != firstBestFit.length) {
                        throw new Error('worker ' + idx.toString() + ' bestFit length changed: expected ' +
                                        firstBestFit.length.toString() + ' got ' + bestFitResult.length.toString())
                    }
                    for (let k = 0; k < firstBestFit.length; k++) {
                        if (bestFitResult[k] != firstBestFit[k]) {
                            throw new Error('worker ' + idx.toString() + ' bestFit[' + k.toString() +
                                            '] changed: expected ' + firstBestFit[k] + ' got ' + bestFitResult[k])
                        }
                    }
                }
            } catch (err) {
                errorSlots[idx] = 'worker ' + idx.toString() + ': ' + err.message
            }
        })
        workers.push(w)
    }

    // Start all workers at once to maximize concurrency
    for (let i = 0; i < workerCount; i++) {
        workers[i].start()
    }

    for (let i = 0; i < workerCount; i++) {
        workers[i].quit()
    }

    let errors = collectErrors(errorSlots, workerCount)
    arktest.assertEQ(errors.length, 0)
}
