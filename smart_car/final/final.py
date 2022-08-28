import cv2
import numpy as np
from urllib.request import urlopen
import threading
import time
import torch
from PIL import Image
import matplotlib.pyplot as plt

from pure_pursuit.utils import car_controllor, VehicleState, pure_pursuit_control
from seg import unet_predict
from seg.unet import UNet
from path_plan.utils import find_path

def read_ip_camera(stream):

    global url
    global img
    global bts
    global is_exit

    count = 0
    while not is_exit:
        try:
            bts += stream.read(CAMERA_BUFFRER_SIZE)
            jpghead=bts.find(b'\xff\xd8')
            jpgend=bts.find(b'\xff\xd9')
            
            if jpghead > -1 and jpgend > -1:
                print(count)
                count += 1
                jpg = bts[jpghead:jpgend + 2]
                bts = bts[jpgend + 2:]
                img = cv2.imdecode(np.frombuffer(jpg, dtype = np.uint8), cv2.IMREAD_UNCHANGED)

        except Exception as e:
            print("Error:" + str(e))
            bts = b''
            stream = urlopen(url)
            continue

'''
主程序
'''
if __name__ == '__main__':

    SHOW_IMAGE = True

    url = 'http://192.168.4.5:80/'
    stream = urlopen(url)
    CAMERA_BUFFRER_SIZE = 4096
    bts = b''
    img = None
    is_exit = False

    # 开始 imu 读取线程
    t1 = threading.Thread(target=read_ip_camera, args=(stream, ))
    t1.setDaemon(True)
    t1.start()

    time.sleep(5)

    net = UNet(n_channels=3, n_classes=3, bilinear=False)
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    net.to(device=device)
    net.load_state_dict(torch.load('seg\checkpoints\checkpoint_epoch1.pth', map_location=device))

    if SHOW_IMAGE:
        plt.imshow(img[:, :, [2,1,0]])
        plt.show()

    ctrller = car_controllor()

    for i in range(10):

        img_Image = Image.fromarray(img)
        mask = unet_predict.predict_img(net=net,
                            full_img=img_Image,
                            scale_factor=0.5,
                            out_threshold=0.5,
                            device=device)
        mask = unet_predict.mask_to_image(mask)

        if SHOW_IMAGE:
            plt.imshow(mask)
            plt.show()

        try:
            path = find_path(mask, time_limit=5, SHOW_IMAGE=SHOW_IMAGE)
        except:
            ctrller.motor_control(-50)
            time.sleep(4)
            ctrller.motor_control(0)
            continue

        path_array = np.array(path)
        path_array = np.vstack((path_array[:, 1], path_array[:, 0])).T
        path_array[:, 1] = 29 - path_array[:, 1]
        path_array = path_array * 40
        print(path_array)

        if SHOW_IMAGE:
            plt.figure()
            plt.scatter(path_array[:, 0], path_array[:, 1], s=20, c='red', alpha=0.6)
            plt.show()


        '''
        路线跟踪常量
        '''
        Lf = 100.0  # look-ahead distance
        dt = 1.0  # [s]
        d = 144.0  # [mm] wheel base of vehicle
        delta_candicate = np.array([-28, -16.2, 0.0, 16.2, 28]) / 180 * np.pi
        delta_cmd = np.array([90, 60, 0, -60, -90], np.int16) + 90

        ctrller.servo_control(90)
        ctrller.motor_control(50)
        state = VehicleState(x=800, y=0, delta=0.0, yaw=0, v=0)

        # 1 px = 40 mm
        pure_pursuit_control(state, path_array, ctrller)

    is_exit = True
    t1.join()