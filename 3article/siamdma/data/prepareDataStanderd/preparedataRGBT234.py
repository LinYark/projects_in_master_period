from glob import glob
from tqdm import tqdm
from fire import Fire
from multiprocessing import Pool
import numpy as np
import pickle
import cv2
import os
import sys
import functools
import xml.etree.ElementTree as ET
sys.path.append(os.getcwd())
import copy
import lmdb
import hashlib
import json
"""
[  
    [         
        [
            '/media/hfc/HFCU/DATA_ALL/TRAIN/GOT10K/train/dir_0001',
            '/media/hfc/HFCU/DATA_ALL/TRAIN/DB/GOT10K/dir_0001'
        ],
        [
            [ 'visible/00001i.jpg','visible/00002i.jpg',...          ],
            [  [125,225,478,744], [128,227,475,743] ,...       ]
        ],
        [
            [ 'infrared/00001v.jpg','infrared/00002v.jpg',...          ],
            [  [125,225,478,744], [128,227,475,743] ,...       ]
        ]
    ]   
    ,...    
]
"""


# dataPath = ''
# output_path  = ''
# '/media/secret/data/hfc/DATA/TEST/GTOT' 
inpath  = '/opt/data/private/data/RGBT234'
outpath = '/opt/data/private/data/DB/rgbt234'  
inpath  = '/opt/data/private/data/RGBT1'
outpath = '/opt/data/private/data/DB/rgbt1'  
inpath  = '/opt/data/private/data/RGBT2'
outpath = '/opt/data/private/data/DB/rgbt2'  
inpath  = '/opt/data/private/data/rgbtleft'
outpath = '/opt/data/private/data/DB/rgbtleft'  
inpath  = '/opt/data/private/data/rgbt60'
outpath = '/opt/data/private/data/DB/rgbt60'  


map_size = int(10e9)

#It's exactly the same as the main function
def mymain(inPath,outPath):

    if not os.path.exists(outPath):
        os.makedirs(outPath)

    vedioStepPath = os.path.join(inPath,'*')

    allinf = []
    #1.提取信息
    a_videos = glob(vedioStepPath)
    a_videos.sort()

    a_videos1 = []
    for x in a_videos:
        if os.path.isdir(x):
            a_videos1.append([[x]])
    allinf.extend(a_videos1)

    singelVideoNode = ['infrared','visible','infrared.txt','visible.txt']
    # singelVideoNode = ['i','v','groundTruth_i.txt','groundTruth_v.txt']
    for a_videos1i in range(len(a_videos1)):

        lastDir = a_videos1[a_videos1i][0][0]
        ipath = lastDir+'/'+singelVideoNode[0]
        vpath = lastDir+'/'+singelVideoNode[1]
        igt = lastDir+'/'+singelVideoNode[2]
        vgt = lastDir+'/'+singelVideoNode[3]

        ipath1 = os.path.join(ipath,'*')
        iimgs = glob(ipath1)
        iimgs = [ x.replace(lastDir+'/','') for x in iimgs  ]
        iimgs.sort()

        vpath1 = os.path.join(vpath,'*')
        vimgs = glob(vpath1)
        vimgs = [ x.replace(lastDir+'/','') for x in vimgs  ]
        vimgs.sort()

        with open(igt, "r") as f:
            igt1 = f.readlines()
            igt2 = [ x.replace('\n','').split(',') for x in igt1   ]
            ilen = len(igt2)
            for igt2i in range(ilen):
                igt2i = ilen -igt2i-1
                if(len(igt2[igt2i])<=1):
                    igt2.pop(igt2i)
            igt3 = np.array(igt2,dtype=int).tolist()
            igt4 = [ [x[0],x[1],x[0]+x[2],x[1]+x[3]]  for x in igt3  ]
            # igt4 = [ [x[0],x[1],x[2],x[3]]  for x in igt3  ]
        with open(vgt, "r") as f:
            vgt1 = f.readlines()
            vgt2 = [ x.replace('\n','').split(',') for x in vgt1   ]
            vlen = len(vgt2)
            for vgt2i in range(vlen):
                vgt2i = vlen -vgt2i-1
                if(len(vgt2[vgt2i])<=1):
                    vgt2.pop(vgt2i)
            vgt3 = np.array(vgt2,dtype=int).tolist()
            # vgt4 = [ [x[0],x[1],x[2],x[3]]  for x in vgt3  ]
            vgt4 = [ [x[0],x[1],x[0]+x[2],x[1]+x[3]]  for x in vgt3  ]
        #check
        if(len(iimgs)!= len(vimgs)):
            print('error')
        if(len(igt4)!= len(vimgs)):
            print('error')
        if(len(vgt4)!= len(vimgs)):
            print('error')
        allinf[a_videos1i].append([vimgs,vgt4])
        allinf[a_videos1i].append([iimgs,igt4])
        # allinf[a_videos1i].append([iimgs,vgt4])

    ##################################################
    #2转换信息  
    allinf1 =copy.deepcopy(allinf) 
    for  i in range(len(allinf1)):
        videoname = allinf1[i][0][0]
        videoname1 = videoname.replace(inPath,outPath)
        allinf1[i][0].append(videoname1) 
        if not os.path.exists(videoname1):
            os.makedirs(videoname1)
        if not os.path.exists(videoname1+'/'+singelVideoNode[0]):
            os.makedirs(videoname1+'/'+singelVideoNode[0])
        if not os.path.exists(videoname1+'/'+singelVideoNode[1]):
            os.makedirs(videoname1+'/'+singelVideoNode[1])


    json.dump(allinf1, open(outPath+'/self.json', 'w'), indent=4, sort_keys=True)

    with Pool(processes=8) as pool:
        for x in tqdm(pool.imap_unordered(worker, allinf1), total=len(allinf1)): 
            pass
    print('done!')


    #4.lmdb

    # data = json.load(open(jsonpaths, 'r'))
    # infos = json.load(open(outpath+'/self_old.json', 'r'))
    
    infosmin = copy.deepcopy(allinf1) 
    for i in range(len(infosmin)):
        infosmin[i][0][0]=''
    json.dump(infosmin, open(outpath+'/self.json', 'w'), indent=4, sort_keys=True)


    # lighting = lmdb.open(outpath+'/self.lmdb', map_size=map_size)
    # with Pool(processes=12) as pool:
    #     for x in tqdm(pool.imap_unordered(lmdbworker, infosmin), total=len(infosmin)): 
    #         with lighting.begin(write=True) as txn:
    #             for k, v in x.items():
    #                 txn.put(k, v)
    # print('lmdb done!')


