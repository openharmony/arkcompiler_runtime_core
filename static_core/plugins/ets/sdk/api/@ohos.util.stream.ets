/*
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

import { BusinessError } from '@ohos.base';
import { util } from './@ohos.util';

type CallbackFnType = (...params: Object[]) => void;

/**
 * The stream module provides a comprehensive set of stream processing capabilities, including four types of streams:
 * - Writable: streams designed for writing data to.
 * - Readable: streams designed for reading data from.
 * - Duplex: streams that are both readable and writable.
 * - Transform: a specialized type of duplex stream that can modify or transform data as it's being written and read.
 *
 */
export namespace stream {
    const DEFAULT_ENCODING = 'utf-8';
    const DEFAULT_HIGH_WATER_MARK = 16 * 1024;
    const ENCODING_SET: Array<string> = new Array<string>(
        'ascii', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le');

    // BusinessError
    const ERR_INCORRECT_PARAMETER: BusinessError =
        createBusinessError(401, 'Parameter error. Incorrect parameter.');
    const ERR_DOWRITE_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200035, 'The doWrite() method is not implemented');
    const ERR_WRITE_AFTER_END: BusinessError =
        createBusinessError(10200036, 'write after end');
    const ERR_MULTIPLE_CALLBACK: BusinessError =
        createBusinessError(10200037, 'Callback called multiple times');
    const ERR_DOREAD_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200038, 'The doRead() method is not implemented');
    const ERR_DOTRANSFORM_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200039, 'The doTransform() method is not implemented');

    enum WritableEvent {
        CLOSE = 'close',
        DRAIN = 'drain',
        ERROR = 'error',
        FINISH = 'finish',
        PIPE = 'pipe',
        UNPIPE = 'unpipe',
    }

    enum ReadableEvent {
        CLOSE = 'close',
        DATA = 'data',
        END = 'end',
        ERROR = 'error',
        PAUSE = 'pause',
        READABLE = 'readable',
        RESUME = 'resume',
    }

    function createBusinessError(code: number, message: string) {
        let err = new BusinessError();
        err.code = code;
        err.name = 'BusinessError';
        err.message = message;
        return err;
    }

    function asyncFn(callback: () => void): void {
        setTimeout((): void => {
            callback();
        });
    }

    interface WritableBuffer {
        encoding?: string;
        chunk: string | Uint8Array;
        callback: Function;
    }

    export interface ReadablePipeStream {
        write: Writable;
        dataCallback: CallbackFnType;
        drainCallback: CallbackFnType;
        endCallback: CallbackFnType;
    }

    /**
     * Return readable options.
     *
     */
    export interface ReadableOptions {
        /**
         * Specifies the encoding format of the data. If this parameter is provided,
         * the readable stream decodes the data into a string in the specified encoding format. Default: utf8.
         * If an invalid string is entered, a 401 exception is thrown in the Readable constructor.
         * Supported encoding formats: utf-8, ibm866, iso-8859-2, iso-8859-3, iso-8859-4, iso-8859-5, iso-8859-6,
         * iso-8859-7, iso-8859-8, iso-8859-8-i, iso-8859-10, iso-8859-13, iso-8859-14, iso-8859-15, koi8-r, koi8-u,
         * macintosh, windows-874, windows-1250, windows-1251, windows-1252, windows-1253, windows-1254, windows-1255,
         * windows-1256, windows-1257, windows-1258, x-mac-cyrillic, gbk, gb18030, big5, euc-jp, iso-2022-jp, shift_jis,
         * euc-kr, utf-16be, utf-16le.
         *
         */
        encoding?: string;
        highWatermark?: number;
        doRead?: (size: number) => void;
    }

    /**
     * EventEmitter to which event callbacks can be held.
     *
     */
    class EventEmitter {
        public handlers: Map<string, Array<Function>>;

        /**
         * The EventEmitter constructor.
         *
         */
        constructor() {
            this.handlers = new Map<string, Array<Function>>();
        }

        /**
         * Registering event messages.
         *
         * @param { string } event - Register event.
         * @param { Function } callback - Event callbacks.
         */
        on(event: string, callback: Function): void {
            if (!this.handlers.has(event)) {
                const funcList = new Array<Function>();
                this.handlers.set(event, funcList);
            }
            const funcList = this.handlers.get(event);
            funcList!.push(callback);
        }

        /**
         * Unregistering event messages.
         *
         * @param { string } event - Unregister event.
         * @param { Function } callback - Event callbacks.
         */
        off(event: string, callback: Function): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                const pos = funcList!.findIndex((element) => element === callback);
                if (pos !== -1) {
                    funcList!.splice(pos, 1);
                }
            }
        }

        /**
         * Clear event messages.
         *
         * @param { string } event - Clear event.
         */
        clear(event: string): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                funcList!.splice(0);
            }
        }

        /**
         * Emit event messages.
         *
         * @param { string } event - Emit event.
         * @param { Object } [param] - The parameter of event callbacks.
         */
        emit(event: string, param?: Object): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                funcList!.forEach((callback: Function) => {
                    if (param !== undefined) {
                        callback.unsafeCall(param);
                    } else {
                        callback.unsafeCall();
                    }
                })
            }
        }

        /**
         * Judge the event is on or not.
         *
         * @param { string } event - The event to be judged is on or not.
         * @returns { boolean } Event is on returns true, event is off returns false.
         */
        isOn(event: string): boolean {
            return this.handlers.has(event) ? this.handlers.get(event)!.length > 0 : false;
        }

        /**
         * Find the event count in the listener.
         *
         * @param { string } event - The event to be found its count in the listener.
         * @returns { number } The event count in the listener.
         */
        listenerCount(event: string): number {
            return this.handlers.has(event) ? this.handlers.get(event)!.length : 0;
        }
    }

    /**
     * Streams to which data can be written.
     *
     */
    export class Writable {
        public listener: EventEmitter | undefined;
        private buffer: Array<WritableBuffer> = new Array<WritableBuffer>();
        private writing: boolean = false;
        private encoding: string | undefined;
        protected encoder = new util.TextEncoder();
        private ending: boolean = false;
        private writableObjectModeInner: boolean | undefined;
        private writableHighWatermarkInner: number;
        private writableInner: boolean | undefined;
        private writableLengthInner: number | undefined;
        private writableBufferLength: number;
        private writableNeedDrainInner: boolean | undefined;
        private writableCorkedInner: number = 0;
        private writableEndedInner: boolean | undefined;
        private writableFinishedInner: boolean | undefined;
        private erroredInner: Error | undefined | null;
        private closedInner: boolean | undefined;
        private writableCb: number = 0;
        private writableSync: boolean = true;
        private isDoWritevOverride: boolean = true;
        private defaultEncoding: string | undefined;
        private endCallback: Function | undefined;
        private writeCallbackBuffer: Array<Function> = new Array<Function>();
        private emitErrorExecutedInner: boolean = false;
        private emitErrorIdInner: number;
        doWriteFunc: ((chunk: string | Uint8Array, encoding: string, callback: Function) => void) | null;
        doWritevFunc: ((chunks: string[] | Uint8Array[], callback: Function) => void) | null;

        /**
         * The Writable constructor.
         *
         */
        constructor() {
            this.listener = new EventEmitter();
            this.writableHighWatermarkInner = DEFAULT_HIGH_WATER_MARK as number;
            this.writableObjectModeInner = false;
            this.writableLengthInner = 0;
            this.writableBufferLength = 0;
            this.writableEndedInner = false;
            this.writableNeedDrainInner = false;
            this.writableInner = true;
            this.writableCorkedInner = 0;
            this.writableFinishedInner = false;
            this.erroredInner = null;
            this.encoding = 'utf-8';
            this.closedInner = false;
            this.writableCb = 0;
            this.writableSync = true;
            this.defaultEncoding = 'utf-8';
            this.doWriteFunc = this.noWriteOpes;
            this.doWritevFunc = null;
            this.doInitialize((...param: Object[]): void => {
                this.listener?.emit(WritableEvent.ERROR);
            });
        }

        private getChunkLength(chunk: string | Uint8Array): number {
            if (chunk instanceof Uint8Array) {
                return chunk.byteLength;
            } else {
                return (chunk as string).length;
            }
        }

        private setEncoding(encoding: string): boolean {
            let encodingLowCase = encoding.toLowerCase();
            if (encodingLowCase === 'utf8' || encodingLowCase === 'ascii') {
                encoding = 'utf-8';
                encodingLowCase = 'utf-8';
            }

            if (ENCODING_SET.indexOf(encodingLowCase) !== -1) {
                this.encoding = encodingLowCase;
                try {
                    this.encoder = new util.TextEncoder(encodingLowCase);
                } catch (e) {
                    this.throwError(e as Error);
                    return false;
                }
                return true;
            } else {
                this.throwError(createBusinessError(401, `Unknown encoding: ${encoding}`));
                return false;
            }
        }

        private writeUint8Array(chunk: Uint8Array, encoding?: string, callback?: Function): boolean {
            const chunkLength = this.getChunkLength(chunk);
            if (this.encoding !== this.defaultEncoding) {
                this.setEncoding(this.defaultEncoding!);
            }
            this.writableLengthInner = this.writableLengthInner! + chunkLength;
            const hasRemaining = this.writableLengthInner! < this.writableHighWatermark!;
            let executed = false;
            const fnBack = (...param: Object[]): void => {
                if (!executed) {
                    executed = true;
                    if (param.length > 0 && param[0] instanceof Error) {
                        this.writableInner = false;
                        this.throwError(param[0] as Error);
                        return;
                    }
                    callback?.unsafeCall(param);
                    this.writableLengthInner = this.writableLengthInner! - chunkLength;
                    this.writing = false;
                    this.writableCb --;
                    if (!this.writableSync) {
                        this.freshCacheV();
                    }
                    this.afterWrite();
                } else {
                    Promise.resolve().then((): void => {
                        this.listener?.emit(WritableEvent.ERROR, ERR_MULTIPLE_CALLBACK);
                    });
                }
            };
            if (this.writableCorkedInner === 0) {
                if (!this.writing) {
                    this.writing = true;
                    this.writableCb ++;
                    this.writableSync = true;
                    this.doWrite(chunk, encoding ?? 'utf8', fnBack);
                    this.writableSync = false;
                } else {
                    const buffInfo: WritableBuffer = {
                        encoding: encoding,
                        chunk: chunk,
                        callback: fnBack
                    };
                    this.buffer.push(buffInfo);
                    if (callback instanceof Function) {
                        this.writeCallbackBuffer.push(callback);
                    } else {
                        this.writeCallbackBuffer.push((): void => {});
                    }
                    this.writableBufferLength += chunkLength;
                }
            } else {
                const buffInfo: WritableBuffer = {
                    encoding: encoding,
                    chunk: chunk,
                    callback: fnBack
                };
                this.buffer.push(buffInfo);
                if (callback instanceof Function) {
                    this.writeCallbackBuffer.push(callback);
                } else {
                    this.writeCallbackBuffer.push((): void => {});
                }
                this.writableBufferLength += chunkLength;
            }
            return hasRemaining;
        }

        private writeString(chunk: string, encoding?: string, callback?: Function): boolean {
            const chunkLength = this.getChunkLength(chunk);
            if (this.encoding !== this.defaultEncoding) {
                this.setEncoding(this.defaultEncoding!);
            }
            this.writableLengthInner = this.writableLengthInner! + chunkLength;
            const hasRemaining = this.writableLengthInner! < this.writableHighWatermark!;
            let executed = false;
            const fb = (...param: Object[]): void => {
                if (!executed) {
                    executed = true;
                    if (param.length > 0) {
                        this.erroredInner = param[0] as Error;
                    }
                    callback?.unsafeCall(param);
                    this.writableLengthInner = this.writableLengthInner! - chunkLength;
                    this.writing = false;
                    this.writableCb --;
                    if (!this.writableSync) {
                        this.freshCacheV();
                    }
                    if (param.length > 0 && param[0] instanceof Error) {
                        this.writableInner = false;
                        this.erroredInner = param[0] as Error;
                        Promise.resolve().then((): void => {
                            if (this.isOnError()) {
                                this.emitErrorOnce(this.erroredInner!);
                            } else {
                                this.emitErrorOnce(this.erroredInner!);
                                throw this.erroredInner!;
                            }
                        });
                        return;
                    }
                    this.afterWrite();
                } else {
                    Promise.resolve().then(():void => {
                        this.emitErrorOnce(ERR_MULTIPLE_CALLBACK, true);
                    });
                }
            };

            if (this.writableCorkedInner === 0) {
                if (!this.writing) {
                    this.writing = true;
                    this.writableCb ++;
                    this.writableSync = true;
                    this.doWrite(chunk, encoding ?? 'utf8', fb);
                    this.writableSync = false;
                } else {
                    const buffInfo: WritableBuffer = {
                        encoding: encoding,
                        chunk: chunk,
                        callback: fb
                    };
                    this.buffer.push(buffInfo);
                    if (callback instanceof Function) {
                        this.writeCallbackBuffer.push(callback);
                    } else {
                        this.writeCallbackBuffer.push((): void => {});
                    }
                    this.writableBufferLength += chunkLength;
                }
            } else {
                const buffInfo: WritableBuffer = {
                    encoding: encoding,
                    chunk: chunk,
                    callback: fb
                };
                this.buffer.push(buffInfo);
                if (callback instanceof Function) {
                    this.writeCallbackBuffer.push(callback);
                } else {
                    this.writeCallbackBuffer.push((): void => {});
                }
                this.writableBufferLength += chunkLength;
            }
            return this.erroredInner ? false : hasRemaining;
        }

        private freshCache(): void {
            if (this.writableCorkedInner != 0) {
                return;
            }
            let currentLength = this.buffer.length;
            let current = this.buffer.splice(0, 1);
            this.writeCallbackBuffer.splice(0, 1);
            while (currentLength > 0) {
                this.writableBufferLength -= this.getChunkLength(current[0].chunk);
                this.writing = true;
                this.writableCb ++;
                this.writableSync = true;
                this.doWrite(current[0].chunk, current[0].encoding ?? 'utf8', current[0].callback);
                this.writableSync = false;
                if (!this.writing) {
                    currentLength = this.buffer.length;
                    current = this.buffer.splice(0, 1);
                } else {
                    break;
                }
            }
            if (this.finishMayBe()) {
                this.finishWrite();
            }
        }

        private freshCacheV(): void {
            if (this.writableCorkedInner != 0) {
                return;
            }
            if (this.buffer.length > 0) {
                const bufferChunkLength = this.writableBufferLength;
                this.writableBufferLength = 0;
                let writeCallbacks: Array<Function> = new Array<Function>();
                let executed = false;
                const funCallback = (...param: Object[]): void => {
                    if (!executed) {
                        executed = true;
                        if (param.length > 0 && param[0] instanceof Error) {
                            this.erroredInner = param[0] as Error;
                            this.listener?.emit(WritableEvent.ERROR, param[0] as Error);
                            return;
                        }
                        this.writableLengthInner = this.writableLengthInner! - bufferChunkLength;
                        this.writing = false;
                        this.writableCb --;
                        writeCallbacks.forEach(callback => {
                            if (callback instanceof Function) {
                                callback.unsafeCall();
                            }
                        });
                        writeCallbacks = new Array<Function>();

                        if (!this.writableSync) {
                            this.freshCacheV();
                        }
                        this.afterWrite();
                    } else {
                        Promise.resolve().then(():void => {
                            this.listener?.emit(WritableEvent.ERROR, ERR_MULTIPLE_CALLBACK);
                        });
                    }
                };
                const oldWriting = this.writing;
                this.writing = true;
                const oldWritableCb = this.writableCb;
                this.writableCb ++;
                const oldWritableSync = this.writableSync;
                this.writableSync = true;
                this.writeCallbackBuffer.forEach((callback: Function ) => {
                    writeCallbacks.push(callback);
                });
                let strChunks = new (string)[this.buffer.length];
                let arrayChunks = new (Uint8Array)[this.buffer.length];
                let isString: boolean = false;
                let index: int = 0;
                this.buffer.forEach((value: WritableBuffer) => {
                    if (value.chunk instanceof string) {
                        strChunks[index] = value.chunk as string;
                        isString = true;
                    } else {
                        arrayChunks[index] = value.chunk as Uint8Array;
                    }
                    index++;
                });
                if (isString) {
                    this.doWritev(strChunks, funCallback);
                } else {
                    this.doWritev(arrayChunks, funCallback);
                }
                if (this.isDoWritevOverride) {
                    this.writableSync = false;
                    this.buffer = new Array<WritableBuffer>();
                    this.writeCallbackBuffer = new Array<Function>();
                    if (this.finishMayBe()) {
                        this.finishWrite();
                    }
                } else {
                    this.isDoWritevOverride = false;
                    this.writableBufferLength = bufferChunkLength;
                    this.writing = oldWriting;
                    this.writableCb = oldWritableCb;
                    this.writableSync = oldWritableSync;
                    this.freshCache();
                }
            } else {
                if (this.finishMayBe()) {
                    this.finishWrite();
                }
            }
        }

        private isOnError(): boolean {
            if (this.listener === undefined) {
                return false;
            }
            return this.listener!.isOn(WritableEvent.ERROR) || false;
        }

        private emitErrorOnce(error: Error, reset?: boolean): void {
            if (reset) {
                this.emitErrorExecutedInner = false;
                clearTimeout(this.emitErrorIdInner);
            }
            if (!this.emitErrorExecutedInner) {
                this.emitErrorExecutedInner = true;
                this.emitErrorIdInner = setTimeout((): void => {
                    this.listener?.emit(WritableEvent.ERROR, this.erroredInner ?? error);
                });
            }
        }

        private finishMayBe(): boolean {
            return !this.writing && this.writableCorkedInner === 0 && this.ending;
        }

        private finishWrite(): void {
            if (!this.writableFinishedInner && this.writableCb == 0 &&
                (!this.erroredInner || this.erroredInner!.message === 'write after end')) {
                if (this.writableSync) {
                    asyncFn((): void => {
                        this.writableFinishedInner = true;
                        this.endCallback?.unsafeCall(this.erroredInner);
                        this.listener?.emit(WritableEvent.FINISH);
                    });
                } else {
                    this.writableFinishedInner = true;
                    this.endCallback?.unsafeCall(this.erroredInner);
                    this.listener?.emit(WritableEvent.FINISH);
                }
                asyncFn((): void => {
                    if (!this.erroredInner || this.erroredInner!.message === 'write after end') {
                        this.listener?.emit(WritableEvent.CLOSE);
                    }
                });
            }
        }

        private afterWrite(): void {
            if (!this.finishMayBe() && this.writableNeedDrainInner && this.writableLengthInner == 0) {
                this.writableNeedDrainInner = false;
                this.listener?.emit(WritableEvent.DRAIN);
            }
        }

        noWriteOpes(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            if (chunk instanceof string) {
                this.doWritev([chunk as string], callback);
            } else {
                this.doWritev([chunk as Uint8Array], callback);
            }
            if (!this.isDoWritevOverride) {
                this.isDoWritevOverride = true;
                this.throwError(ERR_DOWRITE_NOT_IMPLEMENTED);
            }
        }

        private throwError(error: Error): void {
            this.erroredInner = error;
            if (this.listener && this.listener!.listenerCount(WritableEvent.ERROR) > 0) {
                setTimeout(() => {
                    this.listener?.emit(WritableEvent.ERROR, error);
                });
            } else {
                throw error;
            }
        }

        /**
         * Writes a chunk to Writable and invokes callback when the chunk is flushed. The return value indicates
         * whether the internal buffer of the Writable reaches the hightWaterMark. If true is returned, the buffer
         * does not reach the hightWaterMark. If false is returned, the buffer has been reached. The write function
         * should be called after the drain event is triggered. If the write function is called continuously,
         * the chunk is still added to the buffer until the memory overflows.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { boolean } Write success returns true, write failure returns false.
         * @throws { BusinessError } 401 - Parameter error.
         * @throws { BusinessError } 10200035 - The doWrite method has not been implemented.
         * @throws { BusinessError } 10200036 - The stream has been ended.
         * @throws { BusinessError } 10200037 - The callback is invoked multiple times consecutively.
         */
        write(chunk?: string | Uint8Array, encoding?: string, callback?: Function): boolean {
            if (encoding) {
                this.setEncoding(encoding!);
            }
            if (chunk === null) {
                throw ERR_INCORRECT_PARAMETER;
            }
            if (!(chunk instanceof string) && !(chunk instanceof Uint8Array)) {
                throw ERR_INCORRECT_PARAMETER;
            }
            if (this.ending) {
                let cb = () => {
                    this.erroredInner = ERR_WRITE_AFTER_END;
                    callback?.unsafeCall(this.erroredInner!);
                }
                setTimeout((): void => {
                    cb();
                });
                return false;
            }
            if (this.erroredInner) {
                return false;
            }
            let flag = false;
            if (chunk instanceof Uint8Array) {
                flag = this.writeUint8Array(chunk, encoding ?? this.encoding, callback);
            } else {
                flag = this.writeString(chunk! as string, encoding ?? this.encoding, callback);
            }
            if (!flag) {
                this.writableNeedDrainInner = true;
            }
            return flag;
        }

        /**
         * This method is invoked by the Writable method during initialization and must not be invoked directly.
         * After the resource is initialized in the doInitialize method, the callback () method is invoked.
         *
         * @param { Function } callback - Callback when the stream has completed the initial.
         */
        doInitialize(callback: Function): void {}

        /**
         * Implemented by subclass inheritance. The implementation logic of flushing chunks in the buffer must not be
         * directly called. The call is controlled by Writable.write.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         */
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            if (this.doWriteFunc != null) {
                this.doWriteFunc?.(chunk, encoding, callback);
                return;
            }
        }

        /**
         * The implementation logic of flushing chunks in the buffer in batches should not be actively called.
         * The call is controlled by Writable.write.
         *
         * @param { string[] | Uint8Array[] } [chunks] - Data to be written.
         * @param { Function } [callback] - Callback after writing.
         */
        doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
            if (this.doWritevFunc != null) {
                this.doWritevFunc?.(chunks, callback);
                return;
            }
            this.isDoWritevOverride = false;
        }

        /**
         * Returns boolean indicating whether it is in ObjectMode.
         */
        get writableObjectMode(): boolean | undefined {
            return this.writableObjectModeInner;
        }

        /**
         * Value of highWatermark.
         */
        get writableHighWatermark(): number | undefined {
            return this.writableHighWatermarkInner;
        }

        /**
         * Is true if it is safe to call writable.write(), which means
         * the stream has not been destroyed or emitted 'error' or 'end'.
         *
         */
        get writable(): boolean | undefined {
            return this.writableInner;
        }

        /**
         * Size of data that can be flushed, in bytes or objects.
         */
        get writableLength(): number | undefined {
            return this.writableLengthInner;
        }

        /**
         * If the buffer of the stream is full and true, otherwise it is false.
         */
        get writableNeedDrain(): boolean | undefined {
            return this.writableNeedDrainInner;
        }

        /**
         * Number of times Writable.uncork() needs to be called in order to fully uncork the stream.
         */
        get writableCorked(): number | undefined {
            return this.writableCorkedInner;
        };

        /**
         * Whether Writable.end has been called.
         */
        get writableEnded(): boolean | undefined {
            return this.writableEndedInner;
        }

        /**
         * Whether Writable.end has been called and all buffers have been flushed.
         */
        get writableFinished(): boolean | undefined {
            return this.writableFinishedInner;
        }

        /**
         * Returns error if the stream has been destroyed with an error.
         */
        get errored(): Error | undefined | null {
            return this.erroredInner;
        }

        /**
         * Writable completes destroyfile and returns true, otherwise returns false.
         */
        get closed(): boolean | undefined {
            return this.closedInner;
        }

        /**
         * Write the last chunk to Writable.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200035 - The doWrite method has not been implemented.
         */
        end(chunk?: string | Uint8Array, encoding?: string, callback?: () => void): Writable {
            return this;
        }

        /**
         * Set the default encoding mode.
         *
         * @param { string } [encoding] - Encoding type.Default: utf8.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
        */
        setDefaultEncoding(encoding?: string): boolean {
            return false;
        }

        /**
         * After the call, all Write operations will be forced to write to the buffer instead of being flushed.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        cork(): boolean {
            return false;
        }

        /**
         * After calling, flush all buffers.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        uncork(): boolean {
            return false;
        }

        /**
         * Registering Event Messages.
         *
         * @param { string } event - Register Event.
         * @param { Callback<emitter.EventData> } callback - event callbacks.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        on(event: string, callback: CallbackFnType): void {}

        /**
         * Cancel event message.
         *
         * @param { string } event - Register Event.
         * @param { Callback<emitter.EventData> } callback - event callbacks.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        off(event: string, callback?: CallbackFnType): void {}
    }

    /**
     * The stream from which data can be read.
     *
     * @syscap SystemCapability.Utils.Lang
     */
    export class Readable {
        protected encoder = new util.TextEncoder();
        protected stringDecoder = new util.StringDecoder();

        private buf: Uint8Array;
        private listener: EventEmitter | undefined;
        private callbacks: Map<string, Array<CallbackFnType>>;
        private isInitialized: boolean;
        private pauseInner: boolean;
        private pipeWritableArrayInner: Array<ReadablePipeStream>;
        private readableObjectModeInner: boolean | undefined;
        private readableInner: boolean;
        private readableHighWatermarkInner: number;
        private readableFlowingInner: boolean;
        private readableLengthInner: number;
        private readableEncodingInner: string;
        private readableEndedInner: boolean;
        private erroredInner: Error | undefined;
        private closedInner: boolean | undefined;
        private doReadFunc: ((size: number) => void) | null;

        private initializeMembers(): void {
            this.buf = new Uint8Array();
            this.listener = new EventEmitter();
            this.callbacks = new Map<string, Array<CallbackFnType>>();
            this.isInitialized = false;
            this.pauseInner = false;
            this.pipeWritableArrayInner = new Array<ReadablePipeStream>();
            this.readableObjectModeInner = false;
            this.readableInner = true;
            this.readableHighWatermarkInner = DEFAULT_HIGH_WATER_MARK;
            this.readableFlowingInner = true;
            this.readableLengthInner = 0;
            this.readableEncodingInner = DEFAULT_ENCODING;
            this.readableEndedInner = false;
            this.erroredInner = undefined;
            this.closedInner = undefined;
            this.doReadFunc = (size: number): void => {};
        }

        private computeNewReadableHighWatermark(readSize: number): number {
            readSize--;
            readSize |= readSize >>> 1;
            readSize |= readSize >>> 2;
            readSize |= readSize >>> 4;
            readSize |= readSize >>> 8;
            readSize |= readSize >>> 16;
            readSize++;
            return readSize;
        }

        private throwError(error: Error): void {
            this.erroredInner = error;
            if (this.listener !== undefined && this.listener!.listenerCount(WritableEvent.ERROR) > 0) {
                setTimeout((): void => {
                    this.listener!.emit(WritableEvent.ERROR, error);
                });
            } else {
                throw error;
            }
        }

        setEndType(): void {
            Promise.resolve().then((): void => {
                this.readableInner = false;
                this.readableEndedInner = true;
                if (this.listener !== undefined) {
                    this.listener!.emit(ReadableEvent.END);
                }
            });
        }

        /**
         * The Readable constructor.
         *
         */
        constructor() {
            this.initializeMembers();
            if (ENCODING_SET.findIndex((element) => element === this.readableEncodingInner.toLowerCase()) === -1) {
                throw ERR_INCORRECT_PARAMETER;
            }
            this.stringDecoder = new util.StringDecoder(this.readableEncodingInner);
            this.encoder = new util.TextEncoder(this.readableEncodingInner);
        }

        /**
         * The Readable constructor.
         *
         * @param { ReadableOptions } options - Provide options.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        constructor(options: ReadableOptions) {
            this.initializeMembers();
            if (options.doRead !== undefined) {
                this.doReadFunc = options.doRead!;
            }

            if (options.encoding !== null && options.encoding!.toLowerCase() !== 'utf8') {
                this.readableEncodingInner = options.encoding!;
            }

            if (options.highWatermark !== undefined) {
                this.readableHighWatermarkInner = options.highWatermark!;
            }

            if (ENCODING_SET.findIndex((element) => element === this.readableEncodingInner.toLowerCase()) === -1) {
                throw ERR_INCORRECT_PARAMETER;
            }
            this.stringDecoder = new util.StringDecoder(this.readableEncodingInner);
            this.encoder = new util.TextEncoder(this.readableEncodingInner);
        }

        /**
         * Returns boolean indicating whether it is in ObjectMode.
         */
        get readableObjectMode(): boolean | undefined {
            return this.readableObjectModeInner;
        }

        /**
         * Is true if it is safe to call readable.read(), which means
         * the stream has not been destroyed or emitted 'error' or 'end'.
         */
        get readable(): boolean {
            if (!this.readableInner && this.readableEndedInner) {
                return false;
            }
            return true;
        }

        /**
         * Returns the value of highWatermark passed when creating this Readable.
         */
        get readableHighWatermark(): number {
            return this.readableHighWatermarkInner;
        }

        /**
         * This property reflects the current state of the readable stream null/true/false.
         */
        get readableFlowing(): boolean {
            return this.readableFlowingInner;
        }

        /**
         * Size of the data that can be read, in bytes or objects.
         */
        get readableLength(): number {
            return this.readableLengthInner;
        }

        /**
         * Getter for the property encoding of a given Readable stream. The encoding property can be set using the
         * readable.setEncoding() method.
         */
        get readableEncoding(): string | null {
            return this.readableEncodingInner;
        }

        /**
         * Whether all data has been generated.
         */
        get readableEnded(): boolean {
            return this.readableEndedInner;
        }

        /**
         * Returns error if the stream has been destroyed with an error.
         */
        get errored(): Error | undefined {
            return this.erroredInner;
        }

        /**
         * Readable completes destroyfile and returns true, otherwise returns false.
         */
        get closed(): boolean {
            return this.closedInner === undefined? false : this.closedInner!;
        }

        /**
         * Reads a buffer of a specified size from the buffer. If the available buffer is sufficient, the result
         * of the specified size is returned. Otherwise, if Readable has ended, all remaining buffers are returned.
         *
         * @param { number } size - Expected length of the data to be read.
         * @returns { string | null } If no data is available to read, null is returned.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200038 - The doRead method has not been implemented.
         */
        read(size?: number): string | null {
            if (this.doReadFunc === null && this.readableInner) {
                this.readableInner = false;
                Promise.resolve().then(() => {
                    this.closedInner = true;
                    this.erroredInner = ERR_DOREAD_NOT_IMPLEMENTED;
                    if (this.listener !== undefined) {
                        this.listener!.emit(ReadableEvent.ERROR, this.erroredInner!);
                        this.listener!.emit(ReadableEvent.CLOSE);
                    }
                });
                return null;
            }

            if (size === undefined) {
                size = this.readableLengthInner;
            }

            if (size > this.readableHighWatermarkInner) {
                this.readableHighWatermarkInner = this.computeNewReadableHighWatermark(size);
            }

            if (size > this.readableLengthInner) {
                if (!this.readableFlowingInner) {
                    return null;
                } else {
                    size = this.readableLengthInner;
                }
            }

            let buffer: string | null = null;
            if (size > 0 && size <= this.readableLengthInner) {
                this.readableLengthInner -= size;
                buffer = this.stringDecoder.write(this.buf.slice(0, size));
                this.buf = this.buf.slice(size);
                if (this.doReadFunc !== null && this.listener !== undefined) {
                    this.listener!.emit(ReadableEvent.DATA, buffer);
                }
            }

            if ((!this.readableInner || size <= -1) && this.readableFlowingInner) {
                return null;
            }

            if (this.readableFlowingInner) {
                try {
                    this.doRead(this.readableHighWatermarkInner);
                } catch (error) {
                    this.readableInner = false;
                    this.readableEndedInner = true;
                    if (this.listener !== undefined) {
                        this.listener!.emit(ReadableEvent.ERROR, error);
                        this.listener!.emit(ReadableEvent.CLOSE);
                    }
                }
            }
            return buffer;
        }

        /**
         * Switch Readable to Streaming Mode.
         *
         * @returns { Readable } Return this object.
         */
        resume(): Readable {
            return this;
        }

        /**
         * Toggle Readable to Suspend Mode.
         *
         * @returns { Readable } Return this object.
         */
        pause(): Readable {
            return this;
        }

        /**
         * Sets the encoding format of the input binary data.Default: utf8.
         *
         * @param { string } [encoding] - Original Data Encoding Type.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        setEncoding(encoding?: string): boolean {
            if(this.readableEncodingInner === encoding) {
                return true;
            }

            if (encoding === undefined) {
                this.readableEncodingInner = DEFAULT_ENCODING;
                this.encoder = new util.TextEncoder(this.readableEncodingInner);
                this.stringDecoder = new util.StringDecoder(this.readableEncodingInner);
                return false;
            }

            if (encoding!.toLowerCase() === 'utf8') {
                encoding = 'utf-8';
            }

            if (this.buf.length !== 0) {
                console.error('stream: The buffer also has data, and encoding is not allowed');
                return false;
            }

            let encodingLowCase = encoding!.toLowerCase();
            if (ENCODING_SET.indexOf(encodingLowCase) !== -1) {
                try {
                    this.encoder = new util.TextEncoder(encoding);
                    this.stringDecoder = new util.StringDecoder(encoding);
                    this.readableEncodingInner = encodingLowCase;
                } catch (e) {
                    this.throwError(e as Error);
                    return false;
                }
                return true;
            } else {
                let err = new BusinessError();
                err.name='BusinessError';
                err.message = `Parameter error. The type of ${encoding} must be string.`;
                this.throwError(err);
                return false;
            }
        }

        /**
         * Query whether it is in pause state.
         *
         * @returns { boolean } Pause state returns true, otherwise returns false.
         */
        isPaused(): boolean {
            return false;
        }

        /**
         * Concatenated a Writable to a Readable and switches the Readable to stream mode.
         *
         * @param { Writable } destination - Output writable stream.
         * @param { Object } [options] - Pipeline Options.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        pipe(destination: Writable, options?: Object): Writable {
            return new Writable();
        }

        /**
         * Disconnect Writable from Readable.
         *
         * @param { Writable } [destination] - Writable Streams Needing to Be Disconnected.
         * @returns { Readable } Returns the Readable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        unpipe(destination?: Writable): Readable {
            return this;
        }

        /**
         * Registering Event Messages.
         *
         * @param { string } event - Registering Events.
         * @param { Callback<emitter.EventData> } callback - Event callback.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        on(event: string, callback: CallbackFnType): void {}

        /**
         * Cancel event message.
         *
         * @param { string } event - Registering Events.
         * @param { Callback<emitter.EventData> } callback - Event callback.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        off(event: string, callback?: CallbackFnType): void {}

        /**
         * It may be implemented by child classes, and if so, will be called by the Readable class methods only.
         * It must not be called directly.
         *
         * @param { Function } callback - Callback when the stream has completed the initial.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doInitialize(callback: () => void): void {}

        /**
         * The specific implementation of data production. It must not be actively called.
         * After data production, Readable.push should be called to push the produced data into the buffer.
         * If push is not called, doRead will not be called again.
         *
         * @param { number } size - Expected length of the data to be read.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doRead(size: number): void {
            if (this.doReadFunc !== null) {
                this.doReadFunc!(size);
            }
        }

        /**
         * Adds the generated data to the buffer. The return value indicates whether the data in the buffer has not
         * reached the highWaterMark (similar to Writable.write). If the chunk is null, all data has been generated.
         *
         * @param {  Uint8Array | string | null } chunk - Binary data to be stored in the buffer.
         * @param { string } [encoding] - Binary data encoding type.
         * @returns { boolean } If true is returned, the data in the buffer reaches the highWaterMark. Otherwise, the
         * data in the buffer does not reach the highWaterMark.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        push(chunk: Uint8Array | string | null, encoding?: string): boolean {
            let bufferArr: Uint8Array;
            if (encoding !== undefined) {
                this.setEncoding(encoding!);
            }

            if (chunk instanceof string || chunk instanceof Uint8Array) {
                if (chunk instanceof string) {
                    return false;
                } else if (chunk instanceof Uint8Array) {
                    this.buf = chunk;
                    this.readableLengthInner += chunk.length;
                }

                const highWaterMark = this.readableLengthInner <= this.readableHighWatermarkInner;
                Promise.resolve().then((): void => {
                    try {
                        if (this.readableFlowingInner) {
                            !this.pauseInner && this.read(highWaterMark ? this.readableLengthInner : -1);
                        } else {
                            if (highWaterMark && this.doReadFunc !== null) {
                                this.doRead(this.readableHighWatermarkInner);
                            }
                        }
                    } catch (error) {
                        if (this.listener !== undefined) {
                            this.listener!.emit(ReadableEvent.ERROR, error);
                            this.listener!.emit(ReadableEvent.CLOSE);
                        }
                    }
                    if (this.listener !== undefined) {
                        this.listener!.emit(ReadableEvent.READABLE);
                    }
                });
                return this.readableLengthInner < this.readableHighWatermarkInner;
            } else if (chunk === null) {
                if (!this.readableEndedInner && this.readableInner) {
                    if (!this.readableFlowingInner && this.listener !== undefined) {
                        this.listener!.emit(ReadableEvent.READABLE);
                    }

                    this.readableInner = false;
                    Promise.resolve().then((): void => {
                        this.readableEndedInner = true;
                        this.pauseInner = true;
                        this.closedInner = true;
                        if (this.listener !== undefined) {
                            this.listener!.emit(ReadableEvent.END);
                            this.listener!.emit(ReadableEvent.CLOSE);
                        }
                    });
                }
                return false;
            } else {
                this.readableInner = false;
                let err = new BusinessError();
                err.name='BusinessError';
                err.message = 'ERR_INVALID_ARG_TYPE';
                this.erroredInner = err;
                if (this.listener !== undefined) {
                    this.listener!.emit(ReadableEvent.ERROR, this.erroredInner!);
                }
                return false;
            }
        }
    }

    /**
     * Duplex streams are streams that inheritance for readable streams and consist of Writable streams.
     *
     * @extends Readable
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    export class Duplex extends Readable {
        private _writable: Writable;
        /**
        * The Duplex constructor.
         *
         */
        constructor() {}

        /**
         * writes a chunk to Writable and invokes callback when the chunk is flushed. The return value indicates
         * whether the internal buffer of the Writable reaches the hightWaterMark. If true is returned, the buffer
         * does not reach the hightWaterMark. If false is returned, the buffer has been reached. The write function
         * should be called after the drain event is triggered. If the write function is called continuously,
         * the chunk is still added to the buffer until the memory overflows
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { boolean } Write success returns true, write failure returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200036 - The stream has been ended.
         * @throws { BusinessError } 10200037 - The callback is invoked multiple times consecutively.
         * @throws { BusinessError } 10200039 - The doTransform method has not been implemented for
         * a class that inherits from Transform.
         */
        write(chunk?: string | Uint8Array, encoding?: string, callback?: () => void): boolean {
            return false;
        }

        /**
         * Write the last chunk to Writable.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200039 - The doTransform method has not been implemented for
         * a class that inherits from Transform.
         */
        end(chunk?: string | Uint8Array, encoding?: string, callback?: () => void): Writable {
            return new Writable();
        }

        /**
         * Set the default encoding mode.
         *
         * @param { string } [encoding] - Encoding type.Default: utf8.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        setDefaultEncoding(encoding?: string): boolean {
            return false;
        }

        /**
         * After the call, all Write operations will be forced to write to the buffer instead of being flushed.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        cork(): boolean {
            return false;
        }

        /**
         * After calling, flush all buffers.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        uncork(): boolean {
            return false;
        }

        doInitialize(callback: Function): void {}

        /**
         * Implemented by subclass inheritance. The implementation logic of flushing chunks in the buffer must not be
         * directly called. The call is controlled by Writable.write.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doWrite(chunk: string | Uint8Array, encoding: string, callback: () => void): void {}

        /**
         * The implementation logic of flushing chunks in the buffer in batches should not be actively called.
         * The call is controlled by Writable.write.
         *
         * @param { string[] | Uint8Array[] } [chunks] - Data to be written.
         * @param { Function } [callback] - Callback after writing.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doWritev(chunks: string[] | Uint8Array[], callback: () => void): void {}

        get writableObjectMode(): boolean {
            return false;
        }

        get writableHighWatermark(): number {
            return 0;
        }

        get writable(): boolean {
            return false;
        }

        get writableLength(): number {
            return 0;
        }

        get writableNeedDrain(): boolean {
            return false;
        }

        get writableCorked(): number {
            return 0;
        }

        get writableEnded(): boolean {
            return false;
        }

        get writableFinished(): boolean {
            return false;
        }
    }

    /**
     * Transform stream is a Duplex stream where the output is computed in some way from the input.
     * Transform implementations must implement the doTransform() method and may also implement the doFlush() method.
     *
     * @extends Duplex
     */
    export class Transform extends Duplex {
        /**
         * The Transform constructor.
         *
         */
        constructor() {}

        /**
         * Convert the input data. After the conversion,
         * Transform.push can be called to send the input to the read stream.
         * Transform.push should not be called Transform.write to call.
         *
         * @param { string } chunk - Input data to be converted.
         * @param { string } encoding - If the chunk is a string, then this is the encoding type. If chunk is a buffer,
         * then this is the special value 'buffer'. Ignore it in that case.
         * @param { Function } callback - Callback after conversion.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doTransform(chunk: string, encoding: string, callback: () => void): void {}

        /**
         * After all data is flushed to the write stream,
         * you can use the Transform.doFlush writes some extra data, must
         * not be called directly, only called by Writable after flushing all data.
         *
         * @param { Function } callback - Callback after flush completion.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doFlush(callback: () => void): void {}
    }
}
