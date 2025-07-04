/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package std.concurrency;

type CoroFunT<A, R> = (
  a0: A,
  a1: A,
  a2: A,
  a3: A,
  a4: A,
  a5: A,
  a6: A,
  a7: A,
  a8: A,
  a9: A,
  a10: A,
  a11: A,
  a12: A,
  a13: A,
  a14: A,
  a15: A
) => R;

/**
 * This function is used to launch new coroutines.
 *
 * @param coroFun entrypoint of the the new coroutine.
 * @param args arguments for the function coroFun.
 *
 * @returns instance of the Job class that represents newly launched coroutine.
 */
export function launch<R, F extends CoroFunT<never, R>>(coroFun: F, ...args: FixedArray<NullishType>): Job<R> {
  return launchInternal<R, F>(coroFun, args as FixedArray<NullishType>);
}

/**
 * @interface LaunchParams Interface for coroutine launch params.
 */
export interface LaunchParams {
  /** @property { boolean } abortFlag if true, aborts the program if unhandled exception were thrown inside the coroutine. */
  abortFlag: boolean;
}

/**
 * Launch new coroutine with LaunchParams
 *
 * @param coroFun entrypoint of the the new coroutine.
 * @param launchParams parameters of the new coroutine.
 * @param args arguments for the function coroFun.
 *
 * @returns instance of the Job class that represents newly launched coroutine.
 */
export function launch<R, F extends CoroFunT<never, R>>(coroFun: F, launchParams: LaunchParams, ...args: FixedArray<NullishType>): Job<R> {
    return launchInternal<R, F>(coroFun, args as FixedArray<NullishType>, launchParams);
}

function launchInternal<R, F extends CoroFunT<never, R>>(coroFun: F, args: FixedArray<NullishType>, launchParams?: LaunchParams): Job<R> {
  const paddedArgs = padArrayToMaxArgs(args);
  if (launchParams) {
    return launchInternalJobNative<F, R>(coroFun, args as FixedArray<NullishType>, launchParams.abortFlag);
  }
  return launchInternalJobNative<F, R>(coroFun, args as FixedArray<NullishType>, false);
}

function padArrayToMaxArgs<T>(arr: FixedArray<T>): FixedArray<(T | undefined)> {
  const MAX_ARGS = 16;
  const res: FixedArray<(T | undefined)> = new (T | undefined)[MAX_ARGS];
  for (let i = 0; i < arr.length && i < MAX_ARGS; ++i) {
    res[i] = arr[i];
  }
  for (let i = arr.length; i < MAX_ARGS; ++i) {
    res[i] = undefined;
  }
  return res;
}

native function launchInternalJobNative<F, R>(coroFun: F, args: FixedArray<NullishType>, abortFlag: boolean): Job<R>;
