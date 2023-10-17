class InvalidFileFormatException(Exception):
    def __init__(self, filepath: str, message: str) -> None:
        super().__init__(f"{filepath}: {message}")
        self.message = f"{filepath}: {message}"
    

class InvalidFileStructureException(Exception):
    def __init__(self, filepath: str, message: str) -> None:
        msg = f"{filepath}: {message}"
        super().__init__(msg)
        self.message = msg


class UnknownTemplateException(Exception):
    def __init__(self, filepath: str, exception: Exception) -> None:
        super().__init__(f"{filepath}: {str(exception)}")
        self.filepath = filepath
        self.exception = exception