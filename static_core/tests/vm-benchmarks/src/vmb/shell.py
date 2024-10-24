#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import re
import os
import logging
import signal
from typing import Union, Optional
from pathlib import Path
from subprocess import Popen, PIPE
from threading import Thread, Timer
from dataclasses import dataclass
from tempfile import mktemp
from vmb.helpers import Singleton

log = logging.getLogger('vmb')
tm_re = re.compile(
    r"(?:Elapsed.*\(h:mm:ss or m:ss\)|Real time)"
    r"[^:]*:\s*(?:(\d*):)?(\d*)(?:.(\d*))?")
rss_re = re.compile(r"(?:Maximum resident set size|Max RSS)[^:]*:\s*(\d*)")


@dataclass
class ShellResult:

    # Default initial result is 'failure'
    ret: int = -13
    out: str = ''
    err: str = ''
    tm: float = 0.0
    rss: int = 0

    def grep(self, regex: str) -> str:
        out = self.out.split("\n")
        err = self.err.split("\n")
        for line in out + err:
            m = re.search(regex, line)
            if m:
                if len(m.groups()) < 1:
                    return m.group()
                return m.group(1)
        return ''

    def set_ret_val(self) -> None:
        if not self.out:
            log.error("No shell output")
            self.ret = -13
        matches = re.search(r"__RET_VAL__=(\d*)", self.out)
        if not matches:
            log.error("No shell ret val; out:")
            self.ret = -13
        else:
            self.ret = int(matches.groups()[0])

    def set_time(self) -> None:
        # expecting output of '\time -v' to stderr
        if not self.err:
            return
        tm_val = re.search(tm_re, self.err)
        if tm_val:
            tmp = tm_val.groups()
            if tmp[0] is None:
                self.tm = float(str(tmp[1]) + "." + tmp[2])
            else:
                self.tm = int(tmp[0]) * 60 + float(str(tmp[1]) + "." + tmp[2])
            self.tm = round(self.tm, 5)
        else:
            self.tm = 0.0
        self.tm *= 1e9
        rss_val = re.search(rss_re, self.err)
        if rss_val:
            self.rss = int(rss_val.group(1))
        else:
            self.rss = 0

    def log_output(self) -> None:
        if self.ret != 0:
            if self.out:
                log.error(self.out)
            err = self.err.split("\n")[:3] if self.err else []
            for line in err:
                log.error(line.strip())
        else:
            if self.out:
                log.debug(self.out)


class ShellBase(metaclass=Singleton):

    def __init__(self, timeout: Optional[float] = None) -> None:
        self._timeout = timeout
        self.taskset = ''

    @staticmethod
    def timed_cmd(cmd: str) -> str:
        return f"\\time -v env {cmd}"

    def run(self,
            cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        raise NotImplementedError

    def run_async(self, cmd: str) -> None:
        raise NotImplementedError

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def get_filesize(self, filepath: Union[str, Path]) -> int:
        if os.path.exists(str(filepath)):
            return os.stat(str(filepath)).st_size
        return 0

    def grep_output(self, cmd: str, regex: str) -> str:
        r = self.run(cmd=cmd)
        return r.grep(regex)

    def set_affinity(self, arg: str) -> None:
        """Set affinity mask for processes.

        Effective only on devices, so hardcoding path
        """
        self.taskset = f'/system/bin/taskset -a {arg}'


class ShellUnix(ShellBase):

    def __init__(self, timeout: Optional[float] = None) -> None:
        super().__init__(timeout=timeout)

    def run(self,
            cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        return self.__run(
            cmd, measure_time=measure_time, timeout=timeout, cwd=cwd)

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def run_async(self, cmd: str) -> None:
        def run_shell():
            # pylint: disable-next=all
            return Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)  # NOQA

        log.debug('Async cmd: %s', cmd)
        async_trhead = Thread(target=run_shell)
        async_trhead.daemon = True
        async_trhead.start()

    def __run(self,
              cmd: str,
              measure_time: bool = False,
              timeout: Optional[float] = None,
              cwd: str = '') -> ShellResult:
        if measure_time:
            cmd = self.timed_cmd(cmd)
        result = self.__exec_process(cmd, cwd=cwd, timeout=timeout)
        if measure_time:
            result.set_time()
        result.log_output()
        return result

    def __exec_process(self, cmd: str, cwd: str = '',
                       timeout: Optional[float] = None) -> ShellResult:
        result = ShellResult()
        # Note: self._timeout=None so default behaivior is to wait forever
        to = timeout if timeout else self._timeout
        if timeout is not None and self._timeout is not None:
            to = max(timeout, self._timeout)
        log.debug(cmd)
        log.trace('CWD="%s" Timeout=[%s]', cwd, to)
        # pylint: disable-next=all
        with Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE,  # NOQA
                   cwd=(cwd if cwd else None),
                   preexec_fn=os.setsid) as proc:
            if to is not None:
                timer = Timer(to,
                              lambda x: os.killpg(
                                  os.getpgid(x.pid), signal.SIGKILL), [proc])
                timer.start()
            out, err = proc.communicate(timeout=to)
            if to is not None:
                timer.cancel()
            ret_code = proc.poll()
            if ret_code is not None:
                result.ret = ret_code
            result.out = out.decode('utf-8')
            result.err = err.decode('utf-8')
        return result


