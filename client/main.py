from core import api

response = api.send_message('Hello world', ['Serega'], 'HTTPS', '')
print(response)
token = api.login('Toha', '12345')
print(token)
if response != 'Not found':
    response = api.send_message('Hello world', ['Serega'], 'HTTPS', token)
    print(response)


while True:
    message = input('\nInput custom message: ')
    response = api.send_message(message, ['Serega'], 'HTTPS', token)
    print(response)