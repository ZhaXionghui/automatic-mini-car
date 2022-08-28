import requests

requests.post('http://192.168.4.1/servo_control?angle=90')
requests.post('http://192.168.4.1/motor_control?speed=0')