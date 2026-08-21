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

import contextlib
import io
import pathlib
import tempfile
import unittest

from src.log import MemmemLogger, configure_logger, get_logger, parse_log_level, reset_logger


class LogTest(unittest.TestCase):
    def tearDown(self) -> None:
        reset_logger()

    def test_parse_log_level_accepts_known_levels(self) -> None:
        self.assertEqual(parse_log_level("info"), "info")
        self.assertEqual(parse_log_level("warn"), "warn")
        self.assertEqual(parse_log_level("err"), "err")

    def test_parse_log_level_rejects_unknown_level(self) -> None:
        with self.assertRaises(ValueError):
            parse_log_level("debug")

    def test_err_level_emits_only_errors(self) -> None:
        stdout = io.StringIO()
        stderr = io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            logger = MemmemLogger("err")

        logger.info("hidden info")
        logger.warn("hidden warning")
        logger.err("visible error")

        self.assertEqual(stdout.getvalue(), "")
        self.assertEqual(stderr.getvalue(), "error: visible error\n")

    def test_warn_level_emits_warnings_and_errors(self) -> None:
        stdout = io.StringIO()
        stderr = io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            logger = MemmemLogger("warn")

        logger.info("hidden info")
        logger.warn("visible warning")
        logger.err("visible error")

        self.assertEqual(stdout.getvalue(), "warning: visible warning\n")
        self.assertEqual(stderr.getvalue(), "error: visible error\n")

    def test_info_level_emits_all_levels(self) -> None:
        stdout = io.StringIO()
        stderr = io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            logger = MemmemLogger("info")

        logger.info("visible info")
        logger.warn("visible warning")
        logger.err("visible error")

        self.assertEqual(
            stdout.getvalue(),
            "info: visible info\nwarning: visible warning\n",
        )
        self.assertEqual(stderr.getvalue(), "error: visible error\n")

    def test_file_logging_writes_all_emitted_levels_only_to_file(self) -> None:
        stdout = io.StringIO()
        stderr = io.StringIO()
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory).joinpath("memmem.log")
            with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
                logger = MemmemLogger("info", path)
            try:
                logger.info("visible info")
                logger.warn("visible warning")
                logger.err("visible error")
            finally:
                logger.reset()

            self.assertEqual(stdout.getvalue(), "")
            self.assertEqual(stderr.getvalue(), "")
            self.assertEqual(
                path.read_text(encoding="utf-8"),
                "info: visible info\n"
                "warning: visible warning\n"
                "error: visible error\n",
            )

    def test_file_logging_overwrites_existing_file(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory).joinpath("memmem.log")
            path.write_text("old\n", encoding="utf-8")
            logger = MemmemLogger("err", path)
            try:
                logger.err("new")
            finally:
                logger.reset()

            self.assertEqual(path.read_text(encoding="utf-8"), "error: new\n")

    def test_file_logging_fails_when_parent_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory).joinpath("missing", "memmem.log")

            with self.assertRaises(FileNotFoundError):
                MemmemLogger("err", path)

    def test_file_logging_rejects_empty_string_path(self) -> None:
        with self.assertRaises(Exception):
            MemmemLogger("err", "")

    def test_configure_logger_updates_singleton(self) -> None:
        stdout = io.StringIO()

        with contextlib.redirect_stdout(stdout):
            configured = configure_logger("info")

        configured.info("configured")

        self.assertIs(get_logger(), configured)
        self.assertEqual(stdout.getvalue(), "info: configured\n")

    def test_configure_logger_replaces_existing_logger(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory).joinpath("memmem.log")
            first = configure_logger("info", path)
            first.info("first")

            second = configure_logger("info", path)
            second.info("second")

            self.assertIsNot(first, second)
            self.assertEqual(
                path.read_text(encoding="utf-8"),
                "info: second\n",
            )


if __name__ == "__main__":
    unittest.main()
