from glob import glob
from tqdm import tqdm
from fire import Fire
from multiprocessing import Pool
import numpy as np
import pickle
import cv2
import os
import sys
import copy
import functools
import time
import xml.etree.ElementTree as ET
sys.path.append(os.getcwd())

import json

import lmdb
import hashlib

"""
最标准的格式：
[    
    [# 注释 vedio_i  
        [
            vedio_i_inpath ,
            vedio_i_outpath
        ],
        [
            [ imgname1, imgname2,imgname3,...     ],
            [  gt1,gt2,gt3, ...     ]   #bbox
        ]
    ],
    ...
    ...
]
eg:
[  
    [         
        [
            '/media/hfc/HFCU/DATA_ALL/TRAIN/GOT10K/train/dir_0001',
            '/media/hfc/HFCU/DATA_ALL/TRAIN/DB/GOT10K/dir_0001'
        ],
        [
            [ 'img/00001.jpg','img/00002.jpg',...          ],
            [  [125,225,478,744], [128,227,475,743] ,...       ]
        ]  
    ]   
    ,...    
]
"""
#80:1 e9




#train
inpath  = '/opt/data/private/data/GOT-10k/got10kbenchmark'
outpath = '/opt/data/private/data/DB/got10k' 
#eval
# inpath  = '/media/secret/data/hfc/DATA/TRAIN/GOT-10k/val'
# outpath = '/media/secret/data/hfc/DATA/TRAIN/DB/GOT10Kval' 
map_size=int(60e9)


def globdir(inPath):
    #1.提取信息
    a_videos = glob(inPath+'/*')
    a_videos.sort()

    # a_videos_1 =[ glob(x+'/*')  for x in a_videos   ]   #combine all video
    a_videos_1 = a_videos
    #2。samework
    a_videos_2 = []
    for x in a_videos_1:
        # x.sort()
        # for y in x:
        a_videos_2.append([[x]])
    return a_videos_2

#It's exactly the same as the main function
def mymain(inPath,outPath):

    if not os.path.exists(outPath):
        os.makedirs(outPath)

    allinf = []
    a_videos_2 = globdir(inpath)
    allinf.extend(a_videos_2)

    singelVideoNode = ['','groundtruth.txt']
    for allinf_i in range(len(allinf)):
        lastDir = allinf[allinf_i][0][0]
        path_i = lastDir #+'/' +singelVideoNode[0]
        gt_i = lastDir+'/'+singelVideoNode[1]

        imgpath = os.path.join(path_i,'*.jpg')
        imgpath_1 = glob(imgpath)
        imgpath_1 = [ x.replace(lastDir+'/','') for x in imgpath_1]
        imgpath_1.sort()

        with open(gt_i, "r") as f:
            gt_i_1 = f.readlines()
            gt_i_2 = [ x.replace('\n','').split(',') for x in gt_i_1   ]
            len_gt_i_2 = len(gt_i_2)
            for gt_i_2_j in range(len_gt_i_2):
                gt_i_2_jend = len_gt_i_2 -gt_i_2_j-1
                if(len(gt_i_2[gt_i_2_jend])<=1):
                    gt_i_2.pop(gt_i_2_jend)
            gt_i_3 = np.array(np.array(gt_i_2,dtype=float),dtype=int).tolist()
            gt_i_4 = [ [x[0],x[1],x[0]+x[2],x[1]+x[3]]  for x in gt_i_3  ]

        # check
        if(len(imgpath_1)!= len(gt_i_4)):
            print(lastDir,' is error')

        allinf[allinf_i].append([imgpath_1,gt_i_4])
        # break
    
    #3转换信息  
    # allinf1 = copy.deepcopy(allinf) 
    allinf1 =[]
    with Pool(processes=12) as pool:
        for x in tqdm(pool.imap_unordered(checkworker, allinf), total=len(allinf)): 
            allinf1.append(x)
    print('check done!')



    for  i in range(len(allinf1)):
        videoname = allinf1[i][0][0]
        videoname1 = videoname.replace(inPath,outPath)
        allinf1[i][0].append(videoname1) 
        # if not os.path.exists(videoname1):
        #     os.makedirs(videoname1)
        videoname2 = videoname1 +'/'+ singelVideoNode[0]
        if not os.path.exists(videoname2):
            os.makedirs(videoname2)

    json.dump(allinf1, open(outpath+'/self.json', 'w'), indent=4, sort_keys=True)

    # allinf1 = json.load(open(outPath+'/self.json', 'r'))
    with Pool(processes=12) as pool:
        for x in tqdm(pool.imap_unordered(worker, allinf1), total=len(allinf1)): 
            pass
    print('worker done!')


    #4.lmdb

    # data = json.load(open(jsonpaths, 'r'))
    # infos = json.load(open(outpath+'/self_old.json', 'r'))
    
    infosmin = copy.deepcopy(allinf1) 
    for i in range(len(infosmin)):
        infosmin[i][0][0]=''
        # infosmin[i][0][1] = infosmin[i][0][1].replace('GOT10Kval','got10kval')  
    json.dump(infosmin, open(outpath+'/self.json', 'w'), indent=4, sort_keys=True)


    lighting = lmdb.open(outpath+'/self.lmdb', map_size=map_size)
    with Pool(processes=12) as pool:
        for x in tqdm(pool.imap_unordered(lmdbworker, infosmin), total=len(infosmin)): 
            with lighting.begin(write=True) as txn:
                for k, v in x.items():
                    txn.put(k, v)
    print('lmdb done!')




