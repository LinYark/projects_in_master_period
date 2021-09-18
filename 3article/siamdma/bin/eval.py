# Copyright (c) SenseTime. All Rights Reserved.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys
sys.path.append(os.getcwd())

import cv2
import torch
import numpy as np
from glob import glob
import time

from siamdb.model_builder import ModelBuilder
from siamdb.tracker.tracker_builder import build_tracker
from siamdb.util.bbox import get_axis_aligned_bbox




dbpath = '/opt/data/private/data/DB/'   
dataset_root = '/media/hfc/HFCU/DATA_ALL/TEST/GTOT'     #GTOT(fix)
datasetName = 'GTOT'


dataset_root ='/media/hfc/HFCU/DATA_ALL/TEST/lasher3'#rgbthard,RGBT2
datasetName = 'RGBT234'
vis=True


def main():

    torch.backends.cudnn.benchmark = True
    #get all dataset info
    list1 = np.arange(22,24)
    list1 = [21]
    # dict_snapshot = ['step2_left/step2_15.pth']
    dict_snapshot =[ 'step1/step1_'+str(x) +'.pth' for x in list1  ]
    # dict_snapshot = ['step2_18.pth']
    dict_trackerName = dict_snapshot
    for i in range(len(dict_snapshot)):
        snapshot ='./snapshot/'+ dict_snapshot[i]
        trackerName = dict_trackerName[i]
        
        with torch.no_grad():
            allinfo = getinfo(datasetName)
            
            # create model
            model = ModelBuilder().cuda()

            load_data = torch.load( snapshot )
            model.load_state_dict(load_data['state_dict'])

            # model.vbackbone.load_state_dict(load_data['vbackbone'])
            # model.ibackbone.load_state_dict(load_data['ibackbone'])
            # model.vneck.load_state_dict(load_data['vneck'])
            # model.ineck.load_state_dict(load_data['ineck'])
            # model.enhance.load_state_dict(load_data['enhance'])
            # model.mix.load_state_dict(load_data['mix'])
            # model.head.load_state_dict(load_data['head'])   

            model.eval()
            tracker = build_tracker(model)

            # sizew=np.arange(0.05, 0.2, 0.05).tolist()      #0.1-0.8
            # windoww=np.arange(0.3, 0.6, 0.1).tolist()    #0.1-0.8
            # lrw=np.arange(0.3, 0.6, 0.1).tolist()        #0.1-0.

            sizew =[0.1]
            windoww = [0.475]
            lrw = [0.475]
            for tune1 in range(len(sizew)):
                for tune2 in range(len(windoww)):
                    for tune3 in range(len(lrw)):

                        
                        cur_sizew,cur_windoww, cur_lrw= sizew[tune1],windoww[tune2],lrw[tune3]
                        cur_sizew,cur_windoww, cur_lrw = int(cur_sizew*100)/100,int(cur_windoww*100)/100, int(cur_lrw*100)/100
                        print(trackerName+'  cur: ',cur_sizew,cur_windoww, cur_lrw)
                        onetest( allinfo,tracker,cur_sizew,cur_windoww, cur_lrw ,trackerName  )




