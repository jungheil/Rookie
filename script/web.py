#!/usr/bin/env python3


import ctypes
import cv2
import numpy as np
from flask import Flask, request, render_template, make_response, Response, redirect
# from gevent import pywsgi

def SubInit():
    ll = ctypes.cdll.LoadLibrary
    lib = ll(r"../lib/libpyshare.so")
    lib.PImg_Subscribe.restype = ctypes.c_bool
    lib.PImg_GetWidth.restype = ctypes.c_int
    lib.PImg_GetHeight.restype = ctypes.c_int
    lib.PImg_GetData.restype = ctypes.POINTER(ctypes.c_uint8)

    lib.PRun_Public.restype = ctypes.c_bool
    lib.PTar_Public.restype = ctypes.c_bool
    lib.PTar_Subscribe.restype = ctypes.c_int

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
    #self.video = cv2.VideoCapture(0)
    #def __del__(self)
    # self.video.release()

    def get_frame(self, lib):
        ret, image = GetImg(lib)
        ret, jpeg = cv2.imencode('.jpg', image)
        return jpeg.tobytes()



lib = SubInit()
IS_RUN = True
TRACK_ID = -1

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def index():
    global IS_RUN
    return render_template('index.html' ,\
            bt_status=("Stop" if IS_RUN else "Run"))

def gen(camera):
    while True:
        frame = camera.get_frame(lib)
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n\r\n')



@app.route('/video_feed')
def video_feed():
    return Response(gen(VideoCamera()),
                    mimetype='multipart/x-mixed-replace; boundary=frame')
                    
@app.route('/status')
def status():
    global TRACK_ID
    TRACK_ID = lib.PTar_Subscribe()
    return render_template('status.html' ,\
                track_id=TRACK_ID)

@app.route('/p_run_status', methods=['POST'])
def p_run_status():
    global IS_RUN
    IS_RUN= not IS_RUN
    print(IS_RUN)
    lib.PRun_Public(IS_RUN)
    return redirect('/')
    
@app.route('/p_track_id', methods=['POST'])
def p_track_id():
    global TRACK_ID
    text = request.form['index']
    processed_text = text.upper()
    TRACK_ID=int(processed_text)
    lib.PTar_Public(TRACK_ID)
    return redirect('/')


if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, port=80, threaded = True)
    # server = pywsgi.WSGIServer(('0.0.0.0', 80), app)
    # server.serve_forever()


    #cv2.waitKey(1)
# lib = SubInit()
# while True:
#     lib.PTar_Public(0)
#     ret, src = GetImg(lib)
#     if ret:
#         cv2.imshow("PView", src)
#     cv2.waitKey(1)