class ShellDevice(ShellBase):
    def __init__(self,
                 dev_sh: str,
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb',) -> None:
        super().__init__(timeout=timeout)
        self._sh = ShellUnix()
        self._devsh = dev_sh
        self.tmp_dir = tmp_dir
        self.stderr_out = os.path.join(tmp_dir, 'vmb-stderr.out')

    def run(self, cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        redir = ''
        if measure_time:
            cmd = f"\\time -v {self.taskset} env {cmd}"
            redir = f' 2>{self.stderr_out}'
        cwd = f'cd {cwd}; ' if cwd else ''
        res = self._sh.run(
            f"{self._devsh} shell '{cwd}({cmd}){redir}; echo __RET_VAL__=$?'",
            timeout=timeout,
            measure_time=False)
        res.set_ret_val()
        if measure_time:
            stderr_host = mktemp(prefix='vmb-')
            self.pull(self.stderr_out, stderr_host)
            self._sh.run(f"{self._devsh} shell 'rm -f {self.stderr_out}'")
            if not Path(stderr_host).exists():
                res.err = 'Pull from device failed'
                return res
            with open(stderr_host, 'r', encoding="utf-8") as f:
                res.err = f.read()
            self._sh.run(f'rm -f {stderr_host}')
            res.set_time()
        else:
            res.err = ''
        return res

    def run_async(self, cmd: str) -> None:
        self._sh.run_async(f"{self._devsh} shell '{cmd}'")

    def get_filesize(self, filepath: Union[str, Path]) -> int:
        res = self.run(f"stat -c '%s' {filepath}")
        if res.ret == 0 and res.out:
            return int(res.out.split("\n")[0])
        return 0

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def mk_tmp_dir(self):
        res = self.run(f'mkdir -p {self.tmp_dir}')
        if res.ret != 0:
            raise RuntimeError('Device connection failed!\n'
                               f'{res.out}\n{res.err}')


class ShellAdb(ShellDevice):
    binname = f"a{'d'}b"

    def __init__(self,
                 dev_serial: str = '',
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb') -> None:
        super().__init__(
            f"{os.environ.get(self.binname.upper(), self.binname)}",
            timeout=timeout,
            tmp_dir=tmp_dir)
        if dev_serial:
            self._devsh = f'{self._devsh} -s {dev_serial}'
        self.mk_tmp_dir()

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        return self._sh.run(f'{self._devsh} push {src} {dst}',
                            measure_time=False)

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        return self._sh.run(f'{self._devsh} pull {src} {dst}',
                            measure_time=False)


class ShellHdc(ShellDevice):
    def __init__(self,
                 dev_serial: str = '',
                 dev_host: str = '',
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb') -> None:
        # -l0 because of HDC mutex file permission messages
        # -p (undocumented) due to poor hdc performance
        super().__init__(f"{os.environ.get('HDC', 'hdc')} -p -l0",
                         timeout=timeout,
                         tmp_dir=tmp_dir)
        if dev_serial:
            self._devsh = f'{self._devsh} -t {dev_serial}'
        if dev_host:
            self._devsh = f'{self._devsh} -s {dev_host}'
        self.mk_tmp_dir()

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        return self._sh.run(f'{self._devsh} file send {src} {dst}',
                            measure_time=False)

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        return self._sh.run(f'{self._devsh} file recv {src} {dst}',
                            measure_time=False)
