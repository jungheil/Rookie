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

lib = SubInit()
while True:
    lib.PTar_Public(0)
    ret, src = GetImg(lib)
    if ret:
        cv2.imshow("PView", src)
    cv2.waitKey(1)
