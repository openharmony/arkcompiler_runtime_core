import yaml
from utils.log import *
from utils.exceptions import InvalidFileFormatException


YAML_EXTENSIONS = ".yaml"
PARAM_SUFIX = ".params"


def generate_params(input, bench_name):
    input_dir = input.parent
    param_name = f"{bench_name}{PARAM_SUFIX}{YAML_EXTENSIONS}"
    param_path = input_dir.joinpath(param_name)
    
    result = dict()
    
    if(param_path.exists()):
        info(f"Generating yaml file for test: {input.name}")
        result = parse_yaml_list(param_path)
        info(f"Finish generating yaml file for test: {input.name}")
        
    return result


def parse_yaml_list(path):
    with open(path, "r", encoding='utf8') as f:
        text = f.read()
     
    try:
        params = yaml.load(text, Loader=yaml.FullLoader)
    except Exception as e:
        raise InvalidFileFormatException(message=f"Could not load YAML: {str(e)}", filepath=path)
    
    return params