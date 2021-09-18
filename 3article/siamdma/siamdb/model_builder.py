
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import torch
import torch.nn as nn
import torch.nn.functional as F

import os 
import sys 
sys.path.append(os.getcwd())

from siamdb.loss.loss import select_cross_entropy_loss, select_iou_loss
from siamdb.resnet import resnet18,resnet34
from siamdb.resnet_atrous import resnet50
from siamdb.head import MultiBAN,singelBAN
from siamdb.neck import AdjustAllLayer
from siamdb.util.init_weight import init_weights,init_weights1
from siamdb.new.attention import  allFeatureEnhance
from siamdb.new.mix import  Zmix,Xmixmask,commonwash,mix

import numpy as np
import cv2

class ModelBuilder(nn.Module):
    def __init__(self):
        super(ModelBuilder, self).__init__()

        self.vbackbone = resnet50(used_layers = [2,3])
        self.ibackbone = resnet50(used_layers = [2,3])

        self.vneck = AdjustAllLayer([512,1024],[256,256])
        self.ineck = AdjustAllLayer([512,1024],[256,256])

        self.enhance = allFeatureEnhance([256,256],[256,256])

        self.rgbhead = singelBAN([256,256],2,True)
        self.thead = singelBAN( [256,256],2,True)

        self.Zmix = Zmix([256,256],[256,256])
        self.Xmixmask = Xmixmask([256,256],[256,256])      

        self.head = MultiBAN( [512,512],2,True)

        # self.vbackbone = resnet50(used_layers = [2,4])
        # self.ibackbone = resnet50(used_layers = [2,4])

        # self.vneck = AdjustAllLayer([512,2048],[256,256])
        # self.ineck = AdjustAllLayer([512,2048],[256,256])

        # self.enhance = allFeatureEnhance([256,256],[256,256])

        # self.rgbhead = singelBAN([256,256],2,True)
        # self.thead = singelBAN( [256,256],2,True)

        # self.Zmix = Zmix([256,256],[256,256])
        # self.Xmixmask = Xmixmask([256,256],[256,256])      

        # self.head = MultiBAN( [512,512],2,True)


        # init_weights1(self.backbone)
        # init_weights1(self.neck)
        # init_weights1(self.head)


    def template(self, z,iz):

        vzf = self.vbackbone(z)
        izf = self.ibackbone(iz)
        vzf,izf = self.vneck(vzf),self.ineck(izf)
        self.vzf = vzf
        self.izf  = izf

    def templateupdate(self, zrgb,zi):
        # rgb

        zf_rgb = self.vbackbone(zrgb)
        zf_ir = self.ibackbone(zi)
        zf_rgb = self.vneck(zf_rgb)
        zf_ir = self.ineck(zf_ir)
        LEN = len(zf_rgb)
        # update use fused features
        self.vzf = [(zf_rgb[i] * 0.1 + self.vzf[i] * 0.9) for i in range(LEN)]
        self.izf = [(zf_ir[i] * 0.1 + self.izf[i] * 0.9) for i in range(LEN)]


    def track(self, x,ix):
        vxf = self.vbackbone(x)
        ixf = self.ibackbone(ix)
        vxf,ixf = self.vneck(vxf),self.ineck(ixf)

        # vzf,izf,vxf,ixf =self.vzf,self.izf,vxf,ixf
        vzf,izf,vxf,ixf = self.enhance(self.vzf,self.izf,vxf,ixf)
        # (vzf,izf),(vxf,ixf) = self.commonwash(vzf,izf),self.commonwash(vxf,ixf)


        vcls = self.rgbhead(vzf, vxf)
        icls = self.thead(izf, ixf)
        # vcls = self.log_softmax(vcls)
        # icls = self.log_softmax(icls)
        vb,vc,vw,vh = vcls.size()
        ib,ic,iw,ih = icls.size()
        vclsmask  = vcls.view(vb,2,-1).permute(0,2,1)
        vclsmask = vclsmask.softmax(-1)[:,:, 1].view(vb,1,vw,vh)#是的，31*31
        iclsmask  = icls.view(ib,2,-1).permute(0,2,1)
        iclsmask= iclsmask.softmax(-1)[:,:, 1].view(ib,1,iw,ih)
        
        zf, xf = self.Zmix(vzf,izf),self.Xmixmask(vxf,ixf,vclsmask,iclsmask)

        cls, loc = self.head(zf, xf)

        # vclsmask1 = vclsmask.cpu().numpy()
        # vclsmask1 = vclsmask1.reshape((25,25))
        # mi ,ma= np.min(vclsmask1),np.max(vclsmask1)
        # x1 = (vclsmask1-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('vclsmask1',x2)
        
        # iclsmask1 = iclsmask.cpu().numpy()
        # iclsmask1 = iclsmask1.reshape((25,25))
        # mi ,ma= np.min(iclsmask1),np.max(iclsmask1)
        # x1 = (iclsmask1-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('iclsmask1',x2)

        # cls1 = cls.cpu().numpy()
        # cls1 = cls1.reshape((25,25))
        # mi ,ma= np.min(cls1),np.max(cls1)
        # x1 = (cls1-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('cls1',x2)


        return {
                'cls': cls,
                'loc': loc
               }

    def log_softmax(self, cls):
        cls = cls.permute(0, 2, 3, 1).contiguous()
        cls = F.log_softmax(cls, dim=3)
        return cls

    def forward(self, data):
        vtemplate,vsearch,itemplate ,isearch= data['vz'].cuda(),data['vx'].cuda(),data['iz'].cuda(),data['ix'].cuda()
        label_cls ,label_loc= data['vcls'].cuda(),data['vloc'].cuda()

        # get feature
        vzf ,vxf = self.vbackbone(vtemplate),self.vbackbone(vsearch)
        izf ,ixf = self.ibackbone(itemplate),self.ibackbone(isearch)

        vzf ,vxf,izf ,ixf   = self.vneck(vzf ),self.vneck(vxf ),self.ineck(izf ),self.ineck(ixf )
        vzf,izf,vxf,ixf = self.enhance(vzf,izf,vxf,ixf)
        # (vzf,izf),(vxf,ixf) = self.commonwash(vzf,izf),self.commonwash(vxf,ixf)

        vcls = self.rgbhead(vzf, vxf)
        icls = self.thead(izf, ixf)
        vcls_l = self.log_softmax(vcls)
        icls_l = self.log_softmax(icls)
        vcls_loss = select_cross_entropy_loss(vcls_l, label_cls)
        icls_loss = select_cross_entropy_loss(icls_l, label_cls)
        vb,vc,vw,vh = vcls.size()
        ib,ic,iw,ih = icls.size()
        vclsmask  = vcls.view(vb,2,-1).permute(0,2,1).softmax(-1)[:,:, 1].view(vb,1,vw,vh)#是的，31*31
        iclsmask  = icls.view(ib,2,-1).permute(0,2,1).softmax(-1)[:,:, 1].view(ib,1,iw,ih)


        zf, xf = self.Zmix(vzf,izf),self.Xmixmask(vxf,ixf,vclsmask.detach(),iclsmask.detach())#.detach()
        cls, loc = self.head(zf, xf)
        cls = self.log_softmax(cls)

        cls_loss = select_cross_entropy_loss(cls, label_cls)
        loc_loss = select_iou_loss(loc, label_loc, label_cls)

        outputs = {}
        outputs['total_loss'] = cls_loss + loc_loss + 0.2*vcls_loss +  0.2*icls_loss
        outputs['cls_loss'] = cls_loss
        outputs['loc_loss'] = loc_loss
        outputs['vcls_loss'] = vcls_loss
        outputs['icls_loss'] = icls_loss
        return outputs



if __name__ == "__main__":
    model = ModelBuilder()
    load_data = torch.load( './pretrain_models/model.pth'  )

    model_dict =  model.state_dict()
    state_dict = {}
    for k,v in load_data['state_dict'].items():
        if 'v'+k in model_dict.keys():
            state_dict['v'+k]=v
        if 'i'+k in model_dict.keys():
            state_dict['i'+k]=v
    model_dict.update(state_dict)
    model.load_state_dict(model_dict)
    model.vbackbone.bn1.bias.data[0] +=1






