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

from src.metadata import RelativeParts


class ResultStore:
    def __init__(
        self,
        local_out_dir: pathlib.Path,
        remote_out_dir: pathlib.PurePosixPath,
    ) -> None:
        self._local_out_dir = local_out_dir
        self._remote_out_dir = remote_out_dir

    def local_out_dir(self) -> pathlib.Path:
        return self._local_out_dir

    def local_summary_path(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("summary.csv")

    def local_flow_path(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("flow.json")

    def local_app_metadata_path(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("app_metadata.json")

    def local_artifact_metadata_path(self, path: pathlib.Path) -> pathlib.Path:
        return path.joinpath("metadata.json")

    def local_snapshots_dir(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("snapshots")

    def local_snapshot_dir(self, snapshot_label: str) -> pathlib.Path:
        return self.local_snapshots_dir().joinpath(snapshot_label)

    def local_snapshot_path(
        self,
        snapshot_label: str,
        app_label: str,
    ) -> pathlib.Path:
        return self.local_snapshot_dir(snapshot_label).joinpath(
            f"{app_label}-{snapshot_label}.smaps")

    def snapshot_relative_parts(self, snapshot_label: str, app_label: str) -> RelativeParts:
        return [snapshot_label, f"{app_label}-{snapshot_label}.smaps"]

    def local_hilog_dir(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("hilog")

    def local_hilog_path(self) -> pathlib.Path:
        return self.local_hilog_dir().joinpath("hilog.log")

    def hilog_relative_parts(self) -> RelativeParts:
        return ["hilog.log"]

    def local_screenshots_dir(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("screenshots")

    def local_screenshot_path(self, screenshot_label: str) -> pathlib.Path:
        return self.local_screenshots_dir().joinpath(f"{screenshot_label}.png")

    def screenshot_relative_parts(self, screenshot_label: str) -> RelativeParts:
        return [f"{screenshot_label}.png"]

    def local_plots_dir(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("plots")

    def local_plot_dir(self, app_label: str) -> pathlib.Path:
        return self.local_plots_dir().joinpath(app_label)

    def local_plot_path(
        self,
        app_label: str,
        metric: str,
        suffix: str = "",
        extension: str = "svg",
    ) -> pathlib.Path:
        return self.local_plot_dir(app_label).joinpath(
            f"{app_label}-{metric}{suffix}.{extension}")

    def local_breakdowns_dir(self) -> pathlib.Path:
        return self._local_out_dir.joinpath("breakdowns")

    def local_breakdown_dir(self, snapshot_label: str) -> pathlib.Path:
        return self.local_breakdowns_dir().joinpath(snapshot_label)

    def local_breakdown_path(
        self,
        snapshot_label: str,
        app_label: str,
    ) -> pathlib.Path:
        return self.local_breakdown_dir(snapshot_label).joinpath(
            f"{app_label}-{snapshot_label}.csv")

    def remote_out_dir(self) -> pathlib.PurePosixPath:
        return self._remote_out_dir

    def remote_hilog_dir(self) -> pathlib.PurePosixPath:
        return self._remote_out_dir.joinpath("hilog")

    def remote_hilog_path(self) -> pathlib.PurePosixPath:
        return self.remote_hilog_dir().joinpath("hilog.log")

    def remote_screenshots_dir(self) -> pathlib.PurePosixPath:
        return self._remote_out_dir.joinpath("screenshots")

    def remote_screenshot_path(self, screenshot_label: str) -> pathlib.PurePosixPath:
        return self.remote_screenshots_dir().joinpath(f"{screenshot_label}.png")

    def remote_snapshots_dir(self) -> pathlib.PurePosixPath:
        return self._remote_out_dir.joinpath("snapshots")

    def remote_snapshot_dir(self, snapshot_label: str) -> pathlib.PurePosixPath:
        return self.remote_snapshots_dir().joinpath(snapshot_label)

    def remote_snapshot_path(
        self,
        snapshot_label: str,
        app_label: str,
    ) -> pathlib.PurePosixPath:
        return self.remote_snapshot_dir(snapshot_label).joinpath(
            f"{app_label}-{snapshot_label}.smaps")