def onetest(allinfo,tracker,cur_sizew,cur_windoww, cur_lrw  ,trackerName ):

    for i in range(len(allinfo)):
        #get info
        curvideo = allinfo[i]
        frontpath , vbackpathlist , vinitbbox , ibackpathlist , iinitbbox  = curvideo[0][0],curvideo[1][0],curvideo[1][1][0],curvideo[2][0],curvideo[2][1][0]
        #get first img
        vinitrect,iinitrect =  (vinitbbox[0],vinitbbox[1], vinitbbox[2]-vinitbbox[0],vinitbbox[3]-vinitbbox[1]), (iinitbbox[0],iinitbbox[1], iinitbbox[2]-iinitbbox[0],iinitbbox[3]-iinitbbox[1])
        vimgpath,iimgpath = frontpath+'/'+vbackpathlist[0],frontpath+'/'+ibackpathlist[0]
        vimg,iimg = cv2.imread(vimgpath),cv2.imread(iimgpath)
        tracker.init(vimg,vinitrect,iimg,iinitrect)

        #write first
        # resultpath = './result'+'/'+datasetName+'/'+trackerName + '/SWL_'+str(cur_sizew)+'_'+str(cur_windoww)+'_'+str(cur_lrw)
        resultpath = './result'+'/'+datasetName+'/'+trackerName 
        if not os.path.exists(resultpath):
            os.makedirs(resultpath)
        outtxtpath = resultpath+'/'+'ours_'+frontpath.split('/')[-1] + '.txt'
        f=open(outtxtpath,'w+')
        firstp = rect_2_8points(vinitrect)
        writepoints(f,firstp)

        #main loop
        # cv2.destroyAllWindows()
        count = 0
        for j in range(1,len(vbackpathlist)):
            tic = time.time()
            #track
            windowname = frontpath.split('/')[-1]
            vimgpath,iimgpath = frontpath+'/'+vbackpathlist[j],frontpath+'/'+ibackpathlist[j]
            vimg,iimg = cv2.imread(vimgpath),cv2.imread(iimgpath)

            # vimg = np.array((vimg*0.5 + iimg*0.5),dtype=np.uint8)
            outputs = tracker.track(vimg,iimg,cur_sizew,cur_windoww, cur_lrw)
            rect = outputs['bbox']
            #show
            count += 1
            if count%10==0:
            	tracker.update(vimg,iimg, rect)

            if vis == True:
                bbox = curvideo[1][1][j]
                toc=time.time()
                fps =int( 1/(toc-tic))
                cv2.putText(vimg,('FPS: '+str(fps)),(40, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
                r1 = rect2intrect(rect)
                cv2.rectangle(vimg, (r1[0], r1[1]),(r1[0]+r1[2], r1[1]+r1[3]), (0, 255, 255), 2)
                cv2.rectangle(vimg, (bbox[0], bbox[1]),(bbox[2], bbox[3]), (0, 255, 0), 2)
                # cv2.imshow(windowname+'-rgb',vimg)
                cv2.imshow('-rgb',vimg)

                cv2.rectangle(iimg, (r1[0], r1[1]),(r1[0]+r1[2], r1[1]+r1[3]), (0, 255, 255), 2)
                cv2.rectangle(iimg, (bbox[0], bbox[1]),(bbox[2], bbox[3]), (0, 255, 0), 2)
                # cv2.imshow(windowname+'-i',iimg)          
                cv2.imshow('-i',iimg)          
                cv2.waitKey(1)
            #write
            points = rect_2_8points(rect)
            writepoints(f,points)

        
        f.close()





def rect2intrect(rect):
    r1 = np.array(rect,dtype=int).tolist()
    return r1


def getinfo(datasetname):
    names = ['RGBT234','GTOT']
    
    if datasetname == names[0]:
        allinfo = []   #((videopath,),((liftname,),(gt,)),),

        a_videos = glob(dataset_root+'/*')
        a_videos.sort()

        for x in a_videos:
            allinfo.append([[x]])

        nodes = ['visible','infrared','visible.txt','infrared.txt']
        for i in range(len(allinfo)):
            path = allinfo[i][0][0]
            vpath,ipath = path +'/'+ nodes[0] , path +'/'+ nodes[1] 
            vgt,igt = path +'/'+ nodes[2] , path +'/'+ nodes[3] 

            vliftnames ,iliftnames= glob(vpath+'/*'), glob(ipath+'/*')
            vliftnames.sort() ,iliftnames.sort()
            vliftnames1 ,iliftnames1 = [ x.replace(path+'/','')     for x in vliftnames] , [ x.replace(path+'/','')     for x in iliftnames]
            with open(vgt, "r") as f:
                vgt1 = f.readlines()
                vgt2 = [ x.replace('\n','').split(',') for x in vgt1   ]
                vlen = len(vgt2)
                for vgt2i in range(vlen):
                    vgt2i = vlen -vgt2i-1
                    if(len(vgt2[vgt2i])<=1):
                        vgt2.pop(vgt2i)
                vgt3 = np.array(vgt2,dtype=int).tolist()
                vgt4 = [ [x[0],x[1],x[0]+x[2],x[1]+x[3]]  for x in vgt3  ]
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

            allinfo[i].append([vliftnames1,vgt4])
            allinfo[i].append([iliftnames1,igt4])
        return allinfo

    elif datasetname == names[1]:
        allinfo = []   #((videopath,),((liftname,),(gt,)),),

        a_videos = glob(dataset_root+'/*')
        a_videos.sort()

        for x in a_videos:
            if os.path.isdir(x):
                allinfo.append([[x]])

        nodes = ['v','i','groundTruth_v.txt','groundTruth_i.txt']
        for i in range(len(allinfo)):
            path = allinfo[i][0][0]
            vpath,ipath = path +'/'+ nodes[0] , path +'/'+ nodes[1] 
            vgt,igt = path +'/'+ nodes[2] , path +'/'+ nodes[3] 

            vliftnames ,iliftnames= glob(vpath+'/*'), glob(ipath+'/*')
            vliftnames.sort() ,iliftnames.sort()
            vliftnames1 ,iliftnames1 = [ x.replace(path+'/','')     for x in vliftnames] , [ x.replace(path+'/','')     for x in iliftnames]
            with open(vgt, "r") as f:
                vgt1 = f.readlines()
                vgt2 = [ x.replace('\n','').split(' ') for x in vgt1   ]
                vlen = len(vgt2)
                for vgt2i in range(vlen):
                    vgt2i = vlen -vgt2i-1
                    if(len(vgt2[vgt2i])<=1):
                        vgt2.pop(vgt2i)
                vgt3 = np.array(vgt2,dtype=int).tolist()
                vgt4 = [ [x[0],x[1],x[2],x[3]]  for x in vgt3  ]
            with open(igt, "r") as f:
                igt1 = f.readlines()
                igt2 = [ x.replace('\n','').split(' ') for x in igt1   ]
                ilen = len(igt2)
                for igt2i in range(ilen):
                    igt2i = ilen -igt2i-1
                    if(len(igt2[igt2i])<=1):
                        igt2.pop(igt2i)
                igt3 = np.array(igt2,dtype=int).tolist()
                igt4 = [ [x[0],x[1],x[2],x[3]]  for x in igt3  ]

            allinfo[i].append([vliftnames1,vgt4])
            allinfo[i].append([iliftnames1,igt4])
        return allinfo

    else:
        print('wrong test dataset,check your dataset name! ')
        exit(0)


def rect_2_8points(rect):
    result_bb = rect
    points =  (result_bb[0],result_bb[1],
            result_bb[0]+result_bb[2],result_bb[1],
            result_bb[0]+result_bb[2],result_bb[1]+result_bb[3],
            result_bb[0],result_bb[1]+result_bb[3] )

    return points
    
def writepoints(f,points):
    res='{} {} {} {} {} {} {} {}'.format(points[0],points[1],points[2],points[3],points[4],points[5],points[6],points[7]) 
    f.write(res)
    f.write('\n')






if __name__ == '__main__':
    main()
