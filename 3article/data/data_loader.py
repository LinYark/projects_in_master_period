import os 
import sys 
sys.path.append(os.getcwd())


from data.datasetCombine import CombineDataset#,CombineDatasetWithoutBadMake

from torch.utils.data import DataLoader

def build_data_loader(jsonpaths,alltimes =20,rgbt= False,batchsize = 16):   #jsonpaths:[[path,times] ,  ]  eg:[['path1',5   ],] 
    myDataset  =  CombineDataset(jsonpaths,alltimes,rgbt)
    train_loader = DataLoader(myDataset,
                              shuffle=True,
                              batch_size=batchsize,
                              num_workers=11,
                              drop_last=True
                              )      
            
    return train_loader


