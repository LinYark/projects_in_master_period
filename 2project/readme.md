# UAV Locking Strike System无人机定点抛投打击系统
无人机定点抛投打击系统

```
项目简介：
        本系统基于 Linux 系统、Qt 平台，旨在全自动完成对目标无人化远程打击。
        1.无人机携带 TX2 核心板和 Realsense 相机，约定串口协议与无人机飞控通信。
        2.在高处，检测算法提取目标整体，通过检测、跟踪耦合算法锁定打击对象。
        2.1 通过串口收 GPS、姿态信息，计算目标的定位结果，再通过串口发出结果。
        3.在低处，二次检测提取对象的细节部位，通过检测、跟踪耦合算法锁定打击对象。
        3.1 通过 Realsense 相机获取深度图修正定位距离，转发飞控。
项目所需技能：
        Qt、c++、opencv、matlab、深度学习、TensorRT 加速、onnx。
个人工作：
        a.调用 Realsense 深度相机、串口 API，调整搭建无人机系统。
        b.利用 GPS 与姿态角统一至地理坐标系的目标定位算法，与目标出视场的重定位算法。
        c..部署剪枝后的 yolov3 目标检测算法。
        d.基于相关滤波与基于深度学习的双模 SiamDMA 目标跟踪算法。
        e.针对多机多目标的集群分配打击算法。
```

# Directory structure explanation文件结构简要解释
# v132：
最开始构建该项目所使用的代码。可正常使用。
不过缺点有：

1、以一个结构体承载数据传输。这与面向对象编程的指导原则背道而驰。

2、线程的构建依靠c++的thread构建线程，以锁或者flag标志位作为线程的下一次循环条件。这损失了很多cpu资源。

3、最开始一些源码传承于上上届师兄的无人机项目，代码编写仍非常不规范，很多地方的源码耦合度非常高，想动就必须拆，很难保持开闭原则。

# v403:
对1系列的项目的缺点的更改。修改了v132中缺点1，3条。

修改对应条目1：将数据传输改为单类与单类数据传递，避免使用一个庞大的结构体。

修改对应条目3：将能解耦的代码解耦，新增代码尽量闭合源码。

# v301test:

对1系列的项目所有的缺点的更改。修改了v132中缺点1，2，3条。 由于当前该项目正在使用该代码，与师兄老师协商后认为与合作方签署保密协议，故理应保密，非常抱歉我只能放一个很久之前的测试。

修改对应条目1：使用信号与槽替代数据传输。

修改对应条目2：该系列直接使用qt自带的线程qthread，一个类作为一个对象，有需要就将该类moveto一个线程。

修改对应条目3：将能解耦的代码解耦，新增代码尽量闭合源码。

# Processing analysis具体模块流程、原理分析
具体模块流程、原理分析
# Processing1.Flow chart of each part各部分流程图
各部分流程图
<div align="center">
  <img src="demo/processing.png" width="1280px" />
</div>
# Processing2.Underlying algorithm各部分原理图
各部分原理图
<div align="center">
  <img src="demo/alogithm.png" width="1280px" />
</div>
# Processing3.source可获取的更多信息
该项目原理word介绍与演示ppt可以从以下链接获取。

In 2020s, we relied on this project to participate in the research competition, and I participated in writing corresponding detailed technical papers and demonstration ppt.
Specific information can be found [here](https://pan.baidu.com/s/1PuQ9nNzDlcY0jpMucEI9Bw), key is y1za.

当时有依托这个项目参加研电赛，有参与写过相应详细的技术论文和演示ppt。具体百度云链接：
链接：https://pan.baidu.com/s/1PuQ9nNzDlcY0jpMucEI9Bw 
提取码：y1za 


# All steps实际运行流程
The following will illustrate all the steps
以下演示该项目实际运行流程

# step1.photoing拍摄
龙门架在导轨上平移，携带相机拍摄工件

使用的了通信模块
<div align="center">
  <img src="demo/photoing.gif" width="1280px" />
</div>

# step2.Program running程序处理
程序对拍摄的的工件就行处理

使用到了1.图像分割模块、2.轮廓提取模块、3.字符检测模块、4.工件合并模块、5、工件配准模块。
<div align="center">
  <img src="demo/overall processing.gif" width="1280px" />
</div>

# step3.matching配准
工件点云合并好的匹配流程

这是step2中的匹配片段，但具有代表性，故将其独立处理
<div align="center">
  <img src="demo/match.gif" width="1280px" />
</div>

# step4.welding焊接
机械依照程序计算的结果进行焊接

焊接示意
<div align="center">
  <img src="demo/welding.gif" width="1280px" />
</div>

# step5.Usage hint使用场景示意
使用场景示意，龙门架在导轨上扫面整个视场，携带的相机拍摄工件图片，程序处理后返回焊点坐标，龙门机器人依据坐标焊接。
<div align="center">
  <img src="demo/future.gif" width="1280px" />
</div>
