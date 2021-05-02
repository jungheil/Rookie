from utils.datasets import *
import socket
from utils.utils import *
# import argparse
import cv2
import os

# ----------------------------------------------------------------------------------------------------------------------

# parser = argparse.ArgumentParser()
# parser.add_argument('--weights', type=str, default='weights/5s/best.pt', help='path to weights file')
# parser.add_argument('--conf-thres', type=float, default=0.3, help='object confidence threshold')
# parser.add_argument('--nms-thres', type=float, default=0.5, help='iou threshold for non-maximum suppression')
# opt = parser.parse_args()
# # print(opt)

weights = 'weights/5s/best.pt'
conf_thres = 0.3
nms_thres = 0.5


# ----------------------------------------------------------------------------------------------------------------------


class Yolo():
    def __init__(self):
        self.writer = None
        self.prepare()

    def prepare(self):
        global model, device, classes, colors, names
        device = torch_utils.select_device(device='cpu')

        google_utils.attempt_download(weights)
        model = torch.load(weights, map_location=device)['model'].float()

        model.to(device).eval()

        names = model.names if hasattr(model, 'names') else model.modules.names
        colors = [[random.randint(0, 255) for _ in range(3)] for _ in range(len(names))]

    def detect(self, frame):
        temp_height = frame.shape[0]
        temp_width = frame.shape[1]
       
        im0 = imutils.resize(frame, width=416)
        saved_height = im0.shape[0]
        img = letterbox(frame, new_shape=416)[0]

        img = img[:, :, ::-1].transpose(2, 0, 1)
        img = np.ascontiguousarray(img, dtype=np.float32)
        img /= 255.0

        img = torch.from_numpy(img).to(device)
        if img.ndimension() == 3:
            img = img.unsqueeze(0)
        pred = model(img)[0]
        pred = non_max_suppression(pred, conf_thres, nms_thres)

        boxes = []
        boxes2 = []
        boxes_one_dimension = []
        confidences = []
        classIDs = []

        for i, det in enumerate(pred):

            if det is not None and len(det):
                det[:, :4] = scale_coords(img.shape[2:], det[:, :4], im0.shape).round()

                for *xyxy, score, cls in det:
                    label = '%s ' % (names[int(cls)])
                    if names[int(cls)] == 'person':
                        plot_one_box(xyxy, im0, label=label, color=colors[int(cls)])
                        boxes.append([int(xyxy[0]), int(xyxy[1]), int(xyxy[2] - xyxy[0]), int(xyxy[3] - xyxy[1])])
                        boxes2.append( [ int(xyxy[0]) , int(xyxy[1]) , int(xyxy[2] - xyxy[0]) , int(xyxy[3] - xyxy[1]) ] )
                        boxes_one_dimension.append(int(int(xyxy[0]) * temp_width / 416 ))
                        boxes_one_dimension.append(int(int(xyxy[1])  * temp_height / saved_height))
                        boxes_one_dimension.append(int(int(xyxy[2] - xyxy[0])  * temp_width / 416))
                        boxes_one_dimension.append(int(int(xyxy[3] - xyxy[1])  * temp_height / saved_height))
                        # boxes2.append(int(xyxy[1]))
                        # boxes2.append(int(xyxy[2] - xyxy[0]))
                        # boxes2.append(int(xyxy[3] - xyxy[1]))
                        confidences.append(float(score))
                        classIDs.append(int(cls))
                        # print(xyxy[0], xyxy[1], xyxy[2], xyxy[3], float(score), int(cls))
                        print(int(xyxy[0]) , int(xyxy[1]) , int(xyxy[2] - xyxy[0]) , int(xyxy[3] - xyxy[1]) )

            # im0 是图片 boxes是一个一维list 分别是 1.左上角横座标 2.左上角纵座标 3.人框的宽度 4.人框的长度 即每个框有四个元素
            # 注意 如果一张图片有4个人 那么 boxes里面就有4*4=16个元素 也就是说 读取boxes的时候 要4个一组 4个一组 这是原子操作 不会出现不是4的倍数的情况
            return im0,boxes2,boxes_one_dimension

yolo = Yolo()

# ----------------------------------------------------------------------------------------------------------------------


# previous version------------------------------------------------------------------------------------------------------
# yolo = Yolo()
# files = os.listdir('./data/dataset/images/')
# files.sort()
# for file in files:
#     if file.endswith('jpg') or file.endswith('png'):
#         image_path = './data/dataset/images/' + file
#         image = cv2.imread(image_path)
#         image , _ = yolo.detect(image)
#         cv2.imshow('', image)
#         cv2.waitKey(0)
# previous version------------------------------------------------------------------------------------------------------