def lmdbworker(allinf_i):
    pathdata, vname, iname= allinf_i[0][1],allinf_i[1][0],allinf_i[2][0]
    lastname = pathdata.split('DB/')[-1]
    # lastname = pathdata.replace('/media/secret/data/hfc/DATA/TRAIN/DB/','')

    kv = {}
    for i in range(len(vname)):
        pathr = pathdata +'/'+ vname[i]
        vk = lastname +'/' + vname[i]
        img = cv2.imread(pathr)
        _, img_encode = cv2.imencode('.jpg', img)
        vv = img_encode.tobytes()
        kv[hashlib.md5(vk.encode()).digest()] = vv

        pathr = pathdata +'/'+ iname[i]
        ik = lastname +'/' + iname[i]
        img = cv2.imread(pathr)
        _, img_encode = cv2.imencode('.jpg', img)
        iv = img_encode.tobytes()
        kv[hashlib.md5(ik.encode()).digest()] = iv

    return kv




def worker(allinf1_i):
    inlastpath,outlastpath =allinf1_i[0][0],allinf1_i[0][1]
    idata,vdata = allinf1_i[2],allinf1_i[1]
    ipath ,vpath = idata[0],vdata[0]
    igt ,vgt = idata[1],vdata[1]
    for i in range(len(ipath)):

        iimgpr ,vimgpr = inlastpath+'/'+ipath[i] ,inlastpath+'/'+vpath[i]
        iimgpw,vimgpw = outlastpath+'/'+ipath[i] ,outlastpath+'/'+vpath[i]

        ibox ,vbox = igt[i],vgt[i]
        iimg ,vimg  = cv2.imread(iimgpr),cv2.imread(vimgpr)


        # # print('cur path \n',iimgpr,'\n',vimgpr)
        # point0 = (ibox[0],ibox[1])
        # point1 = (ibox[2],ibox[3])
        # cv2.rectangle(iimg,point0,point1,(0,0,255),2)
        # cv2.imshow('1',iimg)

        # point0 = (vbox[0],vbox[1])
        # point1 = (vbox[2],vbox[3])
        # cv2.rectangle(vimg,point0,point1,(0,0,255),2)
        # cv2.imshow('2',vimg)
        # cv2.waitKey(1)

        iimgCrop ,vimgCrop= crop_like_SiamFC(iimg, ibox),crop_like_SiamFC(vimg, vbox)
        cv2.imwrite(iimgpw,iimgCrop)
        cv2.imwrite(vimgpw,vimgCrop)

    return 'shit,bro'



def crop_hwc(image, bbox, out_sz, padding=(0, 0, 0)):
    a = (out_sz-1) / (bbox[2]-bbox[0])
    b = (out_sz-1) / (bbox[3]-bbox[1])
    c = -a * bbox[0]
    d = -b * bbox[1]
    mapping = np.array([[a, 0, c],
                        [0, b, d]]).astype(np.float64)
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


#####
# if __name__ == '__main__':
#     all_videos = glob(os.path.join(dataPath, '*')) 
#     lenth = len(all_videos)

#     inf = []
#     # with Pool(processes=16) as pool:
#     #     for ret in tqdm(pool.imap_unordered(worker,  all_videos), total=len(all_videos)): 
#     #         inf.append(ret)

#     for i,v in  enumerate(tqdm(all_videos)): 
#         ret =  worker(v)
#         inf.append(ret)

#     pklFile = open(output_path+'vid2015.pkl', 'wb')
#     pickle.dump(inf, pklFile)


mymain(inpath,outpath)












# outpath = '/media/secret/data/hfc/DATA/TRAIN/DB/got10kval' 
# #step1

# # infos = json.load(open(outpath+'/self_old.json', 'r'))
# # infosmin = copy.deepcopy(infos) 
# # for i in range(len(infos)):
# #     infosmin[i][0][0]=''
# #     infosmin[i][0][1] = infosmin[i][0][1].replace('GOT10Kval','got10kval')  

# # json.dump(infosmin, open(outpath+'/self.json', 'w'), indent=4, sort_keys=True)

# #step2
# infos = json.load(open(outpath+'/self.json', 'r'))
# lighting = lmdb.open(outpath+'/self.lmdb', map_size=int(1e9))
# with Pool(processes=12) as pool:
#     for x in tqdm(pool.imap_unordered(lmdbworker, infos), total=len(infos)): 
#         with lighting.begin(write=True) as txn:
#             for k, v in x.items():
#                 txn.put(k, v)
# print('lmdb done!')







