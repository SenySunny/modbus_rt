def add(a: int, b: int, c: float) -> float: ...

def test_list(l: list):
    """ 集合数据交换: list """

def test_dict(d: dict):
    """ 集合数据交换: dict """

def list_return() -> list:
    """ 返回集合数据: list """

def dict_return() -> dict:
    """ 返回集合数据: dict """

def test_any(a: any) -> any:
    """ 任意类型数据: any """

def test_args(*_args):
    """ 任意数目数据: *args """

def test_kwargs(**_kwargs):
    """ 任意关键字数据: **kwargs """


class Test:
    def print(self, name: str):
        """ 打印对象成员 """

def get_module_var(name: str) -> any:
    """ 获取模块变量 """