def arrayreset(array):
    a = array[:, 0:len(array[0] - 2):3]
    b = array[:, 1:len(array[0] - 2):3]
    c = array[:, 2:len(array[0] - 2):3]
    a = a[:, :, None]
    b = b[:, :, None]
    c = c[:, :, None]
    m = np.concatenate((a, b, c), axis=2)
    return m

# new version-----------------------------------------------------------------------------------------------------------
def load_image(single_image):
    global yolo
    single_image = arrayreset(single_image)
    result_image , boxes = yolo.detect(single_image)
    return boxes
# new version-----------------------------------------------------------------------------------------------------------

#!/usr/bin/python

import socket
import sys
import cv2
import numpy as np
from _thread import *
from threading import Thread


class TcpClientTest:
    def __init__(self, image_path):

        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        host = ''
        port = 12336

        print('Waiting for connection')
        try:
            client_socket.connect((host, port))
        except socket.error as e:
            print(str(e))
            return

        image_test = cv2.imread(image_path)
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
        result, imgencode = cv2.imencode('.jpg', image_test, encode_param)
        data = np.array(imgencode)
        stringData = data.tostring()

        while True:
            # cv2.imshow('Client', image_test)
            # cv2.waitKey(1000)
            try:
                client_socket.sendall(str('L83F').encode('utf-8'))
                client_socket.sendall(str(len(stringData)).ljust(16).encode('utf-8'))
                client_socket.sendall(stringData)
                response = client_socket.recv(2048)
                print(response.decode('utf-8'))
            except socket.error as e:
                print(str(e))


class ClientHandlerThread(Thread):
    global yolo
    def __init__(self, connection):
        Thread.__init__(self)
        self.connection = connection
        self.running = True

    def recvall(self,connection, count):
        buf = b''
        while count:
            new_buffer = connection.recv(count)
            if not new_buffer:
                return None
            buf += new_buffer
            print(len(new_buffer))
            count -= len(new_buffer)

        return buf

    def terminate(self):
        self.running = False

    def run(self):
        while self.running:
            try:
                magic_id = self.connection.recv(4)
                if not magic_id:
                    continue

                if magic_id.decode('utf-8') != 'L83F':
                    print("Client sent a strange msg, something wrong...: {}".format(magic_id.decode('utf-8')))
                    continue

                data_header = self.connection.recv(16)
                image_size = int(data_header.decode('utf-8'))

                print("Client sent a image with: {} (bytes)".format(image_size))
                buffer = self.recvall(self.connection, image_size)
                np_encoded_img = np.fromstring(buffer, dtype='uint8')
                decoded_img = cv2.imdecode(np_encoded_img, 1)
                # cv2.imshow('SERVER', decoded_img)

                # -----------------------------------------------------
                im , boxes , send_value  = yolo.detect(decoded_img)
                # print(send_value)
                # cv2.waitKey(100)
                string = ""
                for i in send_value:
                    string = string + str(i) + ","
                string = string + '\0'
                self.connection.sendall(string.encode())
                print(string.encode)
                # -----------------------------------------------------

            except KeyboardInterrupt:  # exit from the client
                break
            except socket.timeout:
                pass

        self.connection.close()
        print("connection with client closed)")

class TcpServer:
    def __init__(self):

        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        host = ''
        port = 12336

        try:
            server_socket.bind((host, port))
        except socket.error as e:
            print(str(e))

        print('Waiting for a Connection.. on {}:{}'.format(host, port))
        server_socket.listen(5)
        server_socket.settimeout(1.0)

        def recvall(connection, count):
            buf = b''
            while count:
                new_buffer = connection.recv(count)
                if not new_buffer:
                    return None
                buf += new_buffer
                print(len(new_buffer))
                count -= len(new_buffer)

            return buf

        current_client_thread = None
        thread_count = 0

        while True:
            try:
                client_conn, address = server_socket.accept()
                if client_conn:
                    if current_client_thread:
                        print('Terminating previous connection - ' + 'Connection Number: ' + str(thread_count))
                        current_client_thread.terminate()
                        current_client_thread.join()
                        print('Terminated previous connection')
                    print('Connected to: ' + address[0] + ':' + str(address[1]))
                    current_client_thread = ClientHandlerThread(client_conn)
                    current_client_thread.start()
                    thread_count += 1
                    print('Connection Number: ' + str(thread_count))
            except socket.timeout:
                pass
            except KeyboardInterrupt: # exit from the server
                current_client_thread.terminate()
                current_client_thread.join(100)
                server_socket.close()
                print("socket closed")
                break
            except:
                raise


if __name__ == '__main__':
    TcpServer()


