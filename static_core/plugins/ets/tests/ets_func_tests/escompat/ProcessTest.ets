/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function test(result: int, message: String ): int {
    if (result != 0) {
      console.log('FAILED: ' + message);
      return 1;
    }
    return 0;
}

function getOutput(child: ChildProcess, isStdErr: boolean) {
    let buffer: Uint8Array;
    if (isStdErr) {
        buffer = child.getErrorOutput().awaitResolution();
    } else {
        buffer = child.getOutput().awaitResolution();
    }
    let output: String = "";

    for (let i: int = 0; i < buffer.length; ++i) {
        output +=  String.fromCharCode(buffer.$_get(i));
    }

    return output;
}

function getChildOutputAsString(child: ChildProcess): String {
    return getOutput(child, false);
}

function getChildErrorOutputAsString(child: ChildProcess): String {
    return getOutput(child, true);
}

function getChildOutputAsString(child: ChildProcess, separator: string): String[] {
    let output = getChildOutputAsString(child);
    let tokens: String[] = output.split(separator);
    return tokens;
}

function getChildErrorOutputAsString(child: ChildProcess, separator: string): String[] {
     let output = getChildErrorOutputAsString(child);
    let tokens: String[] = output.split(separator);
    return tokens;
}

function runChildAndGetOutput(command: string): String {
    let childProcess = process.runCmd(command);
    childProcess.wait();
    return getChildOutputAsString(childProcess);
}

function runChildAndGetOutput(command: string, separator: string): String[] {
    let output: String = runChildAndGetOutput(command);
    let atributes: String[] = output.split(separator);
    return atributes;
}

function runChildAndGetErrorOutput(command: string): String {
    let childProcess = process.runCmd(command);
    childProcess.wait();
    return getChildErrorOutputAsString(childProcess);
}

function getProcAttributes(): Map<String, number> {
        let pid = parseInt(runChildAndGetOutput("cat /proc/$$/status | grep PPid | cut -f2"));
        let cmd = "cat /proc/" + pid + "/status";
        let output = runChildAndGetOutput(cmd, "\n");
        let result = new Map<String, number>();
        let val: number  = 0;
        for (let i:int = 0; i < output.length; ++i) {
            if (output[i].startsWith("Tgid:")) {
                val = parseInt(output[i].replace("Tgid:", ""));
                result.set("tid", val);
            } else if (output[i].startsWith("Pid:")) {
                val = parseInt(output[i].replace("Pid:", ""));
                result.set("pid", val);
            } else if (output[i].startsWith("PPid:")) {
                val = parseInt(output[i].replace("PPid:", ""));
                result.set("ppid", val);
            } else if (output[i].startsWith("Uid:")) {
                let tokens = output[i].replace("Uid:", "").split("\t");
                result.set("uid", parseInt(tokens[1]));
                result.set("euid", parseInt(tokens[2]));
            } else if (output[i].startsWith("Gid:")) {
                let tokens = output[i].replace("Gid:", "").split("\t");
                result.set("gid", parseInt(tokens[1]));
                result.set("egid", parseInt(tokens[2]));
            } else if (output[i].startsWith("Groups:")) {
                let tokens = output[i].replace("Groups:", "").split("\t");
                for (let i = 1; i < tokens.length; ++i) {
                    result.set("group" + i, parseInt(tokens[i]));
                }
            }
        }

    return result;
}

function createChildForTesting(): ChildProcess {
    let command = "echo All good; echo 1>&2 All good; echo $$; echo $(cat /proc/$$/status | grep PPid) | cut -d ' ' -f2;exit 3;"
    let childProcess = process.runCmd(command);
    return childProcess;
}

function testChildWait(child: ChildProcess): int {
    let promise = child.wait();
    return promise.awaitResolution() == 768 ? 0 : 1;
}

function testChildGetStdOut(child: ChildProcess): int {
    let output = getChildOutputAsString(child, "\n");
    return output[0] == "All good" ? 0 : 1;
}

function testChildGetStdErr(child: ChildProcess): int {
    let output = getChildErrorOutputAsString(child);
    return output == "All good\n" ? 0 : 1;
}

function testChildGetPid(child: ChildProcess): int {
    let output = getChildOutputAsString(child, "\n");
    return parseInt(output[1]) == child.pid ? 0 : 1;
}

function testChildGetPpid(child: ChildProcess): int {
    let output = getChildOutputAsString(child, "\n");
    return parseInt(output[2]) == child.ppid ? 0 : 1;
}

function testChildGetExitCode(child: ChildProcess): int {
    return child.exitCode == 768 ? 0 : 1;
}

