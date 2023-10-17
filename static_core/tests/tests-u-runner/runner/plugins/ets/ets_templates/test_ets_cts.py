from pathlib import Path
from typing import List

from runner.plugins.ets.test_ets import TestETS
from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata


class TestEtsCts(TestETS):
    def __init__(self, test_env, test_path, flags, test_id):
        TestETS.__init__(self, test_env, test_path, flags, test_id)
        self.metadata: TestMetadata = get_metadata(Path(test_path))
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"

    @property
    def is_negative_runtime(self) -> bool:
        return self.metadata.tags.negative and not self.metadata.tags.compile_only

    @property
    def is_negative_compile(self) -> bool:
        return self.metadata.tags.negative and self.metadata.tags.compile_only

    @property
    def is_compile_only(self) -> bool:
        return self.metadata.tags.compile_only

    @property
    def is_valid_test(self) -> bool:
        return not self.metadata.tags.not_a_test

    def ark_extra_options(self) -> List[str]:
        return self.metadata.ark_options

    @property
    def ark_timeout(self) -> int:
        return self.metadata.timeout if self.metadata.timeout else super().ark_timeout
