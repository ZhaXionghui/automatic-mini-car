import cv2
import os
import numpy as np
import matplotlib.pyplot as plt

dataset_dir = './photos/'                 #图片数据的相对路径
image_dir = 'photos_original/'            #原图片的相对路径
rgb_mask_dir = 'photos_labeled/'          #标签图片所在目录
gray_mask_dir = 'masks/'                  #mask图片文件生成目录

mask_list = os.listdir(dataset_dir + rgb_mask_dir)
for i in range(len(mask_list)):

    rgb_mask = cv2.imread(dataset_dir + rgb_mask_dir + mask_list[i])
    grass_layer = rgb_mask[:, :, 2] / 128         #本次实验数据的第二个新建的标签为grass，故rgb_mask[:, :, 2]中填写2
    background_layer = rgb_mask[:, :, 1] / 128    #第一个新建的标签为background，故rgb_mask[:, :, 1]填写1
    gray_mask = grass_layer + background_layer * 2


    cv2.imwrite(dataset_dir + gray_mask_dir + mask_list[i], gray_mask)