function testChildKill(): int {
    let res: int = 0;
    let childProcess = process.runCmd("while true; do continue ; done");
    childProcess.kill("SIGKILL");
    childProcess.wait()
    res += childProcess.killed ? 0 : 1;

    childProcess = process.runCmd("while true; do continue ; done");
    childProcess.kill(9);
    childProcess.wait()
    res += childProcess.killed ? 0 : 1;
    return res;
}

function testChildClose(): int {
    let childProcess = process.runCmd("while true; do continue ; done");
    childProcess.close();
    childProcess.wait();
    return childProcess.exitCode >= 0 ? 0 : 1;
}

function testProcessManagerGetUidForName(): int {
    let expected = ProcessManager.getUidForName("root");
    return  expected == 0|| expected == -1 ? 0 : 1;
}

function testProcessManagerGetThreadPriority(): int {
    let output = runChildAndGetOutput("cat /proc/" + process.ppid + "/stat", " ");
    let expected = parseInt(output[17]);
    let actual = ProcessManager.getThreadPriority(process.pid) + 20;
    return  expected == actual ? 0 : 1;
}

function  testProcessManagerGetSystemConfig(): int {
    let res: int = 0;

    let _SC_ARG_MAX = 0;
    let _SC_OPEN_MAX = 4;
    let _SC_PAGE_SIZE = 8;

    res += ProcessManager.getSystemConfig(_SC_ARG_MAX) > 0 ? 0 : 1;
    res += ProcessManager.getSystemConfig(_SC_OPEN_MAX) > 0 ? 0 : 1;
    res += ProcessManager.getSystemConfig(_SC_PAGE_SIZE) > 0 ? 0 : 1;

    return res;
}

function testProcessManagerGetEnvironmentVar(): int {
    let output : String[] = runChildAndGetOutput("printenv","\n");
    let entries: Map<String, String> = new Map<String, String>();
    for (let i = 0; i < output.length - 1 && i < 4; ++i) {
        let entry = output[i].split("=", 2);
        entries.set(entry[0], entry[1]);
    }

    let res: int = 0;
    entries.forEach((value: String, key: String) => {
        let expected = ProcessManager.getEnvironmentVar(key)
        res += expected == value || expected == "" ? 0 : 1;
    })
    return res;
}

// function testProcessManagerExit();

function testProcessManagerKill(): int {
    let childProcess = process.runCmd("while true; do continue ; done");
    let pid = childProcess.pid;
    let SIGKILL: int = 9;
    while(!ProcessManager.kill(SIGKILL, pid));
    childProcess.wait();
    return childProcess.exitCode >= 0 ? 0 : 1;
}

function testGetPid(attributes: Map<String, number> ): int {
    return  process.pid == attributes.get("pid") ? 0 : 1;
}

function testGetTid(attributes: Map<String, number>): int {
    return  process.tid == attributes.get("tid") ? 0 : 1;
}

function testGetPpid(attributes: Map<String, number>): int {
    return  process.ppid == attributes.get("ppid") ? 0 : 1;
}

function testGetUid(attributes: Map<String, number>): int {
    return  process.uid == attributes.get("uid") ? 0 : 1;
}

function testGetEuid(attributes: Map<String, number>): int {
    return  process.euid == attributes.get("euid") ? 0 : 1;
}

function testGetGid(attributes: Map<String, number>): int {
    return  process.gid == attributes.get("gid") ? 0 : 1;
}

function testGetEgid(attributes: Map<String, number>): int {
    return  process.egid == attributes.get("egid") ? 0 : 1;
}

function testGetGroups(attributes: Map<String, number>): int {
    let res: int = 0;
    let groups = process.groups;
    
    for (let i: int = 1; ; ++i) {
        let key : String = "groups" + i;
        if (!attributes.has(key)) {
            break;
        }
        res += attributes.get(key) == groups[i - 1] ? 0 : 1;
    }

    return res;
}

function testIs64Bit(): int {
    let output = runChildAndGetOutput("getconf LONG_BIT", "\n");
    let architecture: String = output[0];
    if (process.is64Bit() && architecture == "64") {
        return 0;
    }
    if (!process.is64Bit() && architecture == "32") {
        return 0;
    }    
    return 1;
}

function testGetStartRealTime(): int {
    let command: string = "cat /proc/" + process.pid + "/stat"
    let values: String[] = runChildAndGetOutput(command, " ");

    let _SC_CLCL_TCK = 2;
    let ticksPerSecond = ProcessManager.getSystemConfig(2);

    let milisecondsPerSecond = 1000;
    let expected = Math.floor(parseInt(values[21]) / ticksPerSecond * milisecondsPerSecond);
    let actual = process.getStartRealtime();

    return actual == expected ? 0 : 1;
}

