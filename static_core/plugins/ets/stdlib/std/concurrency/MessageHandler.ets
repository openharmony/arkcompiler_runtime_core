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

package std.concurrency;

import { launch } from "std/concurrency";

type Handler = (message: concurrency.Message) => void;
type Task = () => void;

export namespace concurrency {
    /**
     * MessageHandler class for handling message processing and callback management
     * Provides functionality for posting tasks, sending messages, and managing callbacks
     */
    export class MessageHandler {
        internal handler: Handler;
        private worker: EAWorker;

        /**
         * Create a MessageHandler instance
         * @param { Handler } handler The message handler function
         * @param { EAWorker } [worker=EAWorker.current()] The worker to handle messages
         */
        constructor(handler: Handler, worker: EAWorker | undefined = EAWorker.current()) {
            if (worker == undefined) {
                worker = EAWorker.current();
                if (worker == undefined) {
                    throw new Error("No worker found for MessageHandler");
                }
            }
            this.handler = handler;
            this.worker = worker as EAWorker;
        }

        /**
         * Check if a specific callback exists in the handler
         * @param { Task } callback The callback function to check
         * @returns { boolean } True if the callback exists, false otherwise
         */
        public hasCallbacks(callback: Task): boolean {
            return this.worker.contains(new concurrency.Message(callback, this));
        }

        /**
         * Check if messages with a specific identifier exist
         * @param { int } what The message identifier to check
         * @returns { boolean } True if messages with the identifier exist, false otherwise
         */
        public hasMessages(what: int): boolean {
            return this.worker.contains(new concurrency.Message(what, this));
        }

        /**
         * Check if messages with a specific identifier and object exist
         * @param { int } what The message identifier to check
         * @param { Object } obj The object to check
         * @returns { boolean } True if messages with the identifier and object exist, false otherwise
         */
        public hasMessages(what: int, obj: Object): boolean {
            return this.worker.contains(new concurrency.Message(what, obj, this));
        }

        /**
         * Post a callback task to be executed
         * @param { Task } callback The callback function to post
         * @returns { boolean } True if the callback was successfully posted, false otherwise
         */
        public post(callback: Task): boolean {
            try {
                let msg = new concurrency.Message(callback, this);
                this.worker.enqueue(msg);
                return true;
            } catch (e) {
                return false;
            }
        }

        /**
         * Remove a specific callback from the handler
         * @param { Task } callback The callback function to remove
         * @returns { boolean } True if the callback was successfully removed, false otherwise
         */
        public removeCallbacks(callback: Task): boolean {
            let msg = new concurrency.Message(callback, this);
            return this.worker.cancelMessage(msg);
        }

        /**
         * Remove all messages with a specific identifier
         * @param { int } what The message identifier to remove
         * @returns { boolean } True if messages were successfully removed, false otherwise
         */
        public removeMessages(what: int): boolean {
            let msg = new concurrency.Message(what, this);
            return this.worker.cancelMessage(msg);
        }

        /**
         * Remove messages with a specific identifier and object
         * @param { int } what The message identifier to remove
         * @param { Object } obj The object to match
         * @returns { boolean } True if messages were successfully removed, false otherwise
         */
        public removeMessages(what: int, obj: Object): boolean {
            let msg = new concurrency.Message(what, obj, this);
            return this.worker.cancelMessage(msg);
        }

        /**
         * Send an empty message with a specific identifier
         * @param { int } what The message identifier
         * @returns { boolean } True if the message was successfully sent, false otherwise
         */
        public sendEmptyMessage(what: int): boolean {
            return this.sendMessage(new concurrency.Message(what, this));
        }

        /**
         * Send a message to the handler
         * @param { Message } message The message to send
         * @returns { boolean } True if the message was successfully sent, false otherwise
         */
        public sendMessage(message: concurrency.Message): boolean {
            try {
                if (message.getTarget() != this) {
                    return false;
                }
                this.worker.enqueue(message);
                return true;
            } catch (e) {
                return false;
            }
        }

        /**
         * Get the target worker associated with this handler
         * @returns { EAWorker } The worker that handles messages for this handler
         */
        public getWorker(): EAWorker {
            return this.worker;
        }
    }
}
