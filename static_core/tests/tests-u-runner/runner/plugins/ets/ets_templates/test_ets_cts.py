from __future__ import annotations

from pathlib import Path
from typing import List, Sequence

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

    @property
    def dependent_files(self) -> Sequence[TestETS]:
        if not self.metadata.files:
            return []

        tests = []
        for file in self.metadata.files:
            path = Path(self.path).parent / Path(file)
            test_id = Path(self.test_id).parent / Path(file)
            tests.append(self.__class__(self.test_env, str(path), self.flags, test_id))
        return tests

    @property
    def runtime_args(self) -> List[str]:
        if not self.dependent_files:
            return super().runtime_args
        return self.add_boot_panda_files(super().runtime_args)

    @property
    def verifier_args(self) -> List[str]:
        if not self.dependent_files:
            return super().verifier_args
        return self.add_boot_panda_files(super().verifier_args)

    def add_boot_panda_files(self, args: List[str]) -> List[str]:
        dep_files_args = []
        for arg in args:
            name, value = arg.split('=')
            if name == '--boot-panda-files':
                dep_files_args.append(f'{name}={":".join([value] + [dt.test_abc for dt in self.dependent_files])}')
            else:
                dep_files_args.append(f'{name}={value}')
        return dep_files_args
