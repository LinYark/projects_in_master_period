import os
import sys
sys.path.append(os.getcwd())
import numpy as np
from torch.optim.lr_scheduler import StepLR,ReduceLROnPlateau
from tqdm import tqdm
import torch 
import random


from  siamdb.model_builder import ModelBuilder
from data.data_loader import build_data_loader#,build_data_loader_WithoutBadMake
from siamdb.optimizer.optimizer_finetune import learnplan,learnstep,modeleval,modeltrain
from util.funcs import helpfuncs,loadmodelinstance,logger


def train():
    allepoch =20
    #构建模型
    model = ModelBuilder().cuda().train()
    # model ,current_epoch,cur_lr = loadmodelinstance.loadmodel_double_resume(model,'./snapshot/step1/step1_24.pth') #,'./snapshot/siamdb_rgbtf_c_11.pth'
    model ,current_epoch,cur_lr = loadmodelinstance.loadmodel_double_resume(model, './fine/step1_21.pth')
    current_epoch,cur_lr  = 1,0.00005
    dbpath = '/opt/data/private/data/DB/'
    jsonpaths1 = [
        # [dbpath+'got10k/self.json',1],   \
        [dbpath+'rgbt234/self.json',50], \
        # [dbpath+'lasot/self.json',2],   \
        [dbpath+'lasher979/self.json',50],
        [dbpath+'gtot/self.json',50],   
        ]
    jsonpaths2 = [[dbpath+'got10kval/self.json',1]]
    jsonpaths3 = [[dbpath+'lasher979/self.json',1]]
    jsonpaths4 = [[dbpath+'lasher245/self.json',1]]


    #dataset
    train_dataloader = build_data_loader(jsonpaths1,alltimes =1,rgbt= True,batchsize =16)
    val_dataloader = build_data_loader(jsonpaths2,alltimes =5,rgbt= True,batchsize = 16)
    val_RGBT234_dataloader = build_data_loader(jsonpaths3,alltimes =5,rgbt= True,batchsize =16)
    val_GTOT_dataloader = build_data_loader(jsonpaths4,alltimes =5,rgbt= True,batchsize = 16)
    #learn_plan
    warmupEpoch,partTrainEpoch = 0,15
    optimizer,ExpLR = learnplan(model,current_epoch,cur_lr,warmupEpoch,partTrainEpoch)
    
    #输出时间、构建snapshot文件夹
    eachEpochSize = len(train_dataloader)
    helpfuncsinstance = helpfuncs(eachEpochSize=eachEpochSize,oneturntime=1/1.7)
    helpfuncsinstance.makesnapshotdir()
    helpfuncsinstance.computetraintime(allepoch,current_epoch)
    # logger = log('traininfo.txt')

    torch.cuda.empty_cache() 

    for epoch in range(current_epoch ,allepoch): 
        cur_lr = optimizer.param_groups[-1]['lr']
        epoch_lr_info = '1.epoch = '+str(epoch) +' . current lr  = '+str(int(cur_lr*1000000)/1000000) #, print('\n')
        print(epoch_lr_info) , logger.info(epoch_lr_info)
        optimizer,ExpLR = learnstep(optimizer,ExpLR, model,epoch,cur_lr,warmupEpoch,partTrainEpoch)
        # ExpLR.step()
        # continue

        train_loss = []
        for i, data in enumerate(train_dataloader):
            outputs = model(data)
            loss = outputs['total_loss']

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()#梯度参数更新

            if eachEpochSize-i<500:
                lossInfo = [ outputs[x].item() for x in outputs  ]
                train_loss.append(lossInfo)


        train_loss = np.mean(train_loss,0)
        train_loss_info = "2. train  mean loss = {}   ".format(train_loss)
        print(train_loss_info)
        logger.info(train_loss_info)


        ExpLR.step()
        if epoch > 0:
            shotpath= "./snapshot/step2"
            if not os.path.exists(shotpath):
                os.makedirs(shotpath)
            torch.save({'state_dict':model.state_dict(),
                        'next_epoch':epoch+1,
                        'next_lr': optimizer.param_groups[-1]['lr']
                        }, shotpath+"/step2_{}.pth".format(epoch))

        # torch.cuda.empty_cache() 
        #eval
        with torch.no_grad():
            modeleval(model)

            val_loss = []
            for i, data in enumerate(val_dataloader):
                outputs = model(data)
                loss = outputs['total_loss']
                lossInfo = [ outputs[x].item() for x in outputs  ]
                val_loss.append(lossInfo)
            val_loss = np.mean(val_loss,0)
            val_loss_info = "3. val  mean loss = {} ".format(val_loss)
            print( val_loss_info),   logger.info(val_loss_info)




            val_rgbt_loss = []
            for i, data in enumerate(val_RGBT234_dataloader):
                outputs = model(data)
                loss = outputs['total_loss']
                lossInfo = [ outputs[x].item() for x in outputs  ]
                val_rgbt_loss.append(lossInfo)
            val_rgbt_loss = np.mean(val_rgbt_loss,0)
            val_rgbt_loss_info = "4. val  mean loss = {}   ".format(val_rgbt_loss)
            print(val_rgbt_loss_info),logger.info(val_rgbt_loss_info)


            val_gtot_loss = []
            for i, data in enumerate(val_GTOT_dataloader):
                outputs = model(data)
                loss = outputs['total_loss']
                lossInfo = [ outputs[x].item() for x in outputs  ]
                val_gtot_loss.append(lossInfo)
            val_gtot_loss = np.mean(val_gtot_loss,0)
            val_gtot_loss_info = "5. val  mean loss = {}   ".format(val_gtot_loss)
            print(val_gtot_loss_info), logger.info(val_gtot_loss_info)


            modeltrain(model,epoch,partTrainEpoch)

        helpfuncsinstance.clock()
        print('\n'),  logger.info('\n')

def seed_torch(seed=0):
    random.seed(seed)
    os.environ['PYTHONHASHSEED'] = str(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.backends.cudnn.benchmark = True
    torch.backends.cudnn.deterministic = True


if __name__ == "__main__":

    seed_torch(123456)
    train()




    
            































