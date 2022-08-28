import numpy as np
import math
from datetime import datetime
import time
import requests
   

'''
路线跟踪常量
'''
Lf = 200.0  # look-ahead distance
dt = 1.0  # [s]
d = 144.0  # [mm] wheel base of vehicle
delta_candicate = np.array([-28, -16.2, 0.0, 16.2, 28])/180*np.pi
delta_cmd = np.array([90, 60, 0, -60, -90], np.int16) + 90

'''
车辆控制类
'''
class car_controllor():

    def __init__(self):
        self.v = 0.0

    def motor_control(self, speed):
    # 这里的 speed 是电机的占空比，不是实际速度
        requests.post('http://192.168.4.1/motor_control?speed={}'.format(speed))

    def servo_control(self, angle):
    # 这里的 angle 是舵机角度，不是车的转角
        requests.post('http://192.168.4.1/servo_control?angle={}'.format(angle))

    def read_yaw(self):
    # 读取航向角，单位度
        r = requests.get('http://192.168.4.1/read_angle')
        yaw = np.float16(r.text)

        return yaw

    def read_speed(self):
    # 读取实测速度（单位为cm/s)，返回 mm/s 单位的速度
        r = requests.get('http://192.168.4.1/read_speed')
        speed = np.float16(r.text)

        return speed*10

'''
车辆状态类
'''
class VehicleState:# 定义一个类，用于调用车辆状态信息

    def __init__(self, x=0.0, y=0.0, delta=0.0, yaw=0.0, v=0.0):
        self.x = x
        self.y = y
        self.delta = delta
        self.yaw = yaw
        self.v = v

'''
根据前视距离计算下一个目标点
'''
def calc_target_index(state, path):
    '''
    path: numpy array
    '''
    x = path[:, 0]
    y = path[:, 1]
    d = ((x - state.x)**2 + (y - state.y)**2)**0.5
    idx = np.where(np.abs(d - d.min()) <= 0.001)
    idx = idx[0][0]

    L = 0.0
    while L < Lf and idx < path.shape[0] - 1:
        dx = x[idx + 1] - state.x
        dy = y[idx + 1] - state.y
        L += np.sqrt((dx**2 + dy**2))
        idx += 1
    return idx

'''
选择一个舵机转向角度
'''
def select_delta(expect_delta):

    diff = (delta_candicate - expect_delta)**2
    idx = np.where(diff == diff.min())[0][0]

    return (idx, delta_candicate[idx])

'''
纯跟踪控制
'''
def pure_pursuit_control(init_state:VehicleState, path, ctrller:car_controllor, timeout=12):
    '''
    path: numpy array, n*2
    ld: 目标与车辆后轴中心直线距离
    yaw: 全局偏航角
    alpha: 目标航偏角与当前航偏角之差, 即规划圆心角的一半
    theta: 实际航偏角变化, 实际行实际行进圆心角
    delda: 前车轮转角
    d: 车辆前后轴距离
    '''

    start_time = time.time()

    last_index = path.shape[0] - 1
    idx = 0
    state = init_state
    initial_yaw = ctrller.read_yaw() - 90

    i = 0

    while ((state.x - path[last_index, 0])**2 + (state.y - path[last_index, 1])**2)**0.5 > 5:

        if time.time() - start_time >= timeout:
            break

        print('--------- {} ---------'.format(i))
        i += 1

        # 确定下一个目标
        idx = calc_target_index(state, path)

        target_x = path[idx, 0]
        target_y = path[idx, 1]
        print('target x: {}, target y: {}'.format(target_x, target_y))

        # 角度形式的目标方向角
        # target_yaw = math.atan2(target_y - state.y, target_x - state.x) \
        #     * 180 / np.pi
        target_yaw = math.atan2(target_y - state.y, target_x - state.x) \
            * 180 / np.pi
        print('target_yaw: {} degree'.format(target_yaw))

        ld = ((target_x - state.x)**2 + (target_y - state.y)**2)**0.5
        print('ld: {} mm'.format(ld))

        state.yaw = ctrller.read_yaw() - initial_yaw
        print('state.yaw: {} degree'.format(state.yaw))

        # 期望的车辆前轮转角
        expect_delta = np.arctan(\
            2*d*np.sin((target_yaw - state.yaw) * np.pi / 180) / ld\
        )
        print('expect_delta: {} degree'.format(expect_delta / np.pi * 180))

        (delta_idx, state.delta) = select_delta(expect_delta)
        print('delta: {} degree'.format(state.delta / np.pi * 180))
        print('delta_cmd: {} degree'.format(delta_cmd[delta_idx]))
        
        ctrller.servo_control(delta_cmd[delta_idx])
        # state.yaw = ctrller.read_yaw() - initial_yaw
        old_yaw = state.yaw

        time.sleep(dt)

        state.v = ctrller.read_speed()
        state.yaw = ctrller.read_yaw() - initial_yaw
        print('state.v: {} mm/s'.format(state.v))
        print('state.yaw: {} degree'.format(state.yaw))

        theta = old_yaw - state.yaw
        if abs(state.delta) <= 0.001:
            l = state.v * dt
            theta_theory = 0.0
            print('theta:', theta, ' theta_theory:', theta_theory)
            print('l:', l)
        else:
            R = d / np.tan(state.delta)
            theta_theory = np.arcsin(state.v * dt / R) * 180 / np.pi
            print('theta:', theta, ' theta_theory:', theta_theory)
            # l = np.sin(theta) * R
            l_theory = np.abs((theta / 180 * np.pi) * R)
            l = state.v * dt
            print('l:', l, ' l_theory:', l_theory)

        state.x = state.x + l*np.cos(old_yaw / 180 * np.pi)
        state.y = state.y + l*np.sin(old_yaw / 180 * np.pi)
        print('x: {} mm'.format(state.x))
        print('y: {} mm'.format(state.y))

        
    ctrller.motor_control(0)   
    ctrller.servo_control(90)