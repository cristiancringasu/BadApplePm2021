import cv2
import numpy as np
import sys
if len(sys.argv) < 3:
    print ("Call with: 'python led_convertor <file_name.mp4> <out_file_name.vid>'")
    exit()
cap = cv2.VideoCapture(sys.argv[1])
frameCount = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
frameWidth = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = int(cap.get(cv2.CAP_PROP_FPS))
#buf = np.empty((frameCount, frameHeight, frameWidth, 3), np.dtype('uint8'))
buf = np.empty((frameCount, 16, 16, 3), np.dtype('uint8'))

fc = 0
ret = True

while (fc < frameCount  and ret):
    #ret, buf[fc] = cap.read()
    ret, frame = cap.read()
    (w, h, c) = frame.shape

    #syntax: cv2.resize(img, (width, height))
    img = cv2.resize(frame,(16, 16))
    buf[fc] = img
    fc += 1

cap.release()

# cv2.namedWindow('frame 100')
# cv2.imshow('frame 10', buf[9])

# cv2.waitKey(0)
white = [255, 255, 255]
black = [0, 0, 0]
with open(sys.argv[2],'wb') as f:
    metadata = str(fc) + " " + str(fps) + "\n"
    f.write(bytes(metadata, 'utf-8'))
    for k in range(fc):
        for i in range(16):
            bits = ""
            for j in range(16):
                if np.linalg.norm(buf[k][i][j] - white) < np.linalg.norm(buf[k][i][j] - black):
                    bits += "1"
                else:
                    bits += "0"
            f.write(int(bits[::-1], 2).to_bytes(2, 'little'))