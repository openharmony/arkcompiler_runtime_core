/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

// This is a test common file on JS VM side

export namespace interop {

globalThis.test = {};
globalThis.test.etsVm = requireNapiPreview('ets_interop_js_napi', true);
if (!globalThis.test.etsVm.createRuntime({
    'log-level': 'debug',
	'log-components': 'ets_interop_js:gc_trigger',
	'boot-panda-files': 'etsstdlib.abc:gc_test_sts_common.abc',
	'gc-trigger-type': 'heap-trigger',
	'compiler-enable-jit': 'false',
    'coroutine-workers-count': '1'
})) {
    throw new Error('Failed to create ETS runtime');
}

let RunJsGC: () => void = () => {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    print('--- Start JS GC Full ---');
    let gcId = globalThis.ArkTools.GC.startGC('full');
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
    print('--- Finish JS GC Full ---');
};
globalThis.test.RunJsGC = RunJsGC;

export let GetSTSObject: () => Object = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetSTSObject');
export let GetSTSArrayOfObjects: () => Object[] = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetSTSArrayOfObjects');
export let GetSTSObjectAndKeepIt: () => Object = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetSTSObjectAndKeepIt');
export let GetSTSObjectWithWeakRef: () => Object = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetSTSObjectWithWeakRef');
export let GetSTSStringArrayAsObject: () => Object = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetSTSStringArrayAsObject');
export let isSTSObjectCollected: () => boolean = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'isSTSObjectCollected');
export let isSTSPromiseCollected: () => boolean = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'isSTSPromiseCollected');
export let SetJSObject: (obj: Object) => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'SetJSObject');
export let SetJSObjectAndKeepIt: (obj: Object) => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'SetJSObjectAndKeepIt');
export let SetJSObjectWithWeakRef: (obj: Object) => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'SetJSObjectWithWeakRef');
export let SetJSPromiseWithWeakRef: (prms: Promise<Object>) => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'SetJSPromiseWithWeakRef');
export let AddPandaArray: (arr: Object[]) => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'AddPandaArray');
export let RunPandaGC: () => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'RunPandaGC');
let RunInteropGCInternal: () => void = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'RunInteropGC');
export let RunInteropGC: () => void = () => {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    RunInteropGCInternal();
};
export let GetPandaFreeHeapSize: () => number = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetPandaFreeHeapSize');
export let GetPandaUsedHeapSize: () => number = globalThis.test.etsVm.getFunction('Lgc_test_sts_common/ETSGLOBAL;', 'GetPandaUsedHeapSize');
export const PandaBaseClass = globalThis.test.etsVm.getClass('Lgc_test_sts_common/PandaBaseClass;');
}
