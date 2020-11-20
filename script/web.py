#!/usr/bin/env python3


import ctypes
import cv2
import numpy as np
from flask import Flask, request, render_template, make_response, Response
from gevent import pywsgi

def SubInit():
    ll = ctypes.cdll.LoadLibrary
    lib = ll(r"../lib/libpyshare.so")
    lib.PImg_Subscribe.restype = ctypes.c_bool
    lib.PImg_GetWidth.restype = ctypes.c_int
    lib.PImg_GetHeight.restype = ctypes.c_int
    lib.PImg_GetData.restype = ctypes.POINTER(ctypes.c_uint8)
    return lib

def GetImg(lib):
    if lib.PImg_Subscribe():
        pointer = lib.PImg_GetData()
        w = lib.PImg_GetWidth()
        h = lib.PImg_GetHeight()
        src = np.array(np.fromiter(pointer, dtype=np.uint8, count=h*w*3))
        src.shape = h,w,3
        return True, src
    else:
        return False



class VideoCamera:
   # def __init__(self):
    # 通过opencv获取实时视频流
    # url来源见我上一篇博客
    #self.video = cv2.VideoCapture(0)
    #def __del__(self)
    # self.video.release()

    def get_frame(self, lib):
        ret, image = GetImg(lib)
        # 因为opencv读取的图片并非jpeg格式，因此要用motion JPEG模式需要先将图片转码成jpg格式图片
        ret, jpeg = cv2.imencode('.jpg', image)
        return jpeg.tobytes()



lib = SubInit()

app = Flask(__name__)

@app.route('/')  # 主页
def index():
    # jinja2模板，具体格式保存在index.html文件中
    return render_template('index.html')

def gen(camera):
    while True:
        frame = camera.get_frame(lib)
        # 使用generator函数输出视频流， 每次请求输出的content类型是image/jpeg
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n\r\n')



@app.route('/video_feed')  # 这个地址返回视频流响应
def video_feed():
    return Response(gen(VideoCamera()),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/', methods=['POST'])
def my_form_post():
    text = request.form['index']
    processed_text = text.upper()
    lib.PTar_Public(int(processed_text))
    response = make_response(render_template('index.html'))
    return response
    #return Response(gen(VideoCamera()),
    #                mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    # app.run(host='0.0.0.0', debug=True, port=80)
    server = pywsgi.WSGIServer(('0.0.0.0', 80), app)
    server.serve_forever()


    #cv2.waitKey(1)
# lib = SubInit()
# while True:
#     lib.PTar_Public(0)
#     ret, src = GetImg(lib)
#     if ret:
#         cv2.imshow("PView", src)
#     cv2.waitKey(1)
