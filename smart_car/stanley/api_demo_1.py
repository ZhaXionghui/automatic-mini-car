import requests
import time

# r = requests.get('http://192.168.4.1/read_speed')
# print(r.text)

# r = requests.get('http://192.168.4.1/read_angle')
# print(r.text)

requests.post('http://192.168.4.1/servo_control?angle=90')
time.sleep(1)

requests.post('http://192.168.4.1/motor_control?speed=60')
time.sleep(3)

r = requests.get('http://192.168.4.1/read_angle')
print(r.text)

requests.post('http://192.168.4.1/servo_control?angle=30')
time.sleep(3)

r = requests.get('http://192.168.4.1/read_angle')
print(r.text)

time.sleep(3)

requests.post('http://192.168.4.1/servo_control?angle=90')
requests.post('http://192.168.4.1/motor_control?speed=0')
time.sleep(1)