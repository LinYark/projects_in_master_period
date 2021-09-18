from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import torch
import torch.nn as nn
from siamdb.new.conv_module import ConvModule


def kaiming_init(module,
                 a=0,
                 mode='fan_out',
                 nonlinearity='relu',
                 bias=0,
                 distribution='normal'):
    assert distribution in ['uniform', 'normal']
    if distribution == 'uniform':
        nn.init.kaiming_uniform_(
            module.weight, a=a, mode=mode, nonlinearity=nonlinearity)
    else:
        nn.init.kaiming_normal_(
            module.weight, a=a, mode=mode, nonlinearity=nonlinearity)
    if hasattr(module, 'bias') and module.bias is not None:
        nn.init.constant_(module.bias, bias)

class washkernel(nn.Module):
    def __init__(self, in_channels, out_channels):
        super(washkernel, self).__init__()
        self.onexone = nn.Sequential(
            nn.Conv2d(in_channels, out_channels, kernel_size=1, bias=False),
            )

    def forward(self, x):
        x = self.onexone(x)
        return x

class commonwash(nn.Module):
    def __init__(self, v_in_channels, v_out_channels,i_in_channels, i_out_channels):
        super(commonwash, self).__init__()
        self.vwashs = nn.ModuleList()
        self.iwashs =  nn.ModuleList()

        for i  in range(3):
            self.vwashs.append(washkernel(v_in_channels, v_out_channels))
            self.iwashs.append(washkernel(i_in_channels, i_out_channels))
    def forward(self, vfeatures,ifeatures):
        vfeatures_out,ifeatures_out = [],[]
        for i in range(3):
            vfea,ifea = vfeatures[i],ifeatures[i]
            vfea,ifea = self.vwashs[i](vfea),self.iwashs[i](ifea)
            vfeatures_out.append(vfea),ifeatures_out.append(ifea)
        return vfeatures_out,ifeatures_out


class washkernel_mask(nn.Module):
    def __init__(self, in_channels, out_channels):
        super(washkernel_mask, self).__init__()
        self.onexone = nn.Sequential(
            nn.Conv2d(in_channels, 32, kernel_size=3,padding=4),
            nn.ReLU(inplace=True),
            nn.Conv2d(32, out_channels, kernel_size=5,padding=2)
            )

    def forward(self, x):
        x = self.onexone(x)
        return x
    


######################################################
class ChannelAttention(nn.Module):
    def __init__(self, in_planes, ratio=2):
        super(ChannelAttention, self).__init__()
        self.avg_pool = nn.AdaptiveAvgPool2d(1)
        self.max_pool = nn.AdaptiveMaxPool2d(1)

        self.fc1 = nn.Conv2d(in_planes,32, 1)
        self.relu1 = nn.ReLU()
        self.fc2 = nn.Conv2d(32, in_planes, 1)

        self.sigmoid = nn.Sigmoid()
        self.gamma = nn.Parameter(torch.zeros(1))

    def forward(self, xin):
        avg_out = self.fc2(self.relu1(self.fc1(self.avg_pool(xin))))
        max_out = self.fc2(self.relu1(self.fc1(self.max_pool(xin))))

        x = avg_out + max_out
        x = self.sigmoid(x)
        # out = self.gamma*(x*xin)+xin

        return x


class SpatialAttention(nn.Module):
    def __init__(self, kernel_size=7):
        super(SpatialAttention, self).__init__()

        assert kernel_size in (3, 7), 'kernel size must be 3 or 7'
        padding = 3 if kernel_size == 7 else 1

        self.conv1 = nn.Conv2d(2, 1, kernel_size, padding=padding) 

        self.gamma = nn.Parameter(torch.zeros(1))
        self.sigmoid = nn.Sigmoid()
   
    def forward(self, xin):
        avg_out = torch.mean(xin, dim=1, keepdim=True)
        max_out, _ = torch.max(xin, dim=1, keepdim=True)
        x = torch.cat([avg_out, max_out], dim=1)

        x = self.conv1(x) 

        x = self.sigmoid(x)
        # out = self.gamma*(x*xin)+xin
        return x
###############################################

import numpy as np
import cv2

