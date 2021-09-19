#ifndef WORKPIECE_H
#define WORKPIECE_H

#include <QObject>

using namespace QtDataVisualization;

/********************************************************
 * @ClassName:  WorkPiece
 * @Brief:      工件数据模型类
 * @date        2020/08/31
 ********************************************************/
class WorkPiece : public QObject
{
    Q_OBJECT
public:
    explicit WorkPiece(int workpiece_index,                 /*工件编号*/
                       std::string name,                    /*工件对应的字符*/
                       std::vector<cv::Point3d> pcd_mesh,   /*该工件点云数据信息pointCloud*/
                       QObject *parent = nullptr);

    enum STATUS{
        char_failed,        //字符失败(浅红)
        char_success,       //字符成功(青)
        matching,           //开始匹配(青绿闪烁)
        match_success,      //匹配成功(绿)
        match_failed,       //匹配失败(红)
    };


    void update();          //刷新状态
    void match_started();        //开始匹配

    void clear();

    int index() const;

    QScatter3DSeries *series() const;

    QString character() const;
    void setCharacter(const QString &character);

    bool isDel() const;
    void setIsDel(bool isDel);

    bool isCharSuccess() const;
    void setIsCharSuccess(bool isCharSuccess);

    bool isMatchSuccess() const;
    void setIsMatchSuccess(bool isMatchSuccess);

    bool isSelect() const;
    void setIsSelect(bool isSelect);

signals:
    void dateChanged(int i);

public slots:

private:
    int _index;                  //序号
    QScatterDataArray *array;   //数据
    QScatterDataProxy *proxy;   //代理
    QScatter3DSeries *_series;   //序列
    QString _character;          //字符串
    bool _isDel;
    bool _isCharSuccess;
    bool _isMatchSuccess;
    bool _isSelect;

    QString status_text;        //状态文字
    STATUS status;
    QTimer *timer_matching;
};

#endif // WORKPIECE_H
