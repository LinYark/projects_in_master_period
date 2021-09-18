import torch
import torch.nn.functional as F
import torch.optim as optim
import torchvision.transforms as transforms
import torchvision
import numpy as np
import pandas as pd
import os
import cv2
import pickle
import lmdb
import os
import sys
sys.path.append(os.getcwd())

from fire import Fire
from torch.autograd import Variable
from torch.optim.lr_scheduler import StepLR
from torch.utils.data import DataLoader
from glob import glob
from tqdm import tqdm

import random

import torch

import os
import numpy as np
import json
import lmdb
import hashlib
from torch.utils.data.dataset import Dataset

from data.preAction.bbox import center2corner, Center
from data.preAction.augmentation import Augmentation
from data.preAction.point_target import PointTarget
from util.funcs import logger
import matplotlib
import matplotlib.pyplot as plt


# np.random.seed(5)

class CombineDataset(Dataset):
    def __init__(self, jsonpathsin,alltimes,rgbt,dbflg=False):     #outputDir is a list

        self.z_trans = Augmentation(4,0.05,0.0,0.0,1.0)
        self.x_trans = Augmentation(64,0.18,0.2,0.0,1.0)    
        self.rgbtsignal = rgbt

        self.point_target = PointTarget()
        ####################    eg ,[[100,2], [200,3]]
        self.datas = []         # [json1,json2]
        
        self.dbflg,self.dbs  = dbflg,[]

        self.lenthtable = []    # [200,600]
        self.singeltable = []   # [100,200]
        self.singelallsize = 0  # 300
        self.allsize = 0        # 800
        # dblist = {'got10k':70e9,'got10kval':1e9,'lasot':150e9,'lasotmini':30e9,'rgbt234':20e9,'gtot':5e9,'rgbt2':10e9,'rgbt1':10e9
        #     }
        for i in range(len(jsonpathsin)):
            jsonpaths = jsonpathsin[i][0]
            #
            if(dbflg==True):
                dbpath = jsonpaths.replace('json','lmdb')
                dbname = dbpath.split('/')[-2]
                # dbsize = dblist[dbname]
                # cur_db = lmdb.open(dbpath, readonly=True, map_size=int(dbsize))
                cur_db = lmdb.open(dbpath)
                cur_txn = cur_db.begin(write = False)
                self.dbs.append(cur_txn)
            #

            data = json.load(open(jsonpaths, 'r'))
            self.datas.append(data)
            times = jsonpathsin[i][1]

            singellenth,tablelenth = len(data),len(data)*times

            self.singeltable.append(singellenth)
            self.singelallsize += singellenth
            self.lenthtable.append(tablelenth) 
            self.allsize += tablelenth
            msg = jsonpaths.split('/')[-2]+'datasets lenth: '+str(singellenth)+' , '+str(tablelenth)
            print(msg) ,logger.info(msg)

        self.num = self.allsize*alltimes      # 800 *alltimes    




    def _get_bbox(self, image, shape):
        imh, imw = image.shape[:2]
        w, h = shape[2]-shape[0], shape[3]-shape[1]
        context_amount = 0.5
        exemplar_size = 127
        wc_z = w + context_amount * (w+h)
        hc_z = h + context_amount * (w+h)
        s_z = np.sqrt(wc_z * hc_z)
        if s_z==0:
            a=1
        scale_z = exemplar_size / s_z
        w = w*scale_z
        h = h*scale_z
        cx, cy = imw//2, imh//2
        bbox = center2corner(Center(cx, cy, w, h))
        return bbox

    def _filter(self,gt,i):
        nv,ni = gt[1][1][i],gt[-1][1][i]
        if len(nv)==1 or len(ni)==1:
            condition = False
            return condition
        nv,ni = np.array(nv),np.array(ni)
        dv,di = nv[2:]-nv[:2],ni[2:]-ni[:2]
        condition =( np.all(dv>5) and np.all(di>5) )
        return condition

    def imread(self,path,dbtableChooseNum ):
        # img = cv2.imread(path)
        # cv2.imshow('nomarl',img)
        # cv2.waitKey(1)
        if self.dbflg == True:
            txn = self.dbs[dbtableChooseNum]
            path = path.split('DB/')[-1]
            k = hashlib.md5(path.encode()).digest()
            v = txn.get(k)
            v = np.frombuffer(v, np.uint8)
            img = cv2.imdecode(v, cv2.IMREAD_COLOR)
            # cv2.imshow('lmdb',img)
            # cv2.waitKey(0)
            return img
        else:
            img = cv2.imread(path)
            return img


    def _samemakeRGBT(self,ximgi, zimgi,xinfo, zinfo,xtableChooseNum,ztableChooseNum,neg):
        xlastpath, zlastpath = xinfo[0][1],zinfo[0][1]

        (vxpath ,vzpath),(ixpath ,izpath) = ((xlastpath+'/'+xinfo[1][0][ximgi] , zlastpath+'/'+zinfo[1][0][zimgi]),
                                            (xlastpath+'/'+xinfo[-1][0][ximgi] , zlastpath+'/'+zinfo[-1][0][zimgi]))

        (vxgt,vzgt),(ixgt,izgt) = ((xinfo[1][1][ximgi],zinfo[1][1][zimgi]),
                                    (xinfo[-1][1][ximgi],zinfo[-1][1][zimgi]))
        #错位能力#
        # (ixgt,izgt) = (vxgt,vzgt) 
        
        
        # (vx,vz),(ix,iz)= ((cv2.imread(vxpath),cv2.imread(vzpath)),
        #                     (cv2.imread(ixpath,0),cv2.imread(izpath,0)) )
        # ix,iz = np.stack((ix,ix,ix),axis=2) ,np.stack((iz,iz,iz),axis=2)
        (vx,vz),(ix,iz)= ((self.imread(vxpath,xtableChooseNum),self.imread(vzpath,ztableChooseNum)),
                            (self.imread(ixpath,xtableChooseNum),self.imread(izpath,ztableChooseNum)) )
        ix,iz = cv2.cvtColor(ix, cv2.COLOR_BGR2GRAY) ,cv2.cvtColor(iz, cv2.COLOR_BGR2GRAY)
        ix,iz = np.stack((ix,ix,ix),axis=2) ,np.stack((iz,iz,iz),axis=2)
        (vxcropgt,vzcropgt),(ixcropgt,izcropgt) =((self._get_bbox(vx, vxgt),self._get_bbox(vz, vzgt)),
                                                    (self._get_bbox(ix, ixgt),self._get_bbox(iz, izgt))) 

        sameblur = (np.random.random()<0.8)
        if sameblur:
            blurv,x_kernelv = np.random.random(),Augmentation.rand_kernel()
            bluri,x_kerneli = blurv,x_kernelv
        else:
            blurv,x_kernelv = np.random.random(),Augmentation.rand_kernel()
            bluri,x_kerneli = np.random.random(),Augmentation.rand_kernel()
            

        x_random_scale_x,x_random_scale_y,x_random_sx ,x_random_sy  = Augmentation.random(),Augmentation.random(),Augmentation.random(),Augmentation.random()
        z_random_scale_x,z_random_scale_y,z_random_sx,z_random_sy = Augmentation.random(), Augmentation.random(), Augmentation.random(), Augmentation.random()
        littleshiftmax = 0.1
        i_x_shift ,i_y_shift= (np.random.random() *2 -1 )*littleshiftmax,(np.random.random() *2 -1 )*littleshiftmax

        ((vx,vxtranbox),(vz,vztranbox)) ,((ix,ixtranbox),(iz,iztranbox)) = (
                            (self.x_trans(vx,vxcropgt,255, x_random_scale_x, x_random_scale_y, x_random_sx          ,x_random_sy          , blurv, x_kernelv),
                             self.z_trans(vz,vzcropgt,127, z_random_scale_x, z_random_scale_y, z_random_sx          ,z_random_sy          )),
                            (self.x_trans(ix,ixcropgt,255, x_random_scale_x, x_random_scale_y, x_random_sx+i_x_shift,x_random_sy+i_y_shift, bluri, x_kerneli),
                             self.z_trans(iz,izcropgt,127, z_random_scale_x, z_random_scale_y, z_random_sx+i_x_shift,z_random_sy+i_y_shift))   )

        # mi ,ma= np.min(vx),np.max(vx)
        # x1 = (vx-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(vxtranbox[0]),int(vxtranbox[1]),int(vxtranbox[2]),int(vxtranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posx',x2)
        # mi ,ma= np.min(vz),np.max(vz)
        # x1 = (vz-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(vztranbox[0]),int(vztranbox[1]),int(vztranbox[2]),int(vztranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posz',x2)

        # mi ,ma= np.min(ix),np.max(ix)
        # x1 = (ix-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(ixtranbox[0]),int(ixtranbox[1]),int(ixtranbox[2]),int(ixtranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posxi',x2)
        # mi ,ma= np.min(iz),np.max(iz)
        # x1 = (iz-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(iztranbox[0]),int(iztranbox[1]),int(iztranbox[2]),int(iztranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('poszi',x2)

        # cv2.waitKey(0)

        (vcls, vloc),(icls, iloc) = self.point_target(vxtranbox, 25,neg),self.point_target(ixtranbox, 25,neg)
        (vx,vz), (ix,iz) = (
                (vx.transpose((2, 0, 1)).astype(np.float32),
                vz.transpose((2, 0, 1)).astype(np.float32)),
                (ix.transpose((2, 0, 1)).astype(np.float32),
                iz.transpose((2, 0, 1)).astype(np.float32))     )    
                        


        return { 
            'vz': vz,
            'vx': vx,
            'vcls' : vcls,
            'vloc' : vloc,
            #'vclspad':vclspad,
            'iz': iz,
            'ix': ix,
            'icls' : icls,
            'iloc' : iloc,
            #'iclspad':iclspad,
            }


    def _samemakeRGB(self,ximgi, zimgi,xinfo, zinfo,neg):
       
        xlastpath, zlastpath = xinfo[0][1],zinfo[0][1]
        xpath ,zpath = xlastpath+'/'+xinfo[-1][0][ximgi] , zlastpath+'/'+zinfo[-1][0][zimgi]
        xgt,zgt = xinfo[-1][1][ximgi],zinfo[-1][1][zimgi]

        
        x,z= cv2.imread(xpath),cv2.imread(zpath)

        # x,z= cv2.imread(xpath,0),cv2.imread(zpath,0)
        # x  = np.stack((x,x,x),axis=2)
        # z = np.stack((z,z,z),axis=2)

        xcropgt,zcropgt = self._get_bbox(x, xgt),self._get_bbox(z, zgt)
        z, ztranbox = self.z_trans(z,zcropgt,127)
        x, xtranbox = self.x_trans(x,xcropgt,255)


        cls, delta = self.point_target(xtranbox, 25, neg)
        x = x.transpose((2, 0, 1)).astype(np.float32)
        z = z.transpose((2, 0, 1)).astype(np.float32)

        return {        
            'z':          z,
            'x':          x,
            'label_cls':  cls,
            'label_loc':  delta,
            }


    def _imgbadmake(self,img,badnum):
        # img = self._imgbadmakehanning2(img,badnum)
        # return img
        random_use_hanning = (np.random.random()<0.6) 
        if random_use_hanning ==True:
            img = self._imgbadmakehanning2(img,badnum)
            return img
        else: 
            img1 = np.array(img,dtype = float)
            img2 = img1/255
            multimes =np.random.choice( np.arange(0.8,1.4,0.1))
            img3 = np.power(img2,multimes)
            img4 = img3*255
            img5 = img4+badnum
            img5[img5>255]=255
            img5[img5<0]=0
            img6 = np.array(img5,dtype = np.uint8)
            img7 = cv2.GaussianBlur(img6,(5,5),0)
            return img7


    def _imgbadmakehanning(self,img,badnum,rangenumin=46,sizex =120 ,sizey =300 ):      #sizx和sizy表达有误
        rangenum = rangenumin
        rangelist= np.arange(255-rangenum,255+rangenum)
        weights = np.sqrt(rangenum-abs(rangelist - 255))
        weights = weights/sum(weights)
        centerx = np.random.choice(rangelist,p=weights)      
        centery = np.random.choice(rangelist,p=weights) 
        hanningsizex = np.random.choice(np.arange(sizex,sizey))
        hanningsizey = np.random.choice(np.arange(sizex,sizey)) 
        window = np.outer(np.hanning(hanningsizex), np.hanning(hanningsizey))
        xfirst = centerx-hanningsizex//2
        yfirst = centery-hanningsizey//2

        img1 = np.array(img,dtype = float)
        img2 = img1/255
        if badnum>0:
            img2 = img2.transpose((2, 0, 1))
            img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] = \
                img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] +window
            
        if badnum<0:
            img2 = img2.transpose((2, 0, 1))
            img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] = \
                img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] -window   
        img2 = img2.transpose(1,2,0)
        img3 = img2*255
        img3[img3>255]=255
        img3[img3<0]=0
        img4 = np.array(img3,dtype = np.uint8)
        # img6 = cv2.GaussianBlur(img5,(3,3),0)
        return img4


    def _imgbadmakehanning2(self,img,badnum,rangenumin=55,sizex =120 ,sizey =300 ):
        rangenum = rangenumin
        rangelist= np.arange(255-rangenum,255+rangenum)
        weights = np.sqrt(rangenum-abs(rangelist - 255))
        weights = weights/sum(weights)
        centerx = np.random.choice(rangelist,p=weights)      
        centery = np.random.choice(rangelist,p=weights) 
        hanningsizex = np.random.choice(np.arange(sizex,sizey))
        hanningsizey = np.random.choice(np.arange(sizex,sizey)) 
        window = np.outer(np.hanning(hanningsizex), np.hanning(hanningsizey))
        xfirst = centerx-hanningsizex//2
        yfirst = centery-hanningsizey//2

        img1 = np.array(img,dtype = float)
        img2 = img1/255

        backgroundrandom = (np.random.random()<0.8)
        if backgroundrandom == True:
            if badnum>0:
                img2 = img2.transpose((2, 0, 1))
                img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] = \
                    img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] +window
                
            if badnum<0:
                img2 = img2.transpose((2, 0, 1))
                img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] = \
                    img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] -window   
        else:
            # print('gauss')
            blurkenelsize = np.arange(35, 65, 2) #5,46
            size1 = np.random.choice(blurkenelsize)
            imggauss = cv2.GaussianBlur(img2,(size1,size1),0)

            powersize  = np.arange(3, 5)
            size2 = np.random.choice(powersize)
            window = np.power(window,1/size2)

            windowneg = 1- window
            img2 = img2.transpose((2, 0, 1))
            imggauss = imggauss.transpose((2, 0, 1))
            img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] = \
                    img2[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] * windowneg + \
                        imggauss[:,xfirst:(xfirst+hanningsizex),yfirst:(yfirst+hanningsizey)] * window

        img2 = img2.transpose(1,2,0)
        img3 = img2*255
        img3[img3>255]=255
        img3[img3<0]=0
        img4 = np.array(img3,dtype = np.uint8)
        # img6 = cv2.GaussianBlur(img5,(3,3),0)
        return img4

    def spatialcore(self,vx,vz,ix,iz):
        randomthresd =  np.random.choice( list(range(150,225)))   

        random_use_who = np.random.random()   #rgb _t
        random_both = (np.random.random()<0.8) #  xz,or x or z

        if(random_use_who <0.4):
            randomblack = (np.random.random()<0.5) 
            if (randomblack==True):
                randomthresd  = randomthresd*(-1)
            if random_both ==True:
                (vx,vz) = self._imgbadmake(vx,randomthresd),self._imgbadmake(vz,randomthresd)
            else:
                random_use_x = np.random.random()<0.5
                if random_use_x==True:
                    vx = self._imgbadmake(vx,randomthresd)
                else:
                    vz= self._imgbadmake(vz,randomthresd)
        elif(random_use_who <=0.9):
            randomblack = (np.random.random()<0.8) 
            if (randomblack==True):
                randomthresd  = randomthresd*(-1)

            if random_both ==True:
                (ix,iz) = self._imgbadmake(ix,randomthresd),self._imgbadmake(iz,randomthresd)
            else:
                random_use_x = np.random.random()<0.5
                if random_use_x==True:
                    ix = self._imgbadmake(ix,randomthresd)
                else:
                    iz= self._imgbadmake(iz,randomthresd)
        elif(random_use_who <0.95): #交叉变坏
            randomvzix = np.random.random()<0.5
            if randomvzix==True :
                vz,ix = self._imgbadmakehanning2(vz,(np.random.random() -0.5),sizex =120 ,sizey =160), self._imgbadmakehanning2(ix,(np.random.random() -0.7),sizex =120 ,sizey =160)
            else:
                vx,iz = self._imgbadmakehanning2(vx,(np.random.random() -0.5),sizex =120 ,sizey =160), self._imgbadmakehanning2(iz,(np.random.random() -0.7),sizex =120 ,sizey =160)

        else:   
            randomzrgb_t = (np.random.random()<0.7) #0.7 zrgb bad   #坏三个
            if randomzrgb_t == True:    
                randomblack = np.random.random()*2 -1
                vz,iz = self._imgbadmakehanning2(vz,randomblack), iz
                vx,ix = self._imgbadmakehanning2(vx,randomblack,sizex =120 ,sizey =160), self._imgbadmakehanning2(ix,(np.random.random() -0.7),sizex =120 ,sizey =160)
            else:
                randomblack = np.random.random() -0.4
                vz,iz = vz, self._imgbadmakehanning2(iz,randomblack)
                vx,ix = self._imgbadmakehanning2(vx,(np.random.random() -0.5),sizex =120 ,sizey =160), self._imgbadmakehanning2(ix,randomblack,sizex =120 ,sizey =160)

        return vx,vz,ix,iz

    def _spatialmakeRGBT(self,ximgi, zimgi,xinfo, zinfo,xtableChooseNum,ztableChooseNum,neg):
        xlastpath, zlastpath = xinfo[0][1],zinfo[0][1]

        (vxpath ,vzpath),(ixpath ,izpath) = ((xlastpath+'/'+xinfo[1][0][ximgi] , zlastpath+'/'+zinfo[1][0][zimgi]),
                                            (xlastpath+'/'+xinfo[-1][0][ximgi] , zlastpath+'/'+zinfo[-1][0][zimgi]))

        (vxgt,vzgt),(ixgt,izgt) = ((xinfo[1][1][ximgi],zinfo[1][1][zimgi]),
                                    (xinfo[-1][1][ximgi],zinfo[-1][1][zimgi]))
        #错位能力#
        # (ixgt,izgt) = (vxgt,vzgt)    

        # (vx,vz),(ix,iz)= ((cv2.imread(vxpath),cv2.imread(vzpath)),
        #                     (cv2.imread(ixpath,0),cv2.imread(izpath,0)) )
        # ix,iz = np.stack((ix,ix,ix),axis=2) ,np.stack((iz,iz,iz),axis=2)
        (vx,vz),(ix,iz)= ((self.imread(vxpath,xtableChooseNum),self.imread(vzpath,ztableChooseNum)),
                            (self.imread(ixpath,xtableChooseNum),self.imread(izpath,ztableChooseNum)) )
        ix,iz = cv2.cvtColor(ix, cv2.COLOR_BGR2GRAY) ,cv2.cvtColor(iz, cv2.COLOR_BGR2GRAY)
        ix,iz = np.stack((ix,ix,ix),axis=2) ,np.stack((iz,iz,iz),axis=2)
        vx,vz,ix,iz = self.spatialcore(vx,vz,ix,iz )
        (vxcropgt,vzcropgt),(ixcropgt,izcropgt) =((self._get_bbox(vx, vxgt),self._get_bbox(vz, vzgt)),
                                                    (self._get_bbox(ix, ixgt),self._get_bbox(iz, izgt))) 

        sameblur = (np.random.random()<0.8)
        if sameblur:
            blurv,x_kernelv = np.random.random(),Augmentation.rand_kernel()
            bluri,x_kerneli = blurv,x_kernelv
        else:
            blurv,x_kernelv = np.random.random(),Augmentation.rand_kernel()
            bluri,x_kerneli = np.random.random(),Augmentation.rand_kernel()
        x_random_scale_x,x_random_scale_y,x_random_sx ,x_random_sy  = Augmentation.random(),Augmentation.random(),Augmentation.random(),Augmentation.random()
        z_random_scale_x,z_random_scale_y,z_random_sx,z_random_sy = Augmentation.random(), Augmentation.random(), Augmentation.random(), Augmentation.random()
        littleshiftmax = 0.1
        i_x_shift ,i_y_shift= (np.random.random() *2 -1 )*littleshiftmax,(np.random.random() *2 -1 )*littleshiftmax

        ((vx,vxtranbox),(vz,vztranbox)) ,((ix,ixtranbox),(iz,iztranbox)) = (
                            (self.x_trans(vx,vxcropgt,255, x_random_scale_x, x_random_scale_y, x_random_sx          ,x_random_sy          , blurv, x_kernelv),
                             self.z_trans(vz,vzcropgt,127, z_random_scale_x, z_random_scale_y, z_random_sx          ,z_random_sy          )),
                            (self.x_trans(ix,ixcropgt,255, x_random_scale_x, x_random_scale_y, x_random_sx+i_x_shift,x_random_sy+i_y_shift, bluri, x_kerneli),
                             self.z_trans(iz,izcropgt,127, z_random_scale_x, z_random_scale_y, z_random_sx+i_x_shift,z_random_sy+i_y_shift))   )

        # mi ,ma= np.min(vx),np.max(vx)
        # x1 = (vx-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(vxtranbox[0]),int(vxtranbox[1]),int(vxtranbox[2]),int(vxtranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posx',x2)
        # mi ,ma= np.min(vz),np.max(vz)
        # x1 = (vz-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(vztranbox[0]),int(vztranbox[1]),int(vztranbox[2]),int(vztranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posz',x2)

        # mi ,ma= np.min(ix),np.max(ix)
        # x1 = (ix-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(ixtranbox[0]),int(ixtranbox[1]),int(ixtranbox[2]),int(ixtranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('posxi',x2)
        # mi ,ma= np.min(iz),np.max(iz)
        # x1 = (iz-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # box = [int(iztranbox[0]),int(iztranbox[1]),int(iztranbox[2]),int(iztranbox[3])]
        # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
        # cv2.imshow('poszi',x2)

        # cv2.waitKey(0)

   

        (vcls, vloc),(icls, iloc) = self.point_target(vxtranbox, 25,neg),self.point_target(ixtranbox, 25,neg)
        (vx,vz), (ix,iz) = (
                (vx.transpose((2, 0, 1)).astype(np.float32),
                vz.transpose((2, 0, 1)).astype(np.float32)),
                (ix.transpose((2, 0, 1)).astype(np.float32),
                iz.transpose((2, 0, 1)).astype(np.float32))     )    
                        


        return { 
            'vz': vz,
            'vx': vx,
            'vcls' : vcls,
            'vloc' : vloc,

            'iz': iz,
            'ix': ix,
            'icls' : icls,
            'iloc' : iloc,
            
            }




    def dividenum(self,idx,table):
        decitionNum = idx
        tableChooseNum = 0
        for i in range(len(table)):
            decitionNum -= table[i]
            if(decitionNum <0):
                dirchoosenum =decitionNum +  table[i]    #最后一个数据集的choosedir
                break
            else:
                tableChooseNum += 1
        return decitionNum,tableChooseNum

    def _neg_pos(self,idx):
        neg = 0.2 and (np.random.random()<0.2) 
        if(neg):

            xtablenum ,ztalblenum = np.random.choice(len(self.lenthtable)) ,np.random.choice(len(self.lenthtable))
            xdata ,zdata= self.datas[xtablenum] , self.datas[ztalblenum]
            xdiri , zdiri =  np.random.choice(len(xdata)), np.random.choice(len(zdata))

            #check different dir
            if xdiri == zdiri and xtablenum== ztalblenum:
                listchoose =np.arange(len(zdata))
                listchoose = np.delete(listchoose,zdiri,axis=0)
                zdiri = np.random.choice(listchoose)
            xinfo , zinfo = xdata[xdiri],zdata[zdiri]

            #check data is right
            condition1 ,condition2 = self._filter(xinfo[1][1]),self._filter( zinfo[1][1])
            if( condition1== False or condition2== False ):
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)

            ximgi, zimgi = np.random.choice(len(xinfo[1][0])),np.random.choice(len(zinfo[1][0]))
            xinfo, zinfo = xinfo, zinfo 

            if self.rgbtsignal==True :
                backinfo = self._samemakeRGBT(ximgi, zimgi,xinfo, zinfo,True)
            else:
                backinfo = self._samemakeRGB(ximgi, zimgi,xinfo, zinfo,True)
            return backinfo

        else:
            #compute which dir
            decitionNum = idx%self.allsize
            tableChooseNum = 0
            for i in range(len(self.lenthtable)):
                decitionNum -= self.lenthtable[i]
                if(decitionNum <0):
                    dirchoosenum =decitionNum +  self.lenthtable[i]    #最后一个数据集的choosedir
                    break
                else:
                    tableChooseNum += 1
            data = self.datas[tableChooseNum]
            infos = data[dirchoosenum]

            #check and choose i,j
            lastpath,paths,gts =  infos[0][1],infos[1][0],infos[1][1]
            condition1 = self._filter(gts)
            if( condition1== False  ):
                index = np.random.choice(len(self.datas))
                return self.__getitem__(index)

            length = len(paths)
            i = np.random.choice(length)
            low_idx,up_idx = max(0, i - 200), min(length, i + 200)  
            choosetemp = list(range(low_idx,up_idx))
            j =  np.random.choice(choosetemp)

            ximgi, zimgi = i,j
            xinfo, zinfo =  infos,infos

            if self.rgbtsignal==True :
                backinfo = self._samemakeRGBT(ximgi, zimgi,xinfo, zinfo,False)
            else:
                backinfo = self._samemakeRGB(ximgi, zimgi,xinfo, zinfo,False)
            return backinfo

    def _neg_redneg_pos(self,idx):
        randomnum = np.random.random()
        # randomnum =0.1
        if(randomnum<0.18):
            xidx ,zidx  = np.random.choice(self.allsize , 2,replace=False)  # no need to thought same dir , itis so hard to effect
            xdecitionNum,xtableChooseNum    = self.dividenum(xidx,self.lenthtable)
            zdecitionNum,ztableChooseNum    = self.dividenum(zidx,self.lenthtable)
            xdata ,zdata= self.datas[xtableChooseNum] , self.datas[ztableChooseNum]
            xdirchoosenum, zdirchoosenum = xdecitionNum % self.singeltable[xtableChooseNum] , zdecitionNum % self.singeltable[ztableChooseNum]
            xdiri , zdiri =  xdirchoosenum, zdirchoosenum
            xinfo , zinfo = xdata[xdiri],zdata[zdiri]

            #check data is right
            lenthx,lenthz = len(xinfo[1][0]),len(zinfo[1][0])
            if lenthx<3 or lenthz<3:    
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)
            ximgi, zimgi = np.random.choice(lenthx),np.random.choice(lenthz)
            xinfo, zinfo = xinfo, zinfo 

            condition1 ,condition2 = self._filter(xinfo,ximgi),self._filter( zinfo,zimgi)
            if( condition1== False or condition2== False ):
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)

            if self.rgbtsignal==True :
                backinfo = self._samemakeRGBT(ximgi, zimgi,xinfo, zinfo,xtableChooseNum,ztableChooseNum,True)
            else:
                backinfo = self._samemakeRGB(ximgi, zimgi,xinfo, zinfo,True)
            return backinfo

        elif (randomnum<0.3):
           #compute which dir
            idx = idx%self.allsize
            dirchoosenum ,tableChooseNum = self.dividenum(idx,self.lenthtable)
            data = self.datas[tableChooseNum]
            dirchoosenum = dirchoosenum % self.singeltable[tableChooseNum]
            infos = data[dirchoosenum]

            #check and choose i,j

            length = len(infos[1][0])
            if length<3:
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)
            i = np.random.choice(length)
            low_idx,up_idx = max(0, i - 150), min(length, i + 150)  
            choosetemp = list(range(low_idx,up_idx))
            j =  np.random.choice(choosetemp)
            ximgi, zimgi = i,j
            xinfo, zinfo =  infos,infos

            condition1 ,condition2 = self._filter(xinfo,ximgi),self._filter( zinfo,zimgi)
            if( condition1== False or condition2== False ):
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)

            backinfo = self._spatialmakeRGBT(ximgi, zimgi,xinfo, zinfo,tableChooseNum,tableChooseNum,False)
            return backinfo
        else:
            #compute which dir
            idx = idx%self.allsize
            dirchoosenum ,tableChooseNum = self.dividenum(idx,self.lenthtable)
            data = self.datas[tableChooseNum]
            dirchoosenum = dirchoosenum % self.singeltable[tableChooseNum]
            infos = data[dirchoosenum]

            #check and choose i,j

            length = len(infos[1][0])
            if length<3:
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)
            i = np.random.choice(length)
            low_idx,up_idx = max(0, i - 150), min(length, i + 150)  
            choosetemp = list(range(low_idx,up_idx))
            j =  np.random.choice(choosetemp)

            ximgi, zimgi = i,j
            xinfo, zinfo =  infos,infos
            condition1 ,condition2 = self._filter(xinfo,ximgi),self._filter( zinfo,zimgi)
            if( condition1== False or condition2== False ):
                index = np.random.choice(self.allsize)
                return self.__getitem__(index)

            if self.rgbtsignal==True :
                backinfo = self._samemakeRGBT(ximgi, zimgi,xinfo, zinfo,tableChooseNum,tableChooseNum,False)
            else:
                backinfo = self._samemakeRGB(ximgi, zimgi,xinfo, zinfo,tableChooseNum,tableChooseNum,False)
            return backinfo


    def __getitem__(self, idx):
        # self._neg_pos(idx)
        backinfo = self._neg_redneg_pos(idx)
        return backinfo


    def __len__(self):
        return self.num



