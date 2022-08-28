import cv2
import numpy as np
from urllib.request import urlopen
import threading
import time

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


if __name__ == '__main__':

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

    for i in range(3):
        print('second {}...'.format(i))
        time.sleep(1)
    
    is_exit = True
    t1.join()