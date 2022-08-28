import numpy as np
import matplotlib.pyplot as plt
from numpy.core.defchararray import index
import math

'''
路线跟踪常量
'''

Lf = 20.0  # look-ahead distance
dt = 2.0  # [s]
d = 12.0  # [cm] wheel base of vehicle
v = 6.11  # [cm]
delta_candicate = np.array([-27.55, -13.12, 0.0, 13.12, 27.55])/180*np.pi

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


def update(state, delta):

    new_state = state
    return new_state

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

    return delta_candicate[idx]

'''
纯跟踪控制
'''
def pure_pursuit_control(init_state, path):
    '''
    path: numpy array, n*2
    '''
    last_index = path.shape[0] - 1
    t = 0.0
    idx = 0
    state = init_state
    x_history = [state.x]
    y_history = [state.y]
    delta_history = [state.delta]
    yaw_history = [state.yaw]
    t_history = [0.0]

    mngr = plt.get_current_fig_manager()  # 获取当前figure manager
    mngr.window.geometry("%sx%s+%s+%s" % (900, 600, 200, 200))
    while ((state.x - path[last_index, 0])**2 + (state.y - path[last_index, 1])**2)**0.5 > 5:

        idx = calc_target_index(state, path)
        target_x = path[idx, 0]
        target_y = path[idx, 1]
        target_yaw = math.atan2(target_y - state.y, target_x - state.x)

        ld = ((target_x - state.x)**2 + (target_y - state.y)**2)**0.5
        expect_delta = np.arctan(2*d*np.sin(target_yaw - state.yaw)/ld)
        state.delta = select_delta(expect_delta)
        print(state.delta)

        l = state.v * dt

        if abs(state.delta) <= 0.001:
            theta = 0.0
        else:
            R = d / np.tan(state.delta)
            theta = np.arcsin(l / R)

        state.x = state.x + l*np.cos(state.yaw + theta)
        state.y = state.y + l*np.sin(state.yaw + theta)
        state.yaw = theta*2 + state.yaw

        t += dt

        x_history.append(state.x)
        y_history.append(state.y)
        delta_history.append(state.delta)
        yaw_history.append(state.yaw)
        t_history.append(t)

        plt.ion()
        plt.cla()
        plt.scatter(path[:, 0], path[:, 1], s=5)
        plt.scatter(target_x, target_y, c='red', marker='x', s=20)
        plt.scatter(x_history, y_history, s=5)
        plt.axis("equal")
        plt.grid(True)
        plt.title("yaw:" + str(state.yaw/np.pi*180)[:4])
        plt.pause(0.05)

    plt.pause(30)

if __name__ == '__main__':

    path_x = list(range(0, 400, 40))
    path_y = [math.sin(ix/80) * 30.0 for ix in path_x]
    path = np.array([path_x, path_y]).T

    # path = np.array([[52, 47], [51.0261300087, 46.0], [50.095810046777004, 45.0], [49.3251735100102, 44.0], [49.0964928290743, 43.0], [49.02863351752672, 42.0], [49.008496742936586, 41.0], [49.00252120720253, 40.0], [49.00074768377762, 39.0], [49.00022029969101, 38.0], [49.00006008173329, 37.0], [49, 36]])

    print(path.shape)

    # state = VehicleState(x=52, y=47, delta=0.0, yaw=np.pi/4*0, v=v)
    state = VehicleState(x=0, y=0, delta=0.0, yaw=0, v=v)

    pure_pursuit_control(state, path)
