from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import numpy as np

from siamdb.tracker.base_tracker import SiameseTracker
from siamdb.util.bbox import corner2center

import cv2
import cv2
import matplotlib.pyplot as plt

class SiamDBTracker(SiameseTracker):
    def __init__(self, model):
        super(SiamDBTracker, self).__init__()
        self.score_size = (255 - 127) // \
            8 + 1 + 8
        hanning = np.hanning(self.score_size)
        window = np.outer(hanning, hanning)
        self.cls_out_channels = 2
        self.window = window.flatten()
        self.points = self.generate_points(8, self.score_size)
        self.model = model
        self.model.eval()

    def generate_points(self, stride, size):
        ori = - (size // 2) * stride
        x, y = np.meshgrid([ori + stride * dx for dx in np.arange(0, size)],
                           [ori + stride * dy for dy in np.arange(0, size)])
        points = np.zeros((size *   size, 2), dtype=np.float32)
        points[:, 0], points[:, 1] = x.astype(np.float32).flatten(), y.astype(np.float32).flatten()

        return points

    def _convert_bbox(self, delta, point):
        delta = delta.permute(1, 2, 3, 0).contiguous().view(4, -1)
        delta = delta.detach().cpu().numpy()

        delta[0, :] = point[:, 0] - delta[0, :]
        delta[1, :] = point[:, 1] - delta[1, :]
        delta[2, :] = point[:, 0] + delta[2, :]
        delta[3, :] = point[:, 1] + delta[3, :]
        delta[0, :], delta[1, :], delta[2, :], delta[3, :] = corner2center(delta)
        return delta

    def _convert_score(self, score):
        if self.cls_out_channels == 1:
            score = score.permute(1, 2, 3, 0).contiguous().view(-1)
            score = score.sigmoid().detach().cpu().numpy()
        else:
            score = score.permute(1, 2, 3, 0).contiguous().view(self.cls_out_channels, -1).permute(1, 0)
            score = score.softmax(1).detach()[:, 1].cpu().numpy()
        return score

    def _bbox_clip(self, cx, cy, width, height, boundary):
        cx = max(0, min(cx, boundary[1]))
        cy = max(0, min(cy, boundary[0]))
        width = max(10, min(width, boundary[1]))
        height = max(10, min(height, boundary[0]))
        return cx, cy, width, height

    def init(self, vimg, vrect, iimg=0,irect=0):
        """
        args:
            img(np.ndarray): BGR image
            bbox: (x, y, w, h) bbox
        """

        self.center_pos = np.array([vrect[0]+(vrect[2]-1)/2,
                                    vrect[1]+(vrect[3]-1)/2])
        self.size = np.array([vrect[2], vrect[3]])

        # calculate z crop size
        w_z = self.size[0] + 0.5 * np.sum(self.size)
        h_z = self.size[1] + 0.5 * np.sum(self.size)
        s_z = round(np.sqrt(w_z * h_z))

        # calculate channle average
        self.channel_average = np.mean(vimg, axis=(0, 1))

        # get crop
        z_crop = self.get_subwindow(vimg, self.center_pos,
                                    127,
                                    s_z, self.channel_average)


        self.icenter_pos = np.array([irect[0]+(irect[2]-1)/2,
                                    irect[1]+(irect[3]-1)/2])
        # calculate channle average
        self.ichannel_average = np.mean(iimg, axis=(0, 1))
        # get crop
        iz_crop = self.get_subwindow(iimg, self.icenter_pos,
                                    127,
                                    s_z, self.ichannel_average)

        self.model.template(z_crop,iz_crop)



    def track(self, vimg,iimg,sizew=0.50,windoww=0.46,lrw=0.44):
        """
        args:
            img(np.ndarray): BGR image
        return:
            bbox(list):[x, y, width, height]
        """
        w_z = self.size[0] +0.5 * np.sum(self.size)
        h_z = self.size[1] +0.5 * np.sum(self.size)
        s_z = np.sqrt(w_z * h_z)
        scale_z = 127 / s_z
        s_x = s_z * (255 / 127)
        x_crop = self.get_subwindow(vimg, self.center_pos,
                                    255,
                                    round(s_x), self.channel_average)
        ix_crop = self.get_subwindow(iimg, self.icenter_pos,
                                    255,
                                    round(s_x), self.ichannel_average)

        outputs = self.model.track(x_crop,ix_crop)

        score = self._convert_score(outputs['cls'])
        scoreshow = score.reshape((25,25))
        
 

        # plt.subplot(111)
        # plt.imshow(scoreshow)
        # plt.title('vclsmask') 
        # plt.show()


        #########################
        # mi ,ma= np.min(scoreshow),np.max(scoreshow)
        # x1 = (scoreshow-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('scoreshow',x2)
        ########################

        pred_bbox = self._convert_bbox(outputs['loc'], self.points)

        def change(r):
            return np.maximum(r, 1. / r)

        def sz(w, h):
            pad = (w + h) * 0.5
            return np.sqrt((w + pad) * (h + pad))

        # scale penalty
        s_c = change(sz(pred_bbox[2, :], pred_bbox[3, :]) /
                     (sz(self.size[0]*scale_z, self.size[1]*scale_z)))

        # aspect ratio penalty
        r_c = change((self.size[0]/self.size[1]) /
                     (pred_bbox[2, :]/pred_bbox[3, :]))
        penalty = np.exp(-(r_c * s_c - 1) *sizew)
        pscore = penalty * score

        #########################
        # pscoreshow = pscore.reshape((25,25) )
        # mi ,ma= np.min(pscoreshow),np.max(pscoreshow)
        # x1 = (pscoreshow-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('pscoreshow',x2)
        ###########################

        # window penalty
        pscore = pscore * (1 -  windoww) + \
            self.window *  windoww


        #############################
        # windowshow = pscore.reshape((25,25))
        # mi ,ma= np.min(windowshow),np.max(windowshow)
        # x1 = (windowshow-mi)/(ma-mi)*240
        # x2 = np.array(x1,dtype=np.uint8)
        # x2 = cv2.resize(x2,(300,300))
        # cv2.imshow('windowshow',x2)
        ############################

        best_idx = np.argmax(pscore)
        bbox = pred_bbox[:, best_idx] / scale_z
        lr = penalty[best_idx] * score[best_idx] * lrw

        cx = bbox[0] + self.center_pos[0]
        cy = bbox[1] + self.center_pos[1]

        # smooth bbox
        width = self.size[0] * (1 - lr) + bbox[2] * lr
        height = self.size[1] * (1 - lr) + bbox[3] * lr

        # clip boundary
        cx, cy, width, height = self._bbox_clip(cx, cy, width,
                                                height, vimg.shape[:2])

        # udpate state
        nowCenter, nowSize = np.array([cx, cy]),np.array([width, height])
        deltaCenter = nowCenter-self.center_pos
        self.center_pos = self.center_pos + deltaCenter
        self.icenter_pos = self.icenter_pos + deltaCenter
        self.size = nowSize
        # self.center_pos = np.array([cx, cy])
        # self.size = np.array([width, height])

        bbox = [cx - width / 2,
                cy - height / 2,
                width,
                height]
        best_score = score[best_idx]
        return {
                'bbox': bbox,
                'best_score': best_score
               }


    # update
    def update(self, rgb, ir, rect):
        # calculate z crop size
        w_z = self.size[0] + 0.5 * np.sum(self.size)
        h_z = self.size[1] + 0.5 * np.sum(self.size)
        s_z = round(np.sqrt(w_z * h_z))


        channel_average_rgb = np.mean(rgb, axis=(0, 1))

        z_crop_rgb = self.get_subwindow(rgb, self.center_pos,
                                        127,
                                        s_z, channel_average_rgb)


        channel_average_ir = np.mean(ir, axis=(0, 1))
        z_crop_ir = self.get_subwindow(ir, self.icenter_pos,
                                       127,
                                       s_z, channel_average_ir)


        self.model.templateupdate(z_crop_rgb,z_crop_ir)