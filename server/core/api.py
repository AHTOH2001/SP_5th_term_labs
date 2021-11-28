from .database import Database 

def hello(request: dict) -> str:
    """
    Привилегированный системный вызов.
    """
    return " ".join(["Hello", request["name"]])

def login(request: dict) -> str:
    credentials = (request['username'], request['password'])
    if credentials in Database().users:
        return Database().users[credentials]
    else:
        return 'Not found'

def message(request: dict) -> str:
    if 'token' not in request:
        return 'Token was not provided'
    if request['token'] not in Database().tokens:
        return 'Wrong token'
    return f"{Database().tokens[request['token']]}, your message: {request['message']}, \nsent to {request['to']}, \nusing {request['protocol']}"
