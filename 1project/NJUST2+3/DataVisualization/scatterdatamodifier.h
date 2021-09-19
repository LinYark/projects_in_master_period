#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtDataVisualization>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>

using namespace QtDataVisualization;
using namespace std;

/********************************************************
 * @ClassName:  ScatterDataModifier
 * @Brief:      3D数据可视化综合管理类,由于过于复杂和冗余,
 *              后被拆分为ScatterViewer和WorkPiece两个类,
 *              但在工程中予以保留
 * @date        2020/08/31
 ********************************************************/
class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatter *scatter);
    ~ScatterDataModifier();

    enum STATUS{
        char_failed             = 0,    //字符失败(浅红)
        char_success            = 1,    //字符成功(浅绿)
        del                     = 2,    //已删除
        match_start             = 3,    //开始匹配
        match_success           = 4,    //匹配成功
        match_failed            = 5,    //匹配失败
        match_pending           = 6,    //待定状态

        //预留
        point_failed            = 10,    //点校验失败(浅红)
        point_success           = 11,    //点校验成功(浅绿)
        weldseam_uncheck        = 12,    //焊缝未校验(浅灰)
        weldseam_checking       = 13,    //焊缝校验中(青)
        weldseam_failed         = 14,    //焊缝校验失败(深红)
        weldseam_success        = 15,    //焊缝校验成功(深绿)
        weldseam_has_been_sent  = 16,    //焊缝已发送
    };

    //工件定义
    struct WorkPiece{
        int number;                 //序号
        QScatterDataArray *array;   //数据
        QScatterDataProxy *proxy;   //代理
        QScatter3DSeries *series;   //序列
        QString character;          //字符串
        bool isDel;
        bool isCharSuccess;
        bool isMatchSuccess;
        int result;                 //结果
        QList<STATUS> result_history;     //结果状态列表
        STATUS status;
        QString status_text;        //状态字符
        void char_check(){
            if(character == "0"){           //匹配失败
                status = char_failed;
            }
            else if(character == "-1"){     //删除
                result = -1;
                status = del;
            }
            else{                           //匹配成功
                result = 1;
                status = char_success;
            }

        }
        void reset(){
            switch (status) {
            case char_failed:
                series->setBaseColor(Qt::red);
                status_text = tr("字符检验失败");
                break;
            case char_success:
                series->setBaseColor(Qt::green);
                status_text = tr("字符检验成功");
                break;
            case del:
                series->setVisible(false);
                status_text = tr("已删除");
                break;
            case match_start:
                series->setBaseColor(Qt::lightGray);
                status_text = tr("开始匹配");
                break;
            case match_success:
                series->setBaseColor(Qt::green);
                status_text = tr("匹配成功");
                break;
            case match_failed:
                series->setBaseColor(Qt::red);
                status_text = tr("匹配失败");
                break;
            case match_pending:
                series->setBaseColor(Qt::cyan);
                status_text = tr("匹配状态待定");
                break;

            case weldseam_uncheck:
                series->setBaseColor(Qt::lightGray);
                status_text = tr("焊缝待检测");
                break;
            case weldseam_checking:
                series->setBaseColor(Qt::cyan);
                status_text = tr("焊缝检测中");
                break;
            case weldseam_failed:
                series->setBaseColor(Qt::darkRed);
                status_text = tr("焊缝检测失败");
                break;
            case weldseam_success:
                series->setBaseColor(Qt::darkGreen);
                status_text = tr("焊缝检测成功");
                break;
            case weldseam_has_been_sent:
                series->setVisible(false);
                status_text = tr("焊缝数据已发送");
                break;
            default:
                break;
            }
            series->setItemLabelFormat(tr("编号:%1 字符:%2 状态:%3").arg(number).arg(character).arg(status_text));
        }

    };

    //数据特性
    void setData1(QVector<QVector4D> mesh,QStringList character_list,QVector<int> result_list);

    void del_current();         //删除当前图形
    QStringList character_list(){
        QStringList list;
        foreach (WorkPiece *p, _workpiece_map) {
            list.append(p->character);
        }
        return list;
    }
    QVector<int> result_list(){
        QVector<int> list;
        foreach (WorkPiece *p, _workpiece_map) {
            list.append(p->result);
        }
        qDebug()<<"result_list"<<list;
        return list;
    }

    void setUnMatched();        //设置未匹配

    bool check_character();     //检验字符校验是否完成


    //图形特性
    void changeStyle();
    void changePresetCamera();
    void changeLabelStyle();

    void changeFontSize(int fontsize);
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setSmoothDots(int smooth);
    void toggleItemCount();

    QString current_character() const;
    void setCurrent_character(QString current_character);

    int current_result() const;
    void setCurrent_result(int current_result);

    int current_number() const;
    void setCurrent_number(int current_number);

    QMap<int, ScatterDataModifier::WorkPiece *> workpiece_map() const;

signals:
    void set_title(QString text);

    void data_changed();

public Q_SLOTS:
    void changeFont(const QFont &font);
    void changeStyle(int style);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);

    void startMatch(int index);      //开始匹配信号
    void matchSuccess(int index);    //匹配成功信号
    void matchFailed(int index);     //匹配失败信号
    void matchPending(int index);    //匹配待定信号

    void add_one_workpiece(int workpiece_index,                 /*工件编号*/
                           std::vector<cv::Point3d> pcd,        /*该工件点云数据信息pointCloud*/
                           std::string name,                    /*工件对应的字符*/
                           std::vector<cv::Point3d> pcd_mesh);
    void match_finished(int workpiece_index, std::string name, bool flag);

Q_SIGNALS:
    void backgroundEnabledChanged(bool enabled);
    void gridEnabledChanged(bool enabled);
    void shadowQualityChanged(int quality);
    void fontChanged(QFont font);

private:
    QVector3D randVector();
    Q3DScatter *m_graph;
    int m_fontSize;
    QAbstract3DSeries::Mesh m_style;
    bool m_smooth;
    int m_itemCount;
    float m_curveDivider;
    QCustom3DLabel *label;

    QVector<QVector4D> mesh;
//    QStringList character_list;
//    QList<int> result_list;




    QMap<int, WorkPiece*> _workpiece_map;

    void data_clear();

    QString _current_character;     //当前字符
    int _current_result;           //当前结果
    int _current_number;            //当前编号


};

#endif
