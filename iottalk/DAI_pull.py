import time, DAN, requests, random
import json
import numpy as np
import cv2
import base64
from ast import literal_eval
import time

#from requests.utils import requote_uri


#ServerURL = 'http://IP:9999' #with no secure connection
#ServerURL = 'https://DomainName' #with SSL connection
ServerURL = 'http://140.113.199.181:9999'
Reg_addr = None #if None, Reg_addr = MAC address

DAN.profile['dm_name']='Transmit_Frame'
DAN.profile['df_list']=['IDF_Frame', 'ODF_Frame']
DAN.profile['d_name']= None # None for autoNaming
DAN.device_registration_with_retry(ServerURL, Reg_addr)

#cap = cv2.VideoCapture('time_counter.flv')

def receive_frame_from_iottalk():

    #print('start receive frame from iottalk')
    try:
        data = DAN.pull('ODF_Frame')
        if data != None:
            frame_string = data[0]
            person_information = data[1]
            #print(person_information)

            tmp_array = literal_eval(frame_string)
            tmp_boxes = json.loads(person_information)
            #tmp_nparray = np.array(tmp_array)
            #tmp_buf = tmp_nparray.astype('uint8')
            #frame = cv2.imdecode(tmp_buf, 1)
            #cv2.imshow('Receive',frame)
            #cv2.waitKey(1)

            return (tmp_array, tmp_boxes)

    except Exception as e:
        print(e)
        if str(e).find('mac_addr not found:') != -1:
            print('Reg_addr is not found. Try to re-register...')
            DAN.device_registration_with_retry(ServerURL, Reg_addr)
        else:
            print('Connection failed due to unknow reasons.')
            #time.sleep(1)    

    #time.sleep(0.2)
    return None
    