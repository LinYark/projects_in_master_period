#ifndef CLIPNETA_H
#define CLIPNETA_H

#include <QObject>

class Mask_Detector;
class character;

/********************************************************
 * @ClassName:  ClipNetA
 * @Brief:      切割线程中的子线程A
 * @date        2020/08/31
 ********************************************************/
class ClipNetA : public QObject
{
    Q_OBJECT
public:
    explicit ClipNetA(QObject *parent = nullptr);

signals:
    void netA_ready(std::vector<cv::Mat> MaskOut,
                    std::vector<std::vector<QPointF>> centerpointout,
                    std::vector<std::vector<std::string>> alltextout
                    );
    void netA_load_ready();

public slots:
    void init();
    void add_one_group(std::vector<cv::Mat> DecImg);

private:
    QThread *thread;
    QMutex mutex;

    Mask_Detector         *Md;
    character             *CCC;
    cv::Mat takeROI(int _index, cv::Mat imgI);
};

#endif // CLIPNETA_H
