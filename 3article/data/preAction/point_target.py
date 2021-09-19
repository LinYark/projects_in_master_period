from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import numpy as np


from data.preAction.bbox import corner2center
from data.preAction.point import Point


class PointTarget:
    def __init__(self,):
        self.points = Point(8, 25, 255//2)
        # self.points31 = Point(8, 31, 255//2)

    def __call__(self, target, size, neg=False):

        cls = -1 * np.ones((size, size), dtype=np.int64)
        delta = np.zeros((4, size, size), dtype=np.float32)

        def select(position, keep_num=16):
            num = position[0].shape[0]
            if num <= keep_num:
                return position, num
            slt = np.arange(num)
            np.random.shuffle(slt)
            slt = slt[:keep_num]
            return tuple(p[slt] for p in position), keep_num

        tcx, tcy, tw, th = corner2center(target)
        points = self.points.points

        if neg:
            neg = np.where(np.square(tcx - points[0]) / np.square(tw / 4) +
                           np.square(tcy - points[1]) / np.square(th / 4) < 1)
            neg, neg_num = select(neg, 16)
            cls[neg] = 0

            return cls, delta

        delta[0] = points[0] - target[0]
        delta[1] = points[1] - target[1]
        delta[2] = target[2] - points[0]
        delta[3] = target[3] - points[1]


        pos = np.where(np.square(tcx - points[0]) / np.square(tw / 4) +
                       np.square(tcy - points[1]) / np.square(th / 4) < 1)
        neg = np.where(np.square(tcx - points[0]) / np.square(tw / 2) +
                       np.square(tcy - points[1]) / np.square(th / 2) > 1)
        
        # sampling
        pos, pos_num = select(pos, 16)
        neg, neg_num = select(neg, 48)

        cls[pos] = 1
        cls[neg] = 0

        return cls, delta
