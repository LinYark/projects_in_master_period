import os
import sys
sys.path.append(os.getcwd())
import numpy as np
import torch.nn as nn
import torch.nn.functional as F
from visdom import Visdom


class visShow():
    def __init__(self,):
        self.vis =  Visdom()

    def text1(self,batchsize):
        self.vis.text(u"MainInfo:\nCurrent BatchSize = "+str(batchsize),win='MainInfo', opts={'title': 'MainInfo'})
        self.vis.text(u"SupInfo:\n",win='SupInfo',opts={'title': 'SupInfo'})

    def text2(self,printstep,allepoch,scheduler_step,gamma,start_epoch,optimizer):
        self.vis.text(u"printstep:"+str(printstep),win='MainInfo', append=True)
        self.vis.text(u"allepoch:"+str(allepoch),win='MainInfo',append=True)
        self.vis.text(u"scheduler_step:"+str(scheduler_step),win='MainInfo',append=True)
        self.vis.text(u"gamma:"+str(gamma),win='MainInfo',append=True)
        self.vis.text(u"start_epoch:"+str(start_epoch),win='MainInfo',append=True)
        self.vis.text(u"lr_backbone:"+str(optimizer.state_dict()['param_groups'][0]['lr']),win='MainInfo',append=True)
        self.vis.text(u"lr_neck:"+str(optimizer.state_dict()['param_groups'][1]['lr']),win='MainInfo',append=True)
        self.vis.text(u"lr_head:"+str(optimizer.state_dict()['param_groups'][2]['lr']),win='MainInfo',append=True)

    def text3(self,oneturntime,leaveTimed,leaveTimeh,leaveTimem):
        self.vis.text((str(oneturntime))+"s/iter,leave time="+str(leaveTimed)+":"+str(leaveTimeh)+":"+str(leaveTimem), win='SupInfo',append=True)

    def text4(self,train_loss_temp):
        self.vis.text("  CURRENT LOSS INF IS (total/cls/loc):"+str(train_loss_temp),win='SupInfo',append=True)

    def text5(self,train_loss):
        self.vis.text("ONE EPOCH TRAIN_LOSS VALUE IS (total/cls/loc):"+str(train_loss),win='SupInfo',append=True)

    def line1(self,step,train_loss):
        self.vis.line([train_loss[0]], [step], win='all', opts={'title': 'all'}, update = 'append')
        self.vis.line([train_loss[1]], [step], win='cls', opts={'title': 'cls'}, update = 'append')
        self.vis.line([train_loss[2]], [step], win='reg', opts={'title': 'reg'}, update = 'append')

