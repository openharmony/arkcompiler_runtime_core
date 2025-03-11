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

package escompat;

export namespace StdProcess {
    const SIGNAL_MAP: Map<string, int> = {
        "SIGHUP": 1,
        "SIGINT": 2,
        "SIGQUIT": 3,
        "SIGILL": 4,
        "SIGTRAP": 5,
        "SIGABRT": 6,
        "SIGBUS": 7,
        "SIGFPE": 8,
        "SIGKILL": 9,
        "SIGUSR1": 10,
        "SIGSEGV": 11,
        "SIGUSR2": 12,
        "SIGPIPE": 13,
        "SIGALRM": 14,
        "SIGTERM": 15,
        "SIGSTKFLT": 16,
        "SIGCHLD": 17,
        "SIGCONT": 18,
        "SIGSTOP": 19,
        "SIGTSTP": 20,
        "SIGTTIN": 21,
        "SIGTTOU": 22,
        "SIGURG": 23,
        "SIGXCPU": 24,
        "SIGXFSZ": 25,
        "SIGVTALRM": 26,
        "SIGPROF": 27,
        "SIGWINCH": 28,
        "SIGIO": 29,
        "SIGPWR": 30,
        "SIGSYS": 31,
    };

    const MAX_BUFFER_SIZE: int = 1024 * 1024;

    /**
     * @class Represents child process created by executing process.runCmd()
     */
    export final class ChildProcess {

        internal constructor(bufferSize: int) {
            this.outBuffer = new ArrayBuffer(bufferSize);
            this.errBuffer = new ArrayBuffer(bufferSize);
            this.pid = -1;
            this.ppid = -1;
            this.exitCode = -1;
            this.killed = false;
            this.outBytesRead = 0;
            this.errBytesRead = 0;
            this.outFd = -1;
            this.errorFd = -1;
        };

        /**
         * Wait for child process to terminate
         *
         * @returns Promise containing child process's exit code value
         */
        wait(): Promise<number> {
            this.waitImpl();
            return Promise.resolve < number > (this.exitCode);
        }

        /**
         * Get child process's output to stdout
         *
         * @returns Promise containing bytes read from child process's stdout in Uint8Array
         */
        getOutput(): Promise<Uint8Array> {
            this.readOutput();
            return Promise.resolve < Uint8Array > (new Uint8Array(this.outBuffer, 0, this.outBytesRead));
        }

        /**
         * Get child process's output to stderr
         *
         * @returns Promise containing bytes read from child process's stderr in Uint8Array
         */
        getErrorOutput(): Promise<Uint8Array> {
            this.readErrorOutput();
            return Promise.resolve < Uint8Array > (new Uint8Array(this.errBuffer, 0, this.errBytesRead));
        }

        /**
         * Close the child process
         */
        native close(): void;

        /**
         * Send a signal to the child process
         *
         * @param signal number or string indicating a kill signal
         *
         */
        kill(signal: number | string): void {
            let killSignal: int;
            if (signal instanceof string) {
                if (SIGNAL_MAP.has(signal as string)) {
                    killSignal = SIGNAL_MAP.get(signal as string)! as int;
                } else {
                    killSignal = SIGNAL_MAP.get("SIGTERM")! as int;
                }
            } else {
                if (!isInteger(signal as number) || (signal as number) < 1 || (signal as number) > 31) {
                    throw new RangeError("Invalid signal value");
                }
                killSignal = signal as int;
            }
            this.killImpl(killSignal);
        }

        internal native spawn(command: string, timeout: int, killSignal: int): void;

        private native killImpl(signal: int): void;
        private native waitImpl(): number;
        private native readOutput(): void;
        private native readErrorOutput(): void;

        /**
         * Buffer for reading child process's stdout
         */
        private outBuffer: ArrayBuffer;

        /**
         * Buffer for reading child process's stderr
         */
        private errBuffer: ArrayBuffer;

        /**
         * Child process's pid as double value
         */
        readonly pid: number;

        /**
         * Child process's ppid as double value
         */
        readonly ppid: number;

        /**
         * Child process's exit code as double value
         */
        readonly exitCode: number;

        private outBytesRead: int;

        private errBytesRead: int;

        private outFd: int;

        private errorFd: int;

        /**
         * Indicates if the child process has been killed
         */
        readonly killed: boolean;
    }


    /**
     * @class Wrapper for parameters used by process.RunCmd function to create a ChildProcess 
     */
    export final class ConditionType {
        /**
        * Time after which a kill signal will be send to child process
        */
        timeout?: number;

        /**
        * Kill signal that is sent to child process after timeout
        */
        killSignal?: number | string;

        /**
        * Maximum buffer size for both sdout and stderr of child process
        */
        maxBuffer?: number;
    }

    /**
     * @class Interface that allows to interact with processes in the system, retrieve process-specific and system-specific information
     */
    export final class ProcessManager {
        /**
         * Determine if a given uid belongs to an app
         *
         * @param uid
         *
         * @returns resulting boolean
         */
        static isAppUid(uid: number): boolean {
            throw new Error("not implemented");
        }

        /**
         * Determine the uid corresponding to a user name string
         *
         * @param v user name string
         *
         * @returns user id as number
         */
        static native getUidForName(v: string): number;

        /**
         * Determine the priority for a given pid or tid
         *
         * @param v
         *
         * @returns priority value as number
         */
        static native getThreadPriority(v: number): number;

        /**
        * Get configuration information
        *
        * @param name constant corresponding to a configuration parameter
        *
        * @returns value of the configuration parameter
        */
        static native getSystemConfig(name: number): number;

