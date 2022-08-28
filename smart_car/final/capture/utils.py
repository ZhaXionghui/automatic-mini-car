from urllib.request import urlopen
import cv2
import numpy as np

def read_ip_camera(stream):

    global url
    global img
    global bts
    global is_exit
    global CAMERA_BUFFRER_SIZE

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