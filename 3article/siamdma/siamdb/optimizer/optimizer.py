

import torch
import torch.nn as nn




def build_opt_lr_no_backbone(model,cur_lr=0):
    for param in model.vbackbone.parameters():
        param.requires_grad = False
    for m in model.vbackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()

    for param in model.ibackbone.parameters():
        param.requires_grad = False
    for m in model.ibackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()


    if cur_lr == 0:
        give_lr = 0.001
    else:
        give_lr = cur_lr

    trainable_params = []
        
    trainable_params += [{'params': model.vneck.parameters(),
                              'lr': give_lr}]
    trainable_params += [{'params': model.ineck.parameters(),
                              'lr': give_lr}]
    trainable_params += [{'params': model.head.parameters(),
                          'lr': give_lr}]
                          
    trainable_params += [{'params': model.Xmixmask.parameters(),
                          'lr': give_lr}]   
    trainable_params += [{'params': model.Zmix.parameters(),
                          'lr': give_lr}]     
    trainable_params += [{'params': model.rgbhead.parameters(),
                          'lr': give_lr}]
    trainable_params += [{'params': model.thead.parameters(),
                          'lr': give_lr}]
    # trainable_params += [{'params': model.commonwash.parameters(),
    #                       'lr': give_lr}]   
                                                    
    trainable_params += [{'params': model.enhance.parameters(),
                              'lr': give_lr}]   
                        
    optimizer = torch.optim.Adam(trainable_params,
                                weight_decay=0.0001)

    return optimizer

def build_opt_lr_with_backbone(model,cur_lr):
    for param in model.vbackbone.parameters():
        param.requires_grad = False
    for m in model.vbackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()
    for layer in ['layer2', 'layer3','layer4']:
        for param in getattr(model.vbackbone, layer).parameters():
            param.requires_grad = True
        for m in getattr(model.vbackbone, layer).modules():
            if isinstance(m, nn.BatchNorm2d):
                m.train()

    for param in model.ibackbone.parameters():
        param.requires_grad = False
    for m in model.ibackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()
    for layer in ['layer2', 'layer3','layer4']:
        for param in getattr(model.ibackbone, layer).parameters():
            param.requires_grad = True
        for m in getattr(model.ibackbone, layer).modules():
            if isinstance(m, nn.BatchNorm2d):
                m.train()


    give_lr = cur_lr
    trainable_params = []
    trainable_params += [{'params': filter(lambda x: x.requires_grad,
                                           model.vbackbone.parameters()),
                          'lr': 0.1 * give_lr}]
    trainable_params += [{'params': model.vneck.parameters(),
                              'lr':give_lr}]

    trainable_params += [{'params': filter(lambda x: x.requires_grad,
                                           model.ibackbone.parameters()),
                          'lr': 0.2 * give_lr}]
    trainable_params += [{'params': model.ineck.parameters(),
                              'lr': give_lr}]

    trainable_params += [{'params': model.head.parameters(),
                          'lr': give_lr}]

    # trainable_params += [{'params': model.commonwash.parameters(),
    #                       'lr': give_lr}]   

    trainable_params += [{'params': model.Xmixmask.parameters(),
                          'lr': give_lr}]   
    trainable_params += [{'params': model.Zmix.parameters(),
                          'lr': give_lr}]   
    trainable_params += [{'params': model.rgbhead.parameters(),
                          'lr': give_lr}]
    trainable_params += [{'params': model.thead.parameters(),
                          'lr': give_lr}]
 
    trainable_params += [{'params': model.enhance.parameters(),
                              'lr': give_lr}]                              


    optimizer = torch.optim.Adam(trainable_params,
                                weight_decay=0.0001)

    return optimizer


def modeleval(model):
    model.eval()
 

def modeltrain(model,curepoch,partTrainEpoch):
    model.train()

    for m in model.vbackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()
    for m in model.ibackbone.modules():
        if isinstance(m, nn.BatchNorm2d):
            m.eval()

    if curepoch >= partTrainEpoch:
        for layer in ['layer2', 'layer3','layer4']:
            for m in getattr(model.vbackbone, layer).modules():
                if isinstance(m, nn.BatchNorm2d):
                    m.train()
        for layer in ['layer2', 'layer3','layer4']:
            for m in getattr(model.ibackbone, layer).modules():
                if isinstance(m, nn.BatchNorm2d):
                    m.train()




def learnplan(model,current_epoch,cur_lr,warmupEpoch,partTrainEpoch):
    if current_epoch<warmupEpoch:
        optimizer = build_opt_lr_no_backbone(model,cur_lr=0)   
        ExpLR = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=1.4)

    elif current_epoch <partTrainEpoch:
        optimizer = build_opt_lr_no_backbone(model,cur_lr) 
        ExpLR = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.85)


    else:
        optimizer = build_opt_lr_with_backbone(model,cur_lr)  
        ExpLR = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.82)

    return optimizer,ExpLR

def learnstep(optimizer,ExpLR, model,current_epoch,cur_lr,warmupEpoch,partTrainEpoch):
    if current_epoch==warmupEpoch:
        optimizer = build_opt_lr_no_backbone(model,cur_lr)
        ExpLR = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.85)


    elif current_epoch==partTrainEpoch:
        optimizer = build_opt_lr_with_backbone(model,cur_lr)    #build_opt_lr_with_backbone
        ExpLR = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.82)

    else: optimizer,ExpLR  =optimizer,ExpLR

    return optimizer,ExpLR



