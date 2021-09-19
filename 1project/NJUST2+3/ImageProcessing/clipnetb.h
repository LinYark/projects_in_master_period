#ifndef CLIPNETB_H
#define CLIPNETB_H

#include <QObject>

class Weld_Processing;

/********************************************************
 * @ClassName:  ClipNetB
 * @Brief:      切割线程中的子线程B
 * @date        2020/08/31
 ********************************************************/
class ClipNetB : public QObject
{
    Q_OBJECT
public:
    explicit ClipNetB(QObject *parent = nullptr);

signals:
    void netB_ready(std::vector<cv::Mat> WeldOut);
    void netB_load_ready();

public slots:
    void init();
    void add_one_group(std::vector<cv::Mat> DecImg);

private:
    QThread *thread;
    QMutex mutex;
    Weld_Processing       *WP;

};

#endif // CLIPNETB_H
