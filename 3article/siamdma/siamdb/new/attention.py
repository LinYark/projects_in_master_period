import torch
import torch.nn as nn
from torch.nn import functional as F

torch_ver = torch.__version__[:3]

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

class PAM_Calculate(nn.Module):
    def __init__(self, in_dim):                         
        super(PAM_Calculate, self).__init__()
        self.chanel_in = in_dim
        self.query_conv = nn.Conv2d(in_channels=in_dim, out_channels=in_dim // 8, kernel_size=1)

        kaiming_init(self.query_conv)



    def forward(self, x):
        m_batchsize, C, height, width = x.size()
        proj_query = self.query_conv(x).view(m_batchsize, -1, width * height).permute(0, 2, 1)


        return proj_query

class PAM_Use(nn.Module):
    def __init__(self, in_dim):
        super(PAM_Use, self).__init__()
        self.value_conv = nn.Conv2d(in_channels=in_dim, out_channels=in_dim, kernel_size=1)
        self.key_conv = nn.Conv2d(in_channels=in_dim, out_channels=in_dim // 8, kernel_size=1)
        self.softmax = nn.Softmax(dim=-1)

        self.gamma = nn.Parameter(torch.zeros(1))
        kaiming_init(self.value_conv)
        kaiming_init(self.key_conv) 

    def forward(self, x, proj_query):
        m_batchsize, C, height, width = x.size()
        proj_value = self.value_conv(x).view(m_batchsize, -1, width * height)

        proj_key = self.key_conv(x).view(m_batchsize, -1, width * height)
        energy = torch.bmm(proj_query, proj_key)
        attention = self.softmax(energy)

        out = torch.bmm(proj_value, attention.permute(0, 2, 1))
        out = out.view(m_batchsize, C, height, width)

        out = self.gamma * out 
        out = out + x
        return out

class CAM_Calculate(nn.Module):
    def __init__(self, in_dim):
        super(CAM_Calculate, self).__init__()
        self.chanel_in = in_dim
        self.softmax = nn.Softmax(dim=-1)

    def forward(self, x):
        m_batchsize, C, height, width = x.size()
        proj_query = x.contiguous().view(m_batchsize, C, -1)
        proj_key = x.contiguous().view(m_batchsize, C, -1).permute(0, 2, 1)
        energy = torch.bmm(proj_query, proj_key)
        energy_new = torch.max(energy, -1, keepdim=True)[0].expand_as(energy) - energy
        attention = self.softmax(energy_new)

        return attention


class CAM_Use(nn.Module):
    def __init__(self, in_dim):
        super(CAM_Use, self).__init__()
        self.chanel_in = in_dim
        self.gamma = nn.Parameter(torch.zeros(1))

    def forward(self, x, attention):
        m_batchsize, C, height, width = x.size()
        proj_value = x.contiguous().view(m_batchsize, C, -1)
        out = torch.bmm(attention, proj_value)
        out = out.view(m_batchsize, C, height, width)
        out = self.gamma * out 
        out = out + x
        return out



class allFeatureEnhance(nn.Module):
    def __init__(self,
                 v_channels,
                 i_channels,
                 ):
        super(allFeatureEnhance, self).__init__()
        self.listlen = len(v_channels)
        self.v_channels = v_channels[0]
        self.i_channels = i_channels[0]

        self.vpam_cals = nn.ModuleList()
        self.vpam_uses = nn.ModuleList()
        self.vcam_cals = nn.ModuleList()
        self.vcam_uses = nn.ModuleList()

        self.ipam_cals = nn.ModuleList()
        self.ipam_uses = nn.ModuleList()
        self.icam_cals = nn.ModuleList()
        self.icam_uses = nn.ModuleList()

        self.vicam_uses = nn.ModuleList()
        self.vipam_uses = nn.ModuleList()
        self.ivcam_uses = nn.ModuleList()
        self.ivpam_uses = nn.ModuleList()

        for i in range(self.listlen):
            self.vpam_cals.append(PAM_Calculate(self.v_channels))
            self.vpam_uses.append(PAM_Use(self.v_channels))
            self.vcam_cals.append(CAM_Calculate(self.v_channels))
            self.vcam_uses.append(CAM_Use(self.v_channels))

            self.ipam_cals.append(PAM_Calculate(self.i_channels))
            self.ipam_uses.append(PAM_Use(self.i_channels))
            self.icam_cals.append(CAM_Calculate(self.i_channels))
            self.icam_uses.append(CAM_Use(self.i_channels))


            self.vipam_uses.append(PAM_Use(self.v_channels))
            self.ivpam_uses.append(PAM_Use(self.i_channels))

            # self.ivcam_uses.append(CAM_Use(self.in_channels))
            # self.vicam_uses.append(CAM_Use(self.in_channels))


    def forward(self, vzs,izs , vxs,ixs):   #all 
        vzs_out,izs_out , vxs_out,ixs_out  = [],[],[],[]

        for idx, (vz,iz , vx,ix) in enumerate(zip(vzs,izs , vxs,ixs)):

            vz_pattention = self.vpam_cals[idx](vz)
            iz_pattention = self.ipam_cals[idx](iz)
            vx_pattention = self.vpam_cals[idx](vx)
            ix_pattention = self.ipam_cals[idx](ix)

            vz_cattention = self.vcam_cals[idx](vz)
            iz_cattention = self.icam_cals[idx](iz)
            vx_cattention = self.vcam_cals[idx](vx)
            ix_cattention = self.icam_cals[idx](ix)

            vzvz_p_feat = self.vpam_uses[idx](vz, vz_pattention)
            iziz_p_feat = self.ipam_uses[idx](iz, iz_pattention)
            vxvx_p_feat = self.vpam_uses[idx](vx, vx_pattention)
            ixix_p_feat = self.ipam_uses[idx](ix, ix_pattention)

            vzvz_c_feat = self.vcam_uses[idx](vz, vz_cattention)
            iziz_c_feat = self.icam_uses[idx](iz, iz_cattention)
            vxvx_c_feat = self.vcam_uses[idx](vx, vx_cattention)
            ixix_c_feat = self.icam_uses[idx](ix, ix_cattention)

            vzvx_c_feat = self.vcam_uses[idx](vz, vx_cattention)
            izix_c_feat = self.icam_uses[idx](iz, ix_cattention)
            vxvz_c_feat = self.vcam_uses[idx](vx, vz_cattention)
            ixiz_c_feat = self.icam_uses[idx](ix, iz_cattention)

            vziz_p_feat = self.vipam_uses[idx](vz, iz_pattention)
            izvz_p_feat = self.ivpam_uses[idx](iz, vz_pattention)
            vxix_p_feat = self.vipam_uses[idx](vx, ix_pattention)
            ixvx_p_feat = self.ivpam_uses[idx](ix, vx_pattention)

            #neck能不能理解为洗通道，可以就加，不可以就不加，最后看指标就可以了
            # vziz_c_feat = self.vicam_uses[idx](vz, iz_cattention)
            # izvz_c_feat = self.ivcam_uses[idx](iz, vz_cattention)
            # vxix_c_feat = self.vicam_uses[idx](vx, ix_cattention)
            # ixvx_c_feat = self.ivcam_uses[idx](ix, vx_cattention)

            vz_out=  vzvz_p_feat + vzvz_c_feat + vziz_p_feat  + vzvx_c_feat # + vziz_c_feat
            iz_out=  izvz_p_feat + iziz_c_feat + iziz_p_feat  + izix_c_feat # + izvz_c_feat
            vx_out=  vxvx_p_feat + vxvx_c_feat + vxix_p_feat  + vxvz_c_feat # + vxix_c_feat
            ix_out=  ixvx_p_feat + ixix_c_feat + ixix_p_feat  + ixiz_c_feat # + ixvx_c_feat

            vzs_out.append(vz_out)
            izs_out.append(iz_out)
            vxs_out.append(vx_out)
            ixs_out.append(ix_out)

        return vzs_out,izs_out , vxs_out,ixs_out 


