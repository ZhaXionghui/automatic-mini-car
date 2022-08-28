import numpy as np
import matplotlib.pyplot as plt
import time
from skimage import morphology
import cv2
import os
import random
from scipy import ndimage

'''
A*算法
'''
class Node():
    """A node class for A* Pathfinding"""

    def __init__(self, parent=None, position=None):
        self.parent = parent
        self.position = position

        self.g = 0
        self.h = 0
        self.f = 0

    def __eq__(self, other):
        return self.position == other.position

def astar(maze, start, end, time_limit=1):
    """
    Returns a list of tuples as a path from the given start to the given end in the given maze.
    if end point is not drivable: return None, 0
    if exceed time limit: return None, 1
    if no valid path: return None, 2
    """

    astar_start_time = time.time()

    if maze[end[0]][end[1]] != 0:
        return (None, 0)

    # Create start and end node
    start_node = Node(None, start)
    start_node.g = start_node.h = start_node.f = 0
    end_node = Node(None, end)
    end_node.g = end_node.h = end_node.f = 0

    # Initialize both open and closed list
    open_list = []
    closed_list = []

    # Add the start node
    open_list.append(start_node)

    # Loop until you find the end
    while len(open_list) > 0:

        if time.time() - astar_start_time >= time_limit:
            return (None, 1)

        # Get the current node
        current_node = open_list[0]
        current_index = 0
        for index, item in enumerate(open_list):
            if item.f < current_node.f:
                current_node = item
                current_index = index

        # Pop current off open list, add to closed list
        open_list.pop(current_index)
        closed_list.append(current_node)

        # Found the goal
        if current_node == end_node:
            path = []
            current = current_node
            while current is not None:
                path.append(current.position)
                current = current.parent
            path = path[::-1] # Return reversed path
            if len(path) == 0:
                return (None, 2)
            else:
                return (path, -1)

        # Generate children
        children = []
        for new_position in [(0, -1), (0, 1), (-1, 0), (1, 0), (-1, -1), (-1, 1), (1, -1), (1, 1)]: # Adjacent squares

            # Get node position
            # node_position = (current_node.position[0] + new_position[0], current_node.position[1] + new_position[1])
            node_position = [current_node.position[0] + new_position[0], current_node.position[1] + new_position[1]]

            # Make sure within range
            if node_position[0] > (len(maze) - 1) or node_position[0] < 0 or node_position[1] > (len(maze[len(maze)-1]) -1) or node_position[1] < 0:
                continue

            # Make sure walkable terrain
            if maze[node_position[0]][node_position[1]] != 0:
                continue

            # Create new node
            new_node = Node(current_node, node_position)

            # Append
            children.append(new_node)

        # Loop through children
        for child in children:

            # Child is on the closed list
            for closed_child in closed_list:
                if child == closed_child:
                    continue

            # Create the f, g, and h values
            child.g = current_node.g + 1
            child.h = ((child.position[0] - end_node.position[0]) ** 2) + ((child.position[1] - end_node.position[1]) ** 2)
            child.f = child.g + child.h

            # Child is already in the open list
            for open_node in open_list:
                if child == open_node and child.g > open_node.g:
                    continue

            # Add the child to the open list
            open_list.append(child)

'''
根据道路分割结果, 规划行驶路线
'''
def find_path(img, time_limit=1, SHOW_IMAGE=False):

    '''
    固定图像大小, 便于像素长度和实际长度换算
    '''
    DIM=(800, 600)
    
    '''
    开始计时
    '''
    start_time = time.time()

    if SHOW_IMAGE:
        plt.imshow(img)
        plt.show()

    '''
    从网络预测大小放缩到拍摄大小
    '''
    img = cv2.resize(img, DIM, interpolation=cv2.INTER_NEAREST)
    img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)

    '''
    矫正图片
    '''
    K = np.loadtxt('K_SVGA.csv', dtype=np.float32, delimiter=',')
    D = np.loadtxt('D_SVGA.csv', dtype=np.float32, delimiter=',')

    nK = K.copy()
    nK[0,0]=K[0,0]
    nK[1,1]=K[1,1]
    map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), nK, DIM, cv2.CV_16SC2)

    # print('map:', map1, map2)

    def undistort(img):
        img += 1
        undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_NEAREST, borderMode=cv2.BORDER_CONSTANT)

        return undistorted_img

    undistorted_img = undistort(img)
    undistorted_img = undistorted_img[:, :, 0]

    if SHOW_IMAGE:
        plt.imshow(undistorted_img)
        plt.show()

    '''
    鸟瞰图
    '''
    # 1.读取透视变换矩阵
    H = np.loadtxt('H_SVGA.csv', dtype=np.float32, delimiter=',')

    # 2.执行透视变换
    birdview_img = cv2.warpPerspective(undistorted_img, H, (DIM[1], DIM[0]), flags=cv2.INTER_NEAREST)
    birdview_img = birdview_img.swapaxes(1, 0)

    if SHOW_IMAGE:
        plt.imshow(birdview_img)
        plt.show()

    '''
    获取可行驶区域图
    实际尺寸与鸟瞰图像素换算比例为 2mm = 1px
    车身半宽为 7.5 cm, 即 37.5 px
    '''
    # 设置形态学操作的核
    kernel_1 = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3)) #定义结构元素的形状和大小
    kernel_2 = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (39, 39))
    kernel_3 = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (37, 37))

    # 获得 grass 标签
    # grass = np.zeros(birdview_img.shape, dtype=np.uint8)
    # grass[birdview_img == 2] = 2
    grass = birdview_img == 2
    # 去除 grass 中误识别出的小区域
    if SHOW_IMAGE:
        plt.subplot(1, 2, 1)
        plt.imshow(grass)
    grass = morphology.remove_small_objects(grass, min_size=60, connectivity=1)
    grass = grass*np.uint8(2)
    # print('back datatype:', grass.dtype)
    if SHOW_IMAGE:
        plt.subplot(1, 2, 2)
        plt.imshow(grass)
        plt.show()

    # 获得背景标签
    back = birdview_img == 3
    # 去除背景中误识别出的小区域
    if SHOW_IMAGE:
        plt.subplot(1, 2, 1)
        plt.imshow(back)
    back = morphology.remove_small_objects(back, min_size=4000, connectivity=1)
    back = back*np.uint8(3)
    # print('back datatype:', back.dtype)
    if SHOW_IMAGE:
        plt.subplot(1, 2, 2)
        plt.imshow(back)
        plt.show()

    # 可视化
    if SHOW_IMAGE:
        plt.subplot(1, 3, 1)
        plt.imshow(birdview_img, 'gray')

    # 初始化 drivable_img
    drivable_img = np.zeros(birdview_img.shape, dtype=np.uint8)
    drivable_img[birdview_img == 1] = 1

    # 先扩展 grass 部分
    dst = cv2.erode(grass, kernel_1)
    dst = cv2.dilate(dst, kernel_2)
    drivable_img = np.maximum(drivable_img, dst)

    if SHOW_IMAGE:
        plt.subplot(1, 3, 2)
        plt.imshow(drivable_img, 'gray')

    # 再扩展 back 部分
    dst = cv2.dilate(back, kernel_3)
    # dst = cv2.erode(back, kernel_1)
    # dst = cv2.dilate(dst, kernel_2)
    drivable_img = np.maximum(drivable_img, dst)

    if SHOW_IMAGE:
        plt.subplot(1, 3, 3)
        plt.imshow(drivable_img, 'gray')
        plt.show()

    (h, w) = drivable_img.shape
    maze = np.zeros((h, w), dtype=np.uint8)
    maze[drivable_img==1] = 1

    # print(cv2.resize(maze, (20, 20), interpolation=cv2.INTER_NEAREST))

    k = 20

    (maze_h, maze_w) = (h//k, w//k)
    print('maze_size:', (maze_w, maze_h))
    maze = cv2.resize(maze, (maze_w, maze_h), interpolation=cv2.INTER_NEAREST)
    
    '''
    补全车前的盲区
    '''
    maze[-60//k:, 320//k+1:480//k] = 1
    if SHOW_IMAGE:
        plt.imshow(maze)
        plt.show()

    (h, w) = maze.shape
    print('maze shape:', h, w)

    '''
    去掉与起点不连接的“可行驶区域”
    '''
    labeled_maze, _ = ndimage.label(maze)
    if SHOW_IMAGE:
        plt.imshow(labeled_maze)
        plt.show()

    drivable_label = labeled_maze[-1, w//2]
    print('drivable_label:', drivable_label)
    maze = 1 - (labeled_maze == drivable_label)
    if SHOW_IMAGE:
        plt.imshow(maze)
        plt.show()

    '''
    选取终点
    设定候选目标半径为 80 cm, 即 400 px, 400//k
    '''
    candidate_circle = np.ones((h, w), dtype=np.uint8)
    cv2.circle(candidate_circle, center=(w//2, h-1), radius=400//k, color=0, thickness=1)
    candidate_circle = candidate_circle*2
    if SHOW_IMAGE:
        plt.imshow(candidate_circle)
        plt.show()

    final_candidate = (maze == candidate_circle).astype(np.uint8)
    if SHOW_IMAGE:
        plt.imshow(final_candidate)
        plt.show()

    _, contours = cv2.findContours(final_candidate, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    num_contour = len(contours)
    print('num of contours:', num_contour)
    print('contours:', contours)

    index = random.randint(0, num_contour-1)
    contour = contours[index]

    print('contour:', contour)

    num = contour.shape[0]
    contour = np.reshape(contour, (num, 2))

    index = random.randint(0, num-1)
    end = [contour[index, 1], contour[index,0]]

    if SHOW_IMAGE:
        plt.imshow(maze, 'gray')
        plt.scatter(end[1], end[0], s=10, c='red')
        plt.show()

    '''
    路径规划
    目标
    '''
    # start = (h-1, w//2)
    start = [h-1, w//2]

    maze = maze.tolist()
    (path, flag) = astar(maze, start, end, time_limit=time_limit)

    if path is None:
        if flag == 0:
            print('end point is not drivable!')
        elif flag == 1:
            print('exceed time limit!')
        else:
            print('no valid path!')

    if SHOW_IMAGE:
        from matplotlib.patches import Circle
        fig = plt.figure()
        axes = fig.subplots(1, 2)

        ax = axes[0]
        ax.imshow(maze)

        ax = axes[1]
        ax.imshow(maze)
        if path is not None:
            for i in range(0, len(path), 1):
                plt.scatter(path[i][1], path[i][0], s=5)
        circle = Circle(xy = [w//2, h-1], radius=400 // k)
        ax.add_patch(p=circle)
        circle.set(lw=3, facecolor='green', alpha=0.3)

        plt.show()
        
    return path

'''
路径平滑
'''

from copy import deepcopy

def printpaths(path, newpath):
    for old, new in zip(path, newpath):
        print('[' + ', '.join('%.3f' % x for x in old) +
              '] -> [' + ', '.join('%.3f' % x for x in new) + ']')


def smooth(path, weight_data=0.5, weight_smooth=0.1, tolerance=0.000001):
    """
    Creates a smooth path for a n-dimensional series of coordinates.
    Arguments:
        path: List containing coordinates of a path
        weight_data: Float, how much weight to update the data (alpha)
        weight_smooth: Float, how much weight to smooth the coordinates (beta).
        tolerance: Float, how much change per iteration is necessary to keep iterating.
    Output:
        new: List containing smoothed coordinates.
    """

    new = deepcopy(path)
    dims = len(path[0])
    change = tolerance

    while change >= tolerance:
        change = 0.0
        for i in range(1, len(new) - 1):
            for j in range(dims):

                x_i = path[i][j]
                y_i, y_prev, y_next = new[i][j], new[i - 1][j], new[i + 1][j]

                y_i_saved = y_i
                y_i += weight_data * (x_i - y_i) + weight_smooth * (y_next + y_prev - (2 * y_i))
                new[i][j] = y_i

                change += abs(y_i - y_i_saved)
    return new