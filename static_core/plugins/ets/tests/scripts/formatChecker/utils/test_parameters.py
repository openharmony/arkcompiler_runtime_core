# This file provides functions thar are responsible for loading test parameters
# that are used in templates (yaml lists).

import yaml 
import os.path as ospath

from utils.exceptions import InvalidFileFormatException
from utils.fsutils import iter_files, read_file
from utils.constants import YAML_EXTENSIONS, LIST_PREFIX


Params = dict


def load_params(dirpath: str) -> Params:
    """
    Loads all parameters for a directory 
    """
    result = dict()

    for filename, filepath in iter_files(dirpath, allowed_ext=YAML_EXTENSIONS):
        name_without_ext, _ = ospath.splitext(filename)
        if not name_without_ext.startswith(LIST_PREFIX):
            raise InvalidFileFormatException(message="Lists of parameters must start with 'list.'", filepath=filepath)
        listname = name_without_ext[len(LIST_PREFIX):]
        result[listname] = __parse_yaml_list(filepath)
    return result


def __parse_yaml_list(path: str) -> Params:
    """
    Parses a single YAML list of parameters
    Verifies that it is a list
    """
    text = read_file(path)
    
    try:
        params = yaml.load(text, Loader=yaml.FullLoader)
    except Exception as e:
        raise InvalidFileFormatException(message=f"Could not load YAML: {str(e)}", filepath=path)
    
    if type(params) != list:
        raise InvalidFileFormatException(message="Parameters list must be YAML array", filepath=path)
    return params