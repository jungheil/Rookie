#!/usr/bin/env python3

import sys
import ctypes
import cv2
import numpy as np

# class ImageStr(ctypes.Structure):
#     _fields_ = [("width_", ctypes.c_int), ("height_", ctypes.c_int), ("data_", ctypes.c_void_p)]

def SubInit():
    ll = ctypes.cdll.LoadLibrary
    lib = ll(r"../lib/libpyshare.so")
    lib.PSubscribe.restype = ctypes.c_bool
    lib.PGetWidth.restype = ctypes.c_int
    lib.PGetHeight.restype = ctypes.c_int
    lib.PGetData.restype = ctypes.POINTER(ctypes.c_uint8)
    return lib

def GetImg(lib):
    if lib.PSubscribe():
        pointer = lib.PGetData()
        w = lib.PGetWidth()
        h = lib.PGetHeight()
        src = np.array(np.fromiter(pointer, dtype=np.uint8, count=h*w*3)) 
        src.shape = h,w,3
        return True, src
    else:
        return False

lib = SubInit()
while True:
    ret, src = GetImg(lib)
    if ret:
        cv2.imshow("PView", src)
    cv2.waitKey(1)
