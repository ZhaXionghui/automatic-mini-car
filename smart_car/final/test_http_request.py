# https://docs.python-requests.org/zh_CN/latest/

import requests
import time

r = requests.get('http://192.168.4.1/read_speed')
print(r.text)
#r = requests.get('http://192.168.4.1/read_angle')
#print(r.text)

# r = requests.post('http://192.168.4.1/right_light_control?state=0')
# print(r)

# r = requests.post('http://192.168.4.1/motor_control?speed=60')
# time.sleep(10)
# r = requests.post('http://192.168.4.1/motor_control?speed=0')

# r = requests.post('http://192.168.4.1/servo_control?angle=90')

