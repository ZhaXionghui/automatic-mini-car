import cv2
import matplotlib.pyplot as plt

mask = cv2.imread('shapan_460_coco/Masks/train_25.png', cv2.COLOR_BGR2GRAY)
plt.imshow(mask, 'gray')
plt.show()
