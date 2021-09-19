import torch.nn as nn


def init_weights(model):
    for i , m in enumerate(model.modules()):
        if isinstance(m, nn.Conv2d):
            nn.init.kaiming_normal_(m.weight.data,
                                    mode='fan_out',
                                    nonlinearity='relu')
        elif isinstance(m, nn.BatchNorm2d):
            m.weight.data.fill_(1)
            m.bias.data.zero_()


def init_weights1(model, gain=1):
    for m in model.modules():
        if isinstance(m, nn.Conv2d):
            nn.init.xavier_uniform_(m.weight, gain)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.BatchNorm2d):
            nn.init.constant_(m.weight, 1)
            nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.Linear):
            nn.init.xavier_uniform_(m.weight, gain)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)