import os
import time
import torch 
import logging
class helpfuncs:
    def __init__(self,eachEpochSize,oneturntime):
        self.eachEpochSize = eachEpochSize
        self.oneturntime = oneturntime
        self.firsttime = time.time()

    def makesnapshotdir(self,):
        modelSavePath = os.getcwd()+'/snapshot'
        if not os.path.exists(modelSavePath):
            os.makedirs(modelSavePath)
    def computetraintime(self,allepoch,current_epoch):
        leaveTimed = (int)((allepoch-current_epoch)*self.eachEpochSize*self.oneturntime//60)//60//24
        leaveTimeh = (int)((allepoch-current_epoch)*self.eachEpochSize*self.oneturntime//60)//60 % 24 
        leaveTimem = (int)((allepoch-current_epoch)*self.eachEpochSize*self.oneturntime//60)%60
        print("--------------------------ETA: ",leaveTimed,leaveTimeh,leaveTimem,"D/H/M.-------------------------------")

    def clock(self,):
        nowtime = time.time()
        costtime = nowtime-self.firsttime
        costd = (int)(costtime//60)//60//24
        costh = (int)(costtime//60)//60 % 24 
        costm = (int)(costtime//60)%60
        print('COST TIME D/H/M: ',costd,'/',costh,'/',costm)
    

class loadmodel():
    def __init__(self,):
        a=1
        
    def loadmodel_new(self,model ,snappath):
        load_data = torch.load( snappath  )
        model.backbone.load_state_dict(load_data['backbone'])
        model.neck.load_state_dict(load_data['neck'])
        model.head.load_state_dict(load_data['head'])  
        next_epoch = 0
        next_lr = 0
        return model,next_epoch  ,next_lr
    def loadmodel_resume(self,model,snappath):
        load_data = torch.load( snappath  )
        model.backbone.load_state_dict(load_data['backbone'])
        model.neck.load_state_dict(load_data['neck'])
        model.head.load_state_dict(load_data['head'])   
        next_epoch = load_data['next_epoch']
        next_lr = load_data['next_lr']
        return model,next_epoch ,next_lr  
    def loadmodel_double_new(self, model):
        #`1`
        load_data = torch.load( './pretrain_models/resnet50.model'  )
        # model.vbackbone.load_state_dict(load_data)
        # model.ibackbone.load_state_dict(load_data)
        # load_data = torch.load( './pretrain_models/model.pth'  )
        model_dict =  model.state_dict()
        state_dict = {}
        for k,v in load_data.items():
            if 'vbackbone.'+k in model_dict.keys():
                state_dict['vbackbone.'+k]=v
            if 'ibackbone.'+k in model_dict.keys():# and k.split('.')[0] != 'neck':
                state_dict['ibackbone.'+k]=v
            # if k in model_dict.keys():
            #     state_dict[k]=v


        model_dict.update(state_dict)
        model.load_state_dict(model_dict)
        
        next_epoch = 0
        next_lr = 0

        #2
        # vload_data = torch.load( './pretrain_models/resnet18.pth'  )
        # iload_data = torch.load( './pretrain_models/resnet18.pth'  )
        # model.vbackbone.load_state_dict(vload_data)
        # model.ibackbone.load_state_dict(iload_data)

        # # load_data = torch.load( './pretrain_models/model.pth'  )
        # # model_dict =  model.state_dict()
        # # state_dict = {}
        # # for k,v in load_data['state_dict'].items():
        # #     if k in model_dict.keys():
        # #         state_dict[k]=v
        # # model_dict.update(state_dict)
        # # model.load_state_dict(model_dict)

        # next_epoch = 0
        # next_lr = 0

        #3
        # # vload_data = torch.load( './pretrain_models/resnet50.model'  )
        # iload_data = torch.load( './pretrain_models/resnet50.model'  )
        # # model.vbackbone.load_state_dict(vload_data)
        # model.ibackbone.load_state_dict(iload_data)
        # load_data = torch.load( './pretrain_models/model.pth'  )
        # model_dict =  model.state_dict()
        # state_dict = {}
        # for k,v in load_data['state_dict'].items():
        #     if 'v'+k in model_dict.keys():
        #         state_dict['v'+k]=v
        #     if k in model_dict.keys():
        #         state_dict[k]=v
        # model_dict.update(state_dict)
        # model.load_state_dict(model_dict)
        # next_epoch = 0
        # next_lr = 0
        return model,next_epoch ,next_lr  

    def loadmodel_double_step(self, model):
        # load_data = torch.load( './snapshot/step1_35.pth'  )
        # model.vbackbone.load_state_dict(load_data['vbackbone'])
        # model.ibackbone.load_state_dict(load_data['ibackbone'])
        # model.vneck.load_state_dict(load_data['vneck'])
        # model.ineck.load_state_dict(load_data['ineck'])
        # model.enhance.load_state_dict(load_data['enhance'])
        # model.mix.load_state_dict(load_data['mix'])
        # model.head.load_state_dict(load_data['head'])  

        load_data = torch.load( "./snapshot/step1_20.pth" )
        model_dict =  model.state_dict()
        state_dict = {k:v for k,v in load_data['state_dict'].items() if k in model_dict.keys()}
        model_dict.update(state_dict)
        model.load_state_dict(model_dict)
        next_epoch = load_data['next_epoch']
        next_lr = load_data['next_lr'] 
        return model,next_epoch ,next_lr  

    def loadmodel_double_resume(self, model,snappath):
        load_data = torch.load( snappath )
        model_dict =  model.state_dict()
        state_dict = {}
        for k,v in load_data['state_dict'].items():
            if k in model_dict.keys():
                state_dict[k]=v
        model_dict.update(state_dict)
        model.load_state_dict(model_dict)
        next_epoch = 0
        next_lr =0
        
        return model,next_epoch ,next_lr  

loadmodelinstance = loadmodel()
    
class log():
    def __init__(self,path):
        infospath= "./infos"
        if not os.path.exists(infospath):
                os.makedirs(infospath)
        path = infospath + '/'+ path
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(level = logging.INFO)
        handler = logging.FileHandler(path)
        handler.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s - %(message)s')#%(asctime)s - %(name)s - %(levelname)s - %(message)s
        handler.setFormatter(formatter)
        self.logger.addHandler(handler)
        self.logger.info("\n\n##############################################")


    def info(self,msg):
        self.logger.info(msg)
 
logger = log('traininfo.txt')   