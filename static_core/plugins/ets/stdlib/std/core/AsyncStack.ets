/**
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

package std.core;

/**
 * Runtime identity of one synchronous frame in an asynchronous stack.
 *
 * The identity is captured when a continuation is created. The Inspector resolves
 * it to script coordinates only when a paused event is serialized.
 */
final class AsyncStackSourceIdentity {
    private pandaFile?: String;
    private methodId: long = 0;
    private bytecodeOffset: int = 0;
    private nativePc: long = 0;
}

/** One immutable frame in an asynchronous stack segment. */
final class AsyncStackFrame {
    private functionName?: String;
    private sourceIdentity?: AsyncStackSourceIdentity;
}

/** An immutable group of frames captured at one asynchronous boundary. */
final class AsyncStackSegment {
    private description?: String;
    private generation: long = 0;
    private frames?: AsyncStackFrame[];
}

/** An immutable, physically bounded chain of asynchronous stack segments. */
final class AsyncStackSnapshot {
    private generation: long = 0;
    private segments?: AsyncStackSegment[];
}
