# Copyright (c) 2025 Huawei Device Co., Ltd.
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
from pathlib import Path

from taihe.driver import CompilerInstance, CompilerInvocation


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-I",
        dest="src_dirs",
        nargs="*",
        required=True,
        help="directories of .taihe source files",
    )
    parser.add_argument(
        "-O",
        dest="dst_dir",
        required=True,
        help="directory for generated .h and .cpp files",
    )
    parser.add_argument(
        "--author",
        "-a",
        action="store_true",
        help="generate files for interface author",
    )
    parser.add_argument(
        "--user",
        "-u",
        action="store_true",
        help="generate files for interface user",
    )
    parser.add_argument(
        "--ani",
        action="store_true",
        help="generate ani headers and source",
    )
    parser.add_argument(
        "--c-impl",
        action="store_true",
        help="generate cimpl headers and source",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="debug mode",
    )
    args = parser.parse_args()
    invocation = CompilerInvocation(
        src_dirs=[Path(d) for d in args.src_dirs],
        out_dir=Path(args.dst_dir),
        gen_author=args.author,
        gen_user=args.user,
        gen_ani=args.ani,
        gen_c_impl=args.c_impl,
        debug=args.debug,
    )
    instance = CompilerInstance(invocation)
    instance.run()


if __name__ == "__main__":
    main()
