#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from __future__ import annotations

import os
import shutil
from pathlib import Path
from typing import Optional


def reset_path(path: Path) -> None:
    if path.is_symlink() or path.is_file():
        path.unlink()
        return
    if path.is_dir():
        shutil.rmtree(path)


def copy_path(source: Path, target: Path) -> Path:
    if source.is_dir():
        shutil.copytree(source, target)
    else:
        shutil.copy2(source, target)
    return target


def create_windows_directory_junction(link_path: Path, target_path: Path) -> Optional[Path]:
    if os.name != "nt" or not target_path.is_dir():
        return None

    try:
        resolved_target = target_path.resolve()
    except OSError:
        resolved_target = target_path

    if link_path.exists():
        try:
            if link_path.resolve() == resolved_target:
                return link_path
        except OSError:
            pass
        return None

    try:
        link_path.parent.mkdir(parents=True, exist_ok=True)
        link_path.mkdir()
    except OSError:
        return None

    try:
        import ctypes
        import struct
        from ctypes import wintypes

        io_reparse_tag_mount_point = 0xA0000003
        fsctl_set_reparse_point = 0x000900A4
        file_flag_open_reparse_point = 0x00200000
        file_flag_backup_semantics = 0x02000000
        generic_write = 0x40000000
        open_existing = 3
        file_share_all = 0x00000001 | 0x00000002 | 0x00000004
        invalid_handle_value = ctypes.c_void_p(-1).value

        substitute_name = f"\\??\\{resolved_target}"
        print_name = str(resolved_target)
        substitute_bytes = substitute_name.encode("utf-16-le")
        print_bytes = print_name.encode("utf-16-le")
        path_buffer = substitute_bytes + b"\x00\x00" + print_bytes + b"\x00\x00"
        reparse_data = struct.pack(
            "<LHHHHHH",
            io_reparse_tag_mount_point,
            8 + len(path_buffer),
            0,
            0,
            len(substitute_bytes),
            len(substitute_bytes) + 2,
            len(print_bytes),
        ) + path_buffer

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        create_file = kernel32.CreateFileW
        create_file.argtypes = [
            wintypes.LPCWSTR,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.HANDLE,
        ]
        create_file.restype = wintypes.HANDLE
        device_io_control = kernel32.DeviceIoControl
        device_io_control.argtypes = [
            wintypes.HANDLE,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
            ctypes.POINTER(wintypes.DWORD),
            wintypes.LPVOID,
        ]
        device_io_control.restype = wintypes.BOOL
        close_handle = kernel32.CloseHandle
        close_handle.argtypes = [wintypes.HANDLE]
        close_handle.restype = wintypes.BOOL

        handle = create_file(
            str(link_path),
            generic_write,
            file_share_all,
            None,
            open_existing,
            file_flag_open_reparse_point | file_flag_backup_semantics,
            None,
        )
        if handle == invalid_handle_value:
            raise ctypes.WinError(ctypes.get_last_error())

        try:
            bytes_returned = wintypes.DWORD()
            success = device_io_control(
                handle,
                fsctl_set_reparse_point,
                reparse_data,
                len(reparse_data),
                None,
                0,
                ctypes.byref(bytes_returned),
                None,
            )
            if not success:
                raise ctypes.WinError(ctypes.get_last_error())
        finally:
            close_handle(handle)
    except Exception:
        try:
            link_path.rmdir()
        except OSError:
            pass
        return None

    return link_path


def ensure_windows_junction(source: Path, target: Path) -> Optional[Path]:
    if os.name != "nt" or not source.is_dir():
        return None
    return create_windows_directory_junction(target, source)


def ensure_symlink(source: Path, target: Path) -> Path:
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists() or target.is_symlink():
        if target.is_symlink() and target.resolve() == source.resolve():
            return target
        reset_path(target)
    try:
        target.symlink_to(source, target_is_directory=source.is_dir())
        return target
    except OSError:
        reset_path(target)
        junction = ensure_windows_junction(source, target)
        if junction:
            return junction
        return copy_path(source, target)


def ensure_executable_script(script_path: Path, content: str) -> Path:
    script_path.parent.mkdir(parents=True, exist_ok=True)
    script_path.write_text(content, encoding="utf-8")
    if os.name != "nt":
        script_path.chmod(0o755)
    return script_path