class mix(nn.Module):
    def __init__(self, v_channels,i_channels,aim_channels):
        super(mix, self).__init__()
        self.listlen = len(v_channels)
        out_channels = aim_channels[0]+aim_channels[1]    
        v_aimchannel,i_aimchannel = aim_channels[0],aim_channels[1]

        for i in range(self.listlen ):
            self.add_module('vwashkernel'+str(i),washkernel(v_channels[0],v_aimchannel))
        for i in range(self.listlen ):
            self.add_module('iwashkernel'+str(i),washkernel(i_channels[0], i_aimchannel))
        self.washmask = washkernel_mask(1, 1)

        # self.gammav = nn.Parameter(torch.zeros(self.listlen))
        # self.gammai = nn.Parameter(torch.zeros(self.listlen))
        self.zgammac = nn.Parameter(torch.zeros(self.listlen))
        self.xgammac = nn.Parameter(torch.zeros(self.listlen))
        # self.gammas = nn.Parameter(torch.zeros(self.listlen))

        self.zcattn =  ChannelAttention(out_channels)
        self.zsattn = SpatialAttention()

        self.xcattn =  ChannelAttention(out_channels)
        self.xsattn = SpatialAttention()    

    def add(self, vfeatures,ifeatures):
        zfeatures = []
        for  i in range(self.listlen):
            vtemp1,itemp1 = vfeatures[i],ifeatures[i]

            vwashmodule , iwashmodule= getattr(self, 'vwashkernel'+str(i)), getattr(self, 'iwashkernel'+str(i))
            vfea,  ifea = vwashmodule(vtemp1),iwashmodule(itemp1)

            fea =torch.cat((vfea ,ifea),1)

            weight_c_mask= self.zcattn(fea)
            fea = self.zgammac[i]*(fea*weight_c_mask)+ fea
            zfeatures.append(fea)
        return zfeatures

    def addmask(self, vfeatures,ifeatures,vclsmask,iclsmask):
        features = []
        vclsmask_out,iclsmask_out = self.washmask(vclsmask),self.washmask(iclsmask)
        for  i in range(self.listlen ):
            vtemp1,itemp1 = vfeatures[i],ifeatures[i]

            vwashmodule,iwashmodule = getattr(self, 'vwashkernel'+str(i)), getattr(self, 'iwashkernel'+str(i))
            vfea,ifea = vwashmodule(vtemp1),iwashmodule(itemp1)

            vfeamask ,ifeamask = vclsmask_out*vfea,iclsmask_out*ifea
            fea,feamask = torch.cat((vfea ,ifea),1),torch.cat((vfeamask ,ifeamask ),1)

            weight_c_mask= self.xcattn(feamask)
            fea = self.xgammac[i]*(fea*weight_c_mask)+ fea

            # if i ==0:

            #     view_vm = vclsmask_out.cpu().numpy().reshape(31,31)
            #     view_vm = cv2.resize(view_vm,(300,300))
            #     mi ,ma= np.min(view_vm),np.max(view_vm)
            #     x1 = (view_vm-mi)/(ma-mi)*240
            #     x2 = np.array(x1,dtype=np.uint8)
            #     cv2.imshow('view_vm',x2)

            #     view_im = iclsmask_out.cpu().numpy().reshape(31,31)
            #     view_im = cv2.resize(view_im,(300,300))
            #     mi ,ma= np.min(view_im),np.max(view_im)
            #     x1 = (view_im-mi)/(ma-mi)*240
            #     x2 = np.array(x1,dtype=np.uint8)
            #     cv2.imshow('view_im',x2)

            #     # view_s = weight_s_mask.cpu().numpy().reshape(31,31)
            #     # view_s = cv2.resize(view_s,(300,300))
            #     # mi ,ma= np.min(view_s),np.max(view_s)
            #     # x1 = (view_s-mi)/(ma-mi)*240
            #     # x2 = np.array(x1,dtype=np.uint8)
            #     # cv2.imshow('view_s',x2)

            #     view_c = weight_c_mask.cpu().numpy().reshape(1,256)
            #     view_c= view_c.repeat(100,axis=0)
            #     view_c = cv2.resize(view_c,(1200,600))
            #     mi ,ma= np.min(view_c),np.max(view_c)
            #     x1 = (view_c-mi)/(ma-mi)*240
            #     x2 = np.array(x1,dtype=np.uint8)
            #     cv2.imshow('view_c',x2)

            features.append(fea)

        return features
    def forward(self, zv,zi,xv,xi,vclsmask,iclsmask):
        xfeature  = self.addmask(xv,xi,vclsmask,iclsmask)
        zfeature  = self.add(zv,zi)
        return zfeature,xfeature              