        /**
         * Get value of an environment variable
         *
         * @param name environment variable
         *
         * @returns value of the environment variable
         */
        static native getEnvironmentVar(name: string): string;

        /**
         * Exit the program
         *
         * @param code exit code
         *
         */
        static native exit(code: number): void;

        /**
         * Send a kill singnal to a process with a given pid
         *
         * @param signal signal to send
         *
         * @param pid pid of receiving process
         *
         * @returns bloolen value indicating if the signal was sent succesfully
         *
         */
        static native kill(signal: number, pid: number): boolean;
    }

    export type EventListener = (evt: Object) => void;

    /**
     * @class Interface that allows to create and interact with processes in the system, retrieve process-specific and system-specific information
     */
    export final class process {
        /**
         * Get effective group id of the current process
         *
         * @returns egid as number
         */
        static get egid(): number {
            return process.getEgidImpl();
        }

        /**
         * Get effective user id of the current process
         *
         * @returns euid as number
         */
        static get euid(): number {
            return process.getEuidImpl();
        }

        /**
         * Get group id of the current process
         *
         * @returns gid as number
         */
        static get gid(): number {
            return process.getGidImpl();
        }

        /**
         * Get user id of the current process
         *
         * @returns uid as number
         */
        static get uid(): number {
            return process.getUidImpl();
        }

        /**
         * Get groups to which the current process belongs
         *
         * @returns groups as number array
         */
        static get groups(): number[] {
            return process.getGroupsImpl();
        }

        /**
         * Get process id of the current process
         *
         * @returns pid as number
         */
        static get pid(): number {
            return process.getPidImpl();
        }

        /**
         * Get parent process id of the current process
         *
         * @returns ppid as number
         */
        static get ppid(): number {
            return process.getPpidImpl();
        }

        /**
         * Get thread id of the current process
         *
         * @returns tid as number
         */
        static get tid(): number {
            return process.getTidImpl();
        }

        /**
         * Determine if the current process is isolated
         *
         * @returns boolean value indicating if the current process is isolated
         */
        static isIsolatedProcess(): boolean {
            throw new Error("not implemented");
        }

        /**
         * Determine if the current process is a 64-bit process
         *
         * @returns boolean value indicating if the current process is a 64-bit process
         */
        static native is64Bit(): boolean;

        /**
         * Get start time of the current process
         *
         * @returns current process's start time milliseconds as number
         */
        static native getStartRealtime(): number;

        /**
         * Get past cpu time of the current process
         *
         * @returns current process's past cpu time in milliseconds as number
         */
        static native getPastCpuTime(): number;


        /**
         * Create a child process to execute a given command
         *
         * @param command command to execute
         *
         * @param options specifies timeout, kill signal and maximum buffer size for child process created
         *
         * @returns instance of ChildProcess
         *
         */
        public static runCmd(command: string, options?: ConditionType): ChildProcess {
            let args: int[] = process.getCmdOptions(options);
            let childProcess = new ChildProcess(args[2]);
            childProcess.spawn(command, args[0], args[1]);
            return childProcess;
        }

        /**
         * Abort the program
         */
        static native abort(): void;


        /**
         * Store user-triggered event
         *
         * @param type type of storage event
         *
         * @param listener callback for event handling
         *
         */
        static on(type: string, listener: EventListener): void {
            throw new Error("not implemented");
        }

        /**
         * Remove user-triggered event
         *
         * @param type type of storage event
         *
         * @returns boolean indicating if the event has been removed successfully
         *
         */
        static off(type: string): boolean {
            throw new Error("not implemented");
        }

        /**
         * Get the current working directory of the program 
         *
         * @returns current working directory as string
         *
         */
        static native cwd(): string;

        /**
         * Change the current working directory of the program 
         *
         * @param path working directory
         *
         */
        static native chdir(path: string): void;

        /**
        * Get the system uptime
        *
        * @returns uptime in milliseconds as number
        *
        */
        static native uptime(): number;

        private static getCmdOptions(options?: ConditionType): int[] {
            let timeout: int = 0;
            let killSignal: int = SIGNAL_MAP.get("SIGTERM")! as int;
            let maxBuffer: int = MAX_BUFFER_SIZE;

            if (options == undefined) {
                return [timeout, killSignal, maxBuffer];
            }

            if (options.timeout != undefined && options.timeout as number > 0) {
                timeout = options.timeout as int;
            }

            if (options.killSignal != undefined) {
                if (options.killSignal instanceof string) {
                    let key: string = options.killSignal as string;
                    if (SIGNAL_MAP.has(key)) {
                        killSignal = SIGNAL_MAP.get(options.killSignal as string)! as int;
                    }
                } else {
                    if (!isInteger(options.killSignal as number) || (options.killSignal as number) < 1 || (options.killSignal as number) > 31) {
                        throw new RangeError("Invalid signal value");
                    }
                    killSignal = options.killSignal as int;
                }
            }

            if (options.maxBuffer != undefined) {
                if ((options.maxBuffer as int) < 0) {
                    throw new RangeError("Invalid buffer size value");
                }
                maxBuffer = min(options.maxBuffer as int, MAX_BUFFER_SIZE);
            }

            return [timeout, killSignal, maxBuffer];
        }

        private static native getEgidImpl(): number;
        private static native getEuidImpl(): number;
        private static native getGidImpl(): number;
        private static native getUidImpl(): number;
        private static native getGroupsImpl(): number[];
        private static native getPidImpl(): number;
        private static native getPpidImpl(): number;
        private static native getTidImpl(): number;
    }
}