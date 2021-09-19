#include "checkmodel.h"
#include "scatterdatamodifier.h"

CheckModel::CheckModel(ScatterDataModifier *mod, QObject *parent) : QAbstractTableModel(parent)
{
    modifier = mod;
}

/*
 * @Brief:  确定表列数
 * @Return: NULL
 */
int CheckModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

/*
 * @Brief:  确定表行数
 * @Return: NULL
 */
int CheckModel::rowCount(const QModelIndex &parent) const
{
    return modifier->workpiece_map().count();
}

/*
 * @Brief:  确定表数据
 * @Param:  index,表索引
 * @Param:  role,数据角色
 * @Return: NULL
 */
QVariant CheckModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()){
        return QVariant();
    }

    if(modifier->workpiece_map().contains(index.row() + 1)){
        auto work_p = modifier->workpiece_map().value(index.row() + 1);
        if(role == Qt::DisplayRole){
            switch(index.column()){
            case 0:
                return work_p->number;
            case 1:
                return work_p->isCharSuccess;
            case 2:
                return work_p->isMatchSuccess;
            case 3:
                return work_p->isDel ? QString("√") : QString("×");
            default:
                return QVariant();
            }
        }
        else if(role == Qt::TextAlignmentRole){
            return Qt::AlignCenter;
        }
    }
    return QVariant();
}

/*
 * @Brief:  确定表头
 * @Param:  section,表字段
 * @Param:  orientation,表字段方向属性
 * @Param:  role,表字段角色
 * @Return: NULL
 */
QVariant CheckModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation==Qt::Horizontal && role==Qt::DisplayRole){
        switch (section) {
        case 0:
            return tr("工件编号");
        case 1:
            return tr("字符检测");
        case 2:
            return tr("匹配结果");
        case 3:
            return tr("是否删除");
        default:
            break;
        }
    }
    return QVariant();
}

/*
 * @Brief:  更新表数据
 * @Return: NULL
 */
void CheckModel::update()
{
    beginResetModel();
    endResetModel();
}



