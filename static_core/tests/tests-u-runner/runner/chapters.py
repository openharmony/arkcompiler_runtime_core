import logging
from dataclasses import dataclass
from typing import Optional, Dict, Any, Union, List, Sequence

from runner.logger import Log
from runner.options.yaml_document import YamlDocument


@dataclass
class Chapter:
    name: str
    includes: List[str]
    excludes: List[str]


class IncorrectFileFormatChapterException(Exception):
    def __init__(self, chapters_name: str) -> None:
        message = f"Incorrect file format: {chapters_name}"
        Exception.__init__(self, message)


class CyclicDependencyChapterException(Exception):
    def __init__(self, item: str) -> None:
        message = f"Cyclic dependency: {item}"
        Exception.__init__(self, message)


Chapters = Dict[str, Chapter]


def __validate_cycles(chapters: Chapters) -> None:
    """
    :param chapters: list (dictionary) of chapters, key is a chapter name
    :raise: CyclicDependencyChapterException if a cyclic dependency found
        Normal finish means that no cycle found
    """
    seen_chapters: List[str] = []
    for name, chapter in chapters.items():
        seen_chapters.append(name)
        __check_cycle(chapter, chapters, seen_chapters)
        seen_chapters.pop()


_LOGGER = logging.getLogger("runner.chapters")


def __check_cycle(chapter: Chapter, all_chapters: Chapters, seen_chapters: List[str]) -> None:
    """
    Checks if items contains any name from seen
    :param chapter: investigated chapter
    :param all_chapters: array of all chapters
    :param seen_chapters: array of seen items' names
    :raise: CyclicDependencyChapterException if a name of nested chapter is in seen names
        Normal finish means that no cycle found
    """
    for item in chapter.includes + chapter.excludes:
        if item not in all_chapters.keys():
            continue
        if item in seen_chapters:
            Log.exception_and_raise(_LOGGER, item, CyclicDependencyChapterException)
        seen_chapters.append(item)
        __check_cycle(all_chapters[item], all_chapters, seen_chapters)
        seen_chapters.pop()


def __parse(chapters_file: str) -> Chapters:
    result: Chapters = {}
    YamlDocument.load(chapters_file)
    conf = YamlDocument.document()
    yaml_header: Optional[Dict[str, Any]] = conf
    if not yaml_header or not isinstance(yaml_header, dict):
        Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
    yaml_chapters: Optional[List[Dict[str, List[str]]]] = yaml_header.get('chapters')
    if not yaml_chapters or not isinstance(yaml_chapters, list):
        Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
    for yaml_chapter in yaml_chapters:
        if not isinstance(yaml_chapter, dict):
            Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
        for yaml_name, yaml_items in yaml_chapter.items():
            chapter = __parse_chapter(yaml_name, yaml_items, chapters_file)
            if chapter.name in result:
                Log.exception_and_raise(
                    _LOGGER,
                    f"Chapter '{chapter.name}' already used",
                    IncorrectFileFormatChapterException)
            result[chapter.name] = chapter
    return result


def __parse_item(includes: List[str], excludes: List[str], yaml_item: Union[str, dict]) -> None:
    if isinstance(yaml_item, str):
        includes.append(yaml_item.strip())
    elif isinstance(yaml_item, dict):
        for sub_name, sub_items in yaml_item.items():
            if sub_name == 'exclude' and isinstance(sub_items, list):
                excludes.extend(sub_items)


def __parse_chapter(name: str, yaml_items: Sequence[Union[str, Dict[str, str]]], chapters_file: str) -> Chapter:
    if not isinstance(yaml_items, list):
        Log.exception_and_raise(_LOGGER, f"Incorrect file format: {chapters_file}", IncorrectFileFormatChapterException)
    includes: List[str] = []
    excludes: List[str] = []
    for yaml_item in yaml_items:
        __parse_item(includes, excludes, yaml_item)

    return Chapter(name, includes, excludes)


def parse_chapters(chapters_file: str) -> Chapters:
    chapters = __parse(chapters_file)
    __validate_cycles(chapters)
    return chapters
