# -- coding: utf-8 --
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

import pathlib
import tempfile
import unittest

from run import Config, load_config


class ConfigTest(unittest.TestCase):
    def test_loads_valid_env(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = pathlib.Path(directory) / ".env"
            env_path.write_text("HDC_PATH=/opt/hdc\n", encoding="utf-8")

            self.assertEqual(load_config(env_path),
                             Config(pathlib.Path("/opt/hdc")))

    def test_loads_quoted_value(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = pathlib.Path(directory) / ".env"
            env_path.write_text(
                "HDC_PATH='/opt/hdc with spaces'\n", encoding="utf-8")

            self.assertEqual(
                load_config(env_path),
                Config(pathlib.Path("/opt/hdc with spaces")),
            )

    def test_missing_env_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = pathlib.Path(directory) / ".env"

            with self.assertRaises(FileNotFoundError):
                load_config(env_path)

    def test_missing_hdc_path_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = pathlib.Path(directory) / ".env"
            env_path.write_text("OTHER=value\n", encoding="utf-8")

            with self.assertRaises(ValueError):
                load_config(env_path)

    def test_empty_hdc_path_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = pathlib.Path(directory) / ".env"
            env_path.write_text("HDC_PATH=\n", encoding="utf-8")

            with self.assertRaises(ValueError):
                load_config(env_path)


if __name__ == "__main__":
    unittest.main()