function testGetPastCpuTime(): int {
    let start = process.getPastCpuTime();

    while (process.getPastCpuTime()  - start < 1);

    let finish = process.getPastCpuTime();

    return finish - start >= 1 ? 0 : 1;
}

function testRunCmdNoCondition(): int {
    let childProcess = process.runCmd("echo Hello");
    childProcess.wait();
    return childProcess.exitCode == 0 ? 0 : 1;
}

function testRunCmdWithTimeout(): int {
    let condition = new ConditionType();
    condition.timeout = 1;
    let childProcess = process.runCmd("while true; do continue ; done", condition);
    childProcess.wait();
    return childProcess.exitCode >= 0 ? 0 : 1;
}

function testRunCmdWithMaxBuffer(): int {
    let res: int = 0;

    let condition = new ConditionType();
    condition.maxBuffer = 10;

    let childProcess = process.runCmd("echo Display long string output; echo 1>&2 Display long string output", condition);
    childProcess.wait();
    let stdOut = getChildOutputAsString(childProcess);
    let stdErr = getChildErrorOutputAsString(childProcess);
    
    res += stdOut.length == 10 ? 0 : 1;
    res += stdErr.length == 10 ? 0 : 1;

    return res;
}

// function testAbort();

function testCwd(): int {
    let expected = runChildAndGetOutput("pwd");
    let actual  = process.cwd() + "\n";
    return actual == expected ? 0 : 1;
}

function testChangeDir(): int {
    let result: int  = 0;

    let currentDir = process.cwd();
    process.chdir("/");
    result += process.cwd() == "/" ? 0 : 1;

    process.chdir(currentDir);
    result += process.cwd() == currentDir ? 0 : 1;
    return result;
}

function testUptime(): int {
    let start = process.uptime();

    while (process.uptime()  - start < 1);

    let finish = process.uptime();

    return finish - start >= 1 ? 0 : 1;
}

function main(): int {
    let failures: int = 0;

    let child = createChildForTesting();
    failures += test(testChildWait(child), "ChildProcess.wait()");
    failures += test(testChildGetStdOut(child), "ChildProcess.getOutput()");
    failures += test(testChildGetStdErr(child), "ChildProcess.getErrorOutput()");
    failures += test(testChildGetPid(child), "ChildProcess.pid");
    failures += test(testChildGetPpid(child), "ChildProcess.ppid");
    failures += test(testChildGetExitCode(child), "ChildProcess.exitCode");


    failures += test(testChildKill(), "ChildProcess.kill() / ChildProcess.killed");
    failures += test(testChildClose(), "ChildProcess.close()");

    failures += test(testProcessManagerGetUidForName(), "ProcessManager.getUidForName()");
    failures += test(testProcessManagerGetThreadPriority(), "ProcessManager.getThreadPriority()");
    failures += test(testProcessManagerGetSystemConfig(), "ProcessManager.getSystemConfig()");
    failures += test(testProcessManagerGetEnvironmentVar(), "ProcessManager.getEnvironmentVar()");
    failures += test(testProcessManagerKill(), "ProcessManager.kill()");

    let attributes: Map<String, number> = getProcAttributes()
    failures += test(testGetPid(attributes), "process.pid test");
    failures += test(testGetTid(attributes), "process.tid test");
    failures += test(testGetPpid(attributes), "process.ppid test");
    failures += test(testGetUid(attributes), "process.uid test");
    failures += test(testGetEuid(attributes), "process.euid test");
    failures += test(testGetGid(attributes), "process.gid test");
    failures += test(testGetEgid(attributes), "process.egid test");
    failures += test(testGetGroups(attributes), "process.groups test");

    failures += test(testIs64Bit(), "process.is64Bit() test");
    failures += test(testGetStartRealTime(), "process.getStartRealTime() test");
    failures += test(testGetPastCpuTime(), "process.getPastCpuTime() test");

    failures += test(testRunCmdNoCondition(), "process.runCmd() no condition test");
    failures += test(testRunCmdWithTimeout(), "process.runCmd() with timeout test");
    failures += test(testRunCmdWithMaxBuffer(), "process.runCmd() with max buffer test");
    
    failures += test(testCwd(), "process.cwd() test");
    failures += test(testChangeDir(), "process.chdir() test");
    failures += test(testUptime(), "process.uptime() test");

    if (failures != 0) {
        console.log("FAILED: All tests run");
    }

    return failures;
}