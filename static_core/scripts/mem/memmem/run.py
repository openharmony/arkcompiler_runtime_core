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

import argparse
import dataclasses
from datetime import datetime
import json
import pathlib
import re
import sys

from dotenv import dotenv_values

from src.schema import UnprocessedFlow
from src.types import positive_int
from src.path import absolutize

import lib


@dataclasses.dataclass(frozen=True)
class Config:
    hdc_path: pathlib.Path


def load_config(env_path: pathlib.Path) -> Config:
    if not env_path.is_file():
        raise FileNotFoundError(f"env file does not exist: {env_path}")

    values = dotenv_values(env_path)
    hdc_path = values.get("HDC_PATH", "")
    if hdc_path is None or hdc_path == "":
        raise ValueError("HDC_PATH is required")

    return Config(hdc_path=absolutize(pathlib.Path(hdc_path)))


def _compile_smaps_filter(value: str) -> re.Pattern[str]:
    try:
        return re.compile(value)
    except re.error:
        raise argparse.ArgumentTypeError(
            f"invalid regex: {value}") from None


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Benchmark mobile device memory usage")
    parser.add_argument("--flow", required=True, type=pathlib.Path)
    parser.add_argument(
        "--out-dir",
        required=False,
        type=pathlib.Path,
        default=None,
        help="directory to store benchmark results",
    )
    parser.add_argument(
        "--repeats",
        required=False,
        type=positive_int,
        default=1,
        help="number of times to repeat the flow; positive int",
    )
    reboot_group = parser.add_mutually_exclusive_group()
    reboot_group.add_argument(
        "--reboot", dest="reboot", action="store_true", default=False
    )
    reboot_group.add_argument(
        "--no-reboot", dest="reboot", action="store_false")
    hilog_group = parser.add_mutually_exclusive_group()
    hilog_group.add_argument("--hilog", dest="hilog",
                             action="store_true", default=True)
    hilog_group.add_argument("--no-hilog", dest="hilog", action="store_false")
    parser.add_argument(
        "--memmem-log-level",
        choices=["info", "warn", "err"],
        default="err",
    )
    parser.add_argument("--memmem-log-file", required=False,
                        type=pathlib.Path, default=None)
    parser.add_argument(
        "--smaps-filter",
        required=False,
        type=_compile_smaps_filter,
        default=None,
        help=(
            "regex matched with re.match against each normalized smaps tag; "
            "only matching mappings are counted in reports; "
            "e.g. '.*\\.so' counts contribution of only shared-library mappings"
        ),
    )
    return parser


def main() -> int:
    args = build_parser().parse_args()
    flow_path = absolutize(args.flow)
    log_file: pathlib.Path | None = None
    if args.memmem_log_file is not None:
        log_file = absolutize(pathlib.Path(args.memmem_log_file))
    if args.out_dir is None:
        out_dir = absolutize(pathlib.Path(
            f"memmem-out-{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"))
    else:
        out_dir = absolutize(args.out_dir)
    try:
        lib.configure_logger(args.memmem_log_level, log_file)
        unprocessed = UnprocessedFlow.model_validate(json.loads(
            flow_path.read_text(encoding="utf-8")))
        flow = lib.preprocess_flow(unprocessed)
        config = load_config(pathlib.Path(".env"))
        hdc = lib.get_hdc(config.hdc_path)
        device = lib.get_device(hdc)
        lib.run(
            flow,
            device,
            out_dir=out_dir,
            reboot=args.reboot,
            hilog=args.hilog,
            repeats=args.repeats,
            smaps_filter=args.smaps_filter,
        )
    except Exception as error:
        print(f"error: memmem error: {error}", file=sys.stderr)
        return 1
    finally:
        lib.reset_logger()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