if __name__ == "__main__":
    a1 = VIDDataset('/media/hfc/HFCU/DATA_ALL/TRAIN/VOT18')
    a = iter(a1)
    for x in a:
        pass
    b = next(iter(a))


    a = 1

            # mi ,ma= np.min(z),np.max(z)
            # x1 = (z-mi)/(ma-mi)*240
            # x2 = np.array(x1,dtype=np.uint8)
            # box = [int(ztranbox[0]),int(ztranbox[1]),int(ztranbox[2]),int(ztranbox[3])]
            # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
            # cv2.imshow('posz',x2)
            # mi ,ma= np.min(x),np.max(x)
            # x1 = (x-mi)/(ma-mi)*240
            # x2 = np.array(x1,dtype=np.uint8)
            # box = [int(xtranbox[0]),int(xtranbox[1]),int(xtranbox[2]),int(xtranbox[3])]
            # cv2.rectangle(x2,(box[0],box[1]),(box[2],box[3]),(0,255,0),2)
            # cv2.imshow('posx',x2)
            # cv2.waitKey(0)




        # plt.subplot(4,2,1)
        # plt.imshow(backinfo['vz'][0,:,:]), plt.axis('off')
        # plt.subplot(4,2,2)
        # plt.imshow(backinfo['vx'][0,:,:]), plt.axis('off') #这里显示灰度图要加cmap
        # plt.subplot(4,2,3)
        # plt.imshow(backinfo['vcls']), plt.axis('off')
        # plt.subplot(4,2,4)
        # plt.imshow(backinfo['vloc'][0,:,:]), plt.axis('off')
        # plt.subplot(4,2,5)
        # plt.imshow(backinfo['iz'][0,:,:]), plt.axis('off')
        # plt.subplot(4,2,6)
        # plt.imshow(backinfo['ix'][0,:,:]), plt.axis('off')
        # plt.subplot(4,2,7)
        # plt.imshow(backinfo['icls']), plt.axis('off')
        # plt.subplot(4,2,8)
        # plt.imshow(backinfo['iloc'][0,:,:]), plt.axis('off')

        # plt.show()

        # cv2.waitKey(0)




