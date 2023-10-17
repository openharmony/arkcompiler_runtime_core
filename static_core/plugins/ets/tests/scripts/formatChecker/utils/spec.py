import yaml 
from utils.file_structure import TestDirectory, normalize_section_name
from utils.exceptions import InvalidFileFormatException
from utils.constants import SPEC_SECTION_TITLE_FIELD_NAME, SPEC_SUBSECTIONS_FIELD_NAME
from utils.fsutils import read_file
from pathlib import Path

Spec = dict

def build_spec_tree(specpath: str, root: TestDirectory):
    spec = __parse_spec(specpath)
    walk_spec(spec, root, specpath=specpath)


def walk_spec(spec: list, parent: TestDirectory, specpath: str): 
    if type(spec) != list:
        raise InvalidFileFormatException(message="Spec sections list must be a YAML list", filepath=specpath)

    for s in spec:
        if type(s) != dict:
            raise InvalidFileFormatException(message="Spec section must be a YAML dict", filepath=specpath)
        
        s = dict(s)
        if not SPEC_SECTION_TITLE_FIELD_NAME in s:
            raise InvalidFileFormatException(message=f"Spec section must contain the '{SPEC_SECTION_TITLE_FIELD_NAME}' field", filepath=specpath)

        # Normalize name 
        name = normalize_section_name(str(s[SPEC_SECTION_TITLE_FIELD_NAME]))
       
        td = TestDirectory(path=(parent.path / Path(name)), parent=parent) 
        parent.add_subdir(td)   

        # Has subsections?
        if SPEC_SUBSECTIONS_FIELD_NAME in s: 
            walk_spec(s[SPEC_SUBSECTIONS_FIELD_NAME], parent=td, specpath=specpath)


def __parse_spec(path: str) -> Spec:
    text = read_file(path)

    try:
        spec = yaml.load(text, Loader=yaml.FullLoader)
    except Exception as e:
        raise InvalidFileFormatException(message=f"Could not load YAML: {str(e)}", filepath=path)
    
    return spec