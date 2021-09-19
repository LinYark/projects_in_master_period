#ifndef CHECKMODEL_H
#define CHECKMODEL_H

#include <QAbstractTableModel>

class ScatterDataModifier;

/********************************************************
 * @ClassName:  CheckModel
 * @Brief:      配合CheckDialog类,完成数据表的抽象
 * @date        2020/08/31
 ********************************************************/
class CheckModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CheckModel(ScatterDataModifier *mod, QObject *parent = nullptr);

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void update();

signals:

public slots:

private:
    ScatterDataModifier *modifier;
};

#endif // CHECKMODEL_H
