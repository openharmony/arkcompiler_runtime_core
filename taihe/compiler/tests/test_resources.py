# coding=utf-8
#
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

from taihe.utils.resources import DeploymentMode, ResourceLocator


def test_dev():
    p = "repo_root/compiler/taihe/utils/resources.py"
    loc = ResourceLocator.detect(p)
    pass
    assert loc.root_dir.name == "repo_root"


def test_pkg():
    p = ".venv/lib/python3.12/site-packages/taihe/utils/resources.py"
    loc = ResourceLocator.detect(p)
    pass
    assert loc.root_dir.name == "taihe"


def test_bundle():
    p = "taihe-pkg/lib/pyrt/lib/python3.11/site-packages/taihe/utils/resources.py"
    loc = ResourceLocator.detect(p)
    pass
    assert loc.root_dir.name == "taihe-pkg"