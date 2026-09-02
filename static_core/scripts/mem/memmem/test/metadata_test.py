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

import unittest

from src.metadata import (
    AppMetadata,
    AppMetadataFile,
    ArtifactMetadata,
    ArtifactMetadataFile,
)


class MetadataTest(unittest.TestCase):
    def test_metadata_models_serialize(self) -> None:
        app = AppMetadata(
            pid=123,
            label="App",
            bundle="com.example",
            ability="EntryAbility",
        )
        app_metadata = AppMetadataFile(apps=[app])
        artifact_metadata = ArtifactMetadataFile(
            artifacts=[
                ArtifactMetadata(
                    label="after_start",
                    timestamp="1700000000000000000",
                )
            ]
        )

        self.assertEqual(AppMetadataFile.model_validate_json(
            app_metadata.model_dump_json()), app_metadata)
        self.assertEqual(
            ArtifactMetadataFile.model_validate_json(
                artifact_metadata.model_dump_json()),
            artifact_metadata,
        )


if __name__ == "__main__":
    unittest.main()
