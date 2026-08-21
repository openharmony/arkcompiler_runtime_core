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
import unittest

from src.result import ResultStore


_REMOTE_OUT_DIR = pathlib.PurePosixPath("/data/local/tmp/memmem-run")
_OUT_DIR = pathlib.Path.cwd().joinpath("out")


def _local(suffix: str) -> pathlib.Path:
    return _OUT_DIR.joinpath(suffix)


def _store(path: pathlib.Path = _OUT_DIR) -> ResultStore:
    return ResultStore(path, _REMOTE_OUT_DIR)


class ResultStoreTest(unittest.TestCase):
    def test_local_paths(self) -> None:
        store = _store()

        self.assertEqual(store.local_out_dir(), _OUT_DIR)
        self.assertEqual(store.local_flow_path(), _local("flow.json"))
        self.assertEqual(store.local_app_metadata_path(),
                         _local("app_metadata.json"))
        self.assertEqual(
            store.local_artifact_metadata_path(store.local_snapshots_dir()),
            _local("snapshots/metadata.json"),
        )
        self.assertEqual(
            store.local_artifact_metadata_path(store.local_screenshots_dir()),
            _local("screenshots/metadata.json"),
        )
        self.assertEqual(store.local_snapshots_dir(), _local("snapshots"))

        self.assertEqual(store.local_hilog_dir(), _local("hilog"))
        self.assertEqual(store.local_hilog_path(), _local("hilog/hilog.log"))
        self.assertEqual(store.hilog_relative_parts(), ["hilog.log"])
        self.assertEqual(store.local_screenshots_dir(), _local("screenshots"))
        self.assertEqual(store.local_screenshot_path("after_start"),
                         _local("screenshots/after_start.png"))
        self.assertEqual(store.screenshot_relative_parts(
            "after_start"), ["after_start.png"])

        self.assertEqual(store.snapshot_relative_parts(
            "after_start", "App"), ["after_start", "App-after_start.smaps"])
        self.assertEqual(store.local_snapshot_dir("after_start"),
                         _local("snapshots/after_start"))
        self.assertEqual(
            store.local_snapshot_path("after_start", "App"),
            _local("snapshots/after_start/App-after_start.smaps"),
        )
        self.assertEqual(store.local_plots_dir(), _local("plots"))
        self.assertEqual(store.local_plot_dir("App"),
                         _local("plots/App"))
        self.assertEqual(store.local_plot_path("App", "rss"),
                         _local("plots/App/App-rss.svg"))
        self.assertEqual(store.local_plot_path(
            "App", "rss", "_log"),
            _local("plots/App/App-rss_log.svg"))
        self.assertEqual(store.local_breakdowns_dir(),
                         _local("breakdowns"))

        self.assertEqual(store.local_breakdown_dir("after_start"),
                         _local("breakdowns/after_start"))
        self.assertEqual(
            store.local_breakdown_path("after_start", "App"),
            _local("breakdowns/after_start/App-after_start.csv"),
        )

    def test_remote_paths(self) -> None:
        store = _store()

        self.assertEqual(store.remote_out_dir(), _REMOTE_OUT_DIR)
        self.assertEqual(
            store.remote_hilog_dir(),
            pathlib.PurePosixPath("/data/local/tmp/memmem-run/hilog"),
        )
        self.assertEqual(
            store.remote_hilog_path(),
            pathlib.PurePosixPath(
                "/data/local/tmp/memmem-run/hilog/hilog.log"),
        )
        self.assertEqual(
            store.remote_screenshots_dir(),
            pathlib.PurePosixPath("/data/local/tmp/memmem-run/screenshots"),
        )
        self.assertEqual(
            store.remote_screenshot_path("after_start"),
            pathlib.PurePosixPath(
                "/data/local/tmp/memmem-run/screenshots/after_start.png"),
        )
        self.assertEqual(
            store.remote_snapshots_dir(),
            pathlib.PurePosixPath("/data/local/tmp/memmem-run/snapshots"),
        )
        self.assertEqual(
            store.remote_snapshot_dir("after_start"),
            pathlib.PurePosixPath(
                "/data/local/tmp/memmem-run/snapshots/after_start"),
        )
        self.assertEqual(
            store.remote_snapshot_path("after_start", "App"),
            pathlib.PurePosixPath(
                "/data/local/tmp/memmem-run/snapshots/after_start/"
                "App-after_start.smaps"),
        )


if __name__ == "__main__":
    unittest.main()