class Zmix(nn.Module):
    def __init__(self, v_channels,i_channels):
        super(Zmix, self).__init__()
        self.listlen = len(v_channels)
        out_channels = v_channels[0]+i_channels[0]           

        self.cattn =  ChannelAttention(out_channels)
        self.sattn = SpatialAttention()
        self.gamma = nn.Parameter(torch.zeros(self.listlen*2))
    def add(self, vfeatures,ifeatures):
        features = []
        for  i in range(self.listlen):
            vfea,ifea = vfeatures[i],ifeatures[i]

            fea =torch.cat((vfea ,ifea),1)
        
            weight_c = self.cattn(fea)  
            weight_s = self.sattn(fea)

            fea_out =   self.gamma[i*2] * weight_c*fea + \
                        self.gamma[i*2+1] * weight_s*fea  + \
                        fea
            features.append(fea_out)
        return features

    def forward(self, vfeatures,ifeatures):
        feature  = self.add(vfeatures,ifeatures)
        return feature         


import cv2
import matplotlib.pyplot as plt

class Xmixmask(nn.Module):
    def __init__(self, v_channels,i_channels):
        super(Xmixmask, self).__init__()
        self.listlen = len(v_channels)
        out_channels = v_channels[0]+i_channels[0]          
  
        self.washmask =washkernel_mask(1, 1)      

        self.cattn =  ChannelAttention(out_channels)
        self.sattn = SpatialAttention()
        self.gamma = nn.Parameter(torch.zeros(self.listlen*2))
        self.counti = 1

    def addmask(self, vfeatures,ifeatures,vclsmask,iclsmask):
        features = []
        vclsmask_out = self.washmask(vclsmask)
        iclsmask_out = self.washmask(iclsmask)

        # vclsmask1 = vclsmask_out.cpu().numpy()
        # vclsmask1 = vclsmask1.reshape((31,31))
        # # mi ,ma= np.min(vclsmask1),np.max(vclsmask1)
        # # x1 = (vclsmask1-mi)/(ma-mi)*240
        # # x2 = np.array(x1,dtype=np.uint8)
        # # v2 = cv2.resize(vclsmask1,(300,300))
        # # cv2.imshow('vclsmask1',v2)
        
        # iclsmask1 = iclsmask_out.cpu().numpy()
        # iclsmask1 = iclsmask1.reshape((31,31))
        # # mi ,ma= np.min(iclsmask1),np.max(iclsmask1)
        # # x1 = (iclsmask1-mi)/(ma-mi)*240
        # # x2 = np.array(x1,dtype=np.uint8)
        # # i2 = cv2.resize(iclsmask1,(300,300))
        # # cv2.imshow('iclsmask1',i2)

        # plt.subplot(121)
        # plt.imshow(vclsmask1)
        # plt.title('vclsmask') 
        # plt.subplot(122)
        # plt.imshow(iclsmask1) 
        # plt.title('iclsmask')

        # plt.savefig('./pngs/'+str(self.counti) +'.png')
        # self.counti+=1
        # plt.show()


        for  i in range(self.listlen ):
            vfea,ifea = vfeatures[i],ifeatures[i]

            vmask  = vclsmask_out*vfea
            imask  = iclsmask_out*ifea

            fea_mask =torch.cat((vmask ,imask),1)
            fea = torch.cat((vfea ,ifea),1)
            # feamaskadd = fea_mask + fea
        
            weight_c = self.cattn(fea_mask)  #第一次训，这里用的是add，washmask第二个conv是3，1改成了5，2
            weight_s = self.sattn(fea_mask)

            fea_out =   self.gamma[i*2] * weight_c*fea + \
                        self.gamma[i*2+1] * weight_s*fea  + \
                        fea          
            features.append(fea_out)
        return features

    def forward(self, vfeatures,ifeatures,vclsmask,iclsmask):
        feature  = self.addmask(vfeatures,ifeatures,vclsmask,iclsmask)
        return feature                    