def lmdbworker(allinf_i):
    pathdata, name= allinf_i[0][1],allinf_i[1][0]
    lastname = pathdata.split('DB/')[-1]
    # lastname = pathdata.replace('/media/secret/data/hfc/DATA/TRAIN/DB/','')

    kv = {}
    for i in range(len(name)):
        pathr = pathdata +'/'+ name[i]
        k = lastname +'/' + name[i]

        img = cv2.imread(pathr)
        _, img_encode = cv2.imencode('.jpg', img)
        v = img_encode.tobytes()
        kv[hashlib.md5(k.encode()).digest()] = v

    return kv


def checkworker(allinf_i):
    lastdir,imgpath_1 ,gt_i_4 = allinf_i[0][0],allinf_i[1][0],allinf_i[1][1]

    # print('cur  in ', lastdir)
    lenth = len(imgpath_1)
    for  i  in range(lenth):
        ib = lenth - i - 1
        imgpathr = lastdir + '/' + imgpath_1[ib]
        img = cv2.imread(imgpathr)
        if img is None:
            imgpath_1.pop(ib)
            gt_i_4.pop(ib)
            print( lastdir , 'has empty img'  )
            continue
        gt = gt_i_4[ib]
        h,w,_ = img.shape
        cdt1 ,cdt2 ,cdt3 ,cdt4 = (gt[2]<=w) , (gt[3]<=h) ,((gt[2]-gt[0])>5) , ((gt[3]-gt[1])>5) 
        if (cdt1 and cdt2 and cdt3 and cdt4) == False :
            imgpath_1.pop(ib)
            gt_i_4.pop(ib)
            continue
        time.sleep(0.00001)
    if( len(gt_i_4) == 0):
        print( lastdir , 'has empty gt'  )
    if(len(imgpath_1)!= len(gt_i_4)):
            print(lastdir,' is error')
    back = [[lastdir,],[imgpath_1,gt_i_4] ]
    return back





def worker(allinf_i):
    pathdata, data= allinf_i[0],allinf_i[1]
    name ,gt= data[0],data[1]
    # print(pathdata[1])
    for i in range(len(name)):
        pathr,pathw = pathdata[0]+'/'+name[i],pathdata[1]+'/'+name[i]
        gt_i = gt[i]
        box = (gt_i[0],gt_i[1],gt_i[2],gt_i[3])
        img   = cv2.imread(pathr)

        # point0 = (box[0],box[1])
        # point1 = (box[2],box[3])
        # cv2.rectangle(img,point0,point1,(0,0,255),2)
        # cv2.imshow('2',img)
        # cv2.waitKey(1)

        imgCrop= crop_like_SiamFC(img, box)
        # print('finished')
        cv2.imwrite(pathw,imgCrop)
        

    return 'shit,bro'



def crop_hwc(image, bbox, out_sz, padding=(0, 0, 0)):
    a = (out_sz-1) / (bbox[2]-bbox[0])
    b = (out_sz-1) / (bbox[3]-bbox[1])
    c = -a * bbox[0]
    d = -b * bbox[1]
    mapping = np.array([[a, 0, c],
                        [0, b, d]]).astype(np.float)
    crop = cv2.warpAffine(image, mapping, (out_sz, out_sz), borderMode=cv2.BORDER_CONSTANT, borderValue=padding)
    return crop


def pos_s_2_bbox(pos, s):
    return [pos[0]-s/2, pos[1]-s/2, pos[0]+s/2, pos[1]+s/2]


def crop_like_SiamFC(image, bbox, context_amount=0.5, exemplar_size=127, instanc_size=511):
    avg_chans = np.mean(image, axis=(0, 1))
    target_pos = [(bbox[2]+bbox[0])/2., (bbox[3]+bbox[1])/2.]
    target_size = [bbox[2]-bbox[0], bbox[3]-bbox[1]]
    wc_z = target_size[1] + context_amount * sum(target_size)
    hc_z = target_size[0] + context_amount * sum(target_size)
    s_z = np.sqrt(wc_z * hc_z)
    scale_z = exemplar_size / s_z
    d_search = (instanc_size - exemplar_size) / 2
    pad = d_search / scale_z
    s_x = s_z + 2 * pad

    # z = crop_hwc(image, pos_s_2_bbox(target_pos, s_z), exemplar_size, avg_chans)
    x = crop_hwc(image, pos_s_2_bbox(target_pos, s_x), instanc_size, avg_chans)
    return x




# mymain(inpath,outpath)









outpath = '/opt/data/private/data/DB/got10k' 
#step1

# infos = json.load(open(outpath+'/self_old.json', 'r'))
# infosmin = copy.deepcopy(infos) 
# for i in range(len(infos)):
#     infosmin[i][0][0]=''
#     infosmin[i][0][1] = infosmin[i][0][1].replace('GOT10Kval','got10kval')  

# json.dump(infosmin, open(outpath+'/self.json', 'w'), indent=4, sort_keys=True)

#step2
infos = json.load(open(outpath+'/self.json', 'r'))
lighting = lmdb.open(outpath+'/self.lmdb', map_size=map_size)
with Pool(processes=12) as pool:
    for x in tqdm(pool.imap_unordered(lmdbworker, infos), total=len(infos)): 
        with lighting.begin(write=True) as txn:
            for k, v in x.items():
                txn.put(k, v)
print('lmdb done!')
















