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

import unittest
from unittest import mock

from support import load_module


class FsUtilsTest(unittest.TestCase):
    def test_ensure_symlink_skips_rebuild_when_target_already_points_to_source(self) -> None:
        fs_utils = load_module("fs_utils", "src/arkts_migration_visualizer/common/fs_utils.py")
        source = mock.Mock()
        target = mock.Mock()
        source.resolve.return_value = "same-path"
        target.resolve.return_value = "same-path"
        target.parent = mock.Mock()
        target.exists.return_value = True
        target.is_symlink.return_value = False

        with mock.patch.object(fs_utils, "reset_path") as reset_path_mock:
            resolved = fs_utils.ensure_symlink(source, target)

        self.assertEqual(resolved, target)
        target.parent.mkdir.assert_called_once_with(parents=True, exist_ok=True)
        reset_path_mock.assert_not_called()
        target.symlink_to.assert_not_called()

    def test_reset_path_removes_windows_directory_reparse_points_without_rmtree(self) -> None:
        fs_utils = load_module("fs_utils", "src/arkts_migration_visualizer/common/fs_utils.py")
        path = mock.Mock()
        path.exists.return_value = True
        path.is_symlink.return_value = False
        path.is_file.return_value = False
        path.is_dir.return_value = True

        with mock.patch.object(fs_utils.os, "name", "nt"), mock.patch.object(
            fs_utils,
            "is_windows_reparse_point",
            return_value=True,
        ), mock.patch.object(fs_utils.shutil, "rmtree") as rmtree_mock:
            fs_utils.reset_path(path)

        path.unlink.assert_not_called()
        path.rmdir.assert_called_once_with()
        rmtree_mock.assert_not_called()


if __name__ == "__main__":
    unittest.main()
