import requests
import time

# requests.post('http://192.168.4.1/servo_control?angle=150')
requests.post('http://192.168.4.1/motor_control?speed=60')


# requests.post('http://192.168.4.1/servo_control?angle=90')
# requests.post('http://192.168.4.1/motor_control?speed=0')
# time.sleep(1)