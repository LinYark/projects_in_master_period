#include"testzbar.h"

Mat src; Mat src_gray;
RNG rng(12345);

Point Center_cal(vector<vector<Point> > contours, int i)
{
    int centerx = 0, centery = 0, n = contours[i].size();
    //在提取的小正方形的边界上每隔周长个像素提取一个点的坐标，
    //求所提取四个点的平均坐标（即为小正方形的大致中心）
    centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
    centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
    Point point1 = Point(centerx, centery);
    return point1;
}

void testzbar()
{
    cout<<"KOBE"<<endl;
    src = imread("/home/fenglin/QT-prograsms/NJUST-Accelerate/test_QR-new/test_QR/t2-20%.bmp", 1);
    Mat src_all = src.clone();

    //框出二维码后，就可以提取出二维码，然后使用解码库zxing，解出码的信息。
    //或者研究二维码的排布规则，自己写解码部分

    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    Mat image;
    Mat imageGray;
    cvtColor(src_all, imageGray, CV_RGB2GRAY);
    int width = imageGray.cols;
    int height = imageGray.rows;
    uchar *raw = (uchar *)imageGray.data;
    Image imageZbar(width, height, "Y800", raw, width * height);
    scanner.scan(imageZbar); //扫描条码
    Image::SymbolIterator symbol = imageZbar.symbol_begin();
    if (imageZbar.symbol_begin() == imageZbar.symbol_end())
    {
        cout << "查询条码失败，请检查图片！" << endl;
    }
    for (; symbol != imageZbar.symbol_end(); ++symbol)
    {
        cout << "类型：" << endl << symbol->get_type_name() << endl << endl;
        cout << "条码：" << endl << symbol->get_data() << endl << endl;
    }
    //imshow("Source Image", image);
    waitKey();
    imageZbar.set_data(NULL, 0);


//    waitKey(0);
//   return(0);
}
