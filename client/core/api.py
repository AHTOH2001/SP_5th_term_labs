from multiprocessing.connection import Client
from multiprocessing.connection import Listener
from typing import List

# адрес сервера (процесса в руте) для исходящих
# запросов
daemon = ('localhost', 6000)
# адрес клиента (этого процесса) для входящих
# ответов от сервера
cli = ('localhost', 6001)

def send(request: dict) -> bool or dict:
    """
    Принимает словарь аргументов удалённого метода.
    Отправляет запрос, после чего открывет сокет
    и ждет на нем ответ от сервера.
    """
    with Client(daemon) as conn:
        conn.send(request)
    with Listener(cli) as listener:
        with listener.accept() as conn:
            try:
                return conn.recv()
            except EOFError:
                return False

def send_message(message: str, to: List[str], protocol: str, token: str) -> send:
    """
    Формирует уникальный запрос и вызывает функцию
    send для его отправки.
    """
    return send({        
        "message": message,
        "to": to,
        "protocol": protocol,
        "method": 'message',
        'token': token
    })

def login(username: str, password: str) -> send:
    """
    Формирует уникальный запрос и вызывает функцию
    send для его отправки.
    """
    return send({        
        "username": username,
        "password": password,        
        "method": 'login'
    })
