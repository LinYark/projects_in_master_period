#include "character.h"

character::character()
{
//    ssd_model_file = PP->compute_path().toStdString()+"/wangluo/deployzf32.prototxt";
//    ssd_weights_file = PP->compute_path().toStdString()+"/wangluo/1224_iter_4000.caffemodel";
//    east_model_file = PP->compute_path().toStdString()+"/wangluo/0910_frozen_east_model.pb";
//    txt_file_path =PP->compute_path().toStdString()+ "/wangluo/zifuku.txt";

    confThreshold = 0.8;
    nmsThreshold = 0.2;

    east_inpWidth = 1600; // east网络resize的图片宽度
    east_inpHeight = 960; // east网络resize的图片高度

    increment_width = 30; // east检测框较长边的增量
    increment_height = 0; // east检测框较短边的增量

    lc;

    all_lc;

    confidence_threshold = 0.1;

    Alpha = {'0','1','2','3','4','5','6','7','8','9',
             'A','B','C','D','E','F','G','H','I','J',
             'K','L','M','N','O','P','Q','R','S','T',
             'U','V','W','X','Y','Z','-'};

//    load_ssd();
//    load_east_net();
}

//character detector;


void character::load_east_net()
{
    east_model_file = PP->compute_path().toStdString()+"/wangluo/CHRAC_E_0910.pb";
    qDebug() << "east_model_file:    ";
    txt_file_path =PP->compute_path().toStdString()+ "/wangluo/zifuku.txt";
    qDebug() << "txt_file_path:    " ;
    // Load east network.
    east_net = readNet(east_model_file);
    east_net.setPreferableBackend(DNN_BACKEND_CUDA);
    east_net.setPreferableTarget(DNN_TARGET_CUDA);
}


void character::load_ssd()
{
    ssd_model_file = PP->compute_path().toStdString()+"/wangluo/character.prototxt";
    qDebug() << "ssd_model_file:    " ;
    ssd_weights_file = PP->compute_path().toStdString()+"/wangluo/CHRAC_S_4000.caffemodel";
    qDebug() << "ssd_weights_file:    ";
    cout <<"1111111111111111111111111111"<<endl;
//    east_model_file = PP->compute_path().toStdString()+"/wangluo/0910_frozen_east_model.pb";
//    qDebug() << "east_model_file:    ";
//    txt_file_path =PP->compute_path().toStdString()+ "/wangluo/zifuku.txt";
//    qDebug() << "txt_file_path:    " ;

    const string& mean_file = "";
    const string& mean_value = "104,117,123";
    // Load ssd network.
    qDebug() << "Load ssd network ";
    initializeDetector(ssd_model_file, ssd_weights_file, mean_file, mean_value);
}


vector<string> character::readtxt(string txt_path)
{
    ifstream txt_file;
    txt_file.open(txt_path);

    string temp;
    vector<string> alltemp;
    while(getline(txt_file, temp))
    {
        alltemp.push_back(temp);
    }

    return alltemp;
}


int character::min(int a, int b)
{
    return a < b ? a : b;
}


int character::edit(string str1, string str2)
{
    int max1 = str1.size();
    int max2 = str2.size();

    int **ptr = new int*[max1 + 1];
    for (int i = 0; i < max1 + 1; i++)
    {
        ptr[i] = new int[max2 + 1];
    }

    for (int i = 0; i < max1 + 1; i++)
    {
        ptr[i][0] = i;
    }

    for (int i = 0; i < max2 + 1; i++)
    {
        ptr[0][i] = i;
    }

    for (int i = 1; i < max1 + 1; i++)
    {
        for (int j = 1; j < max2 + 1; j++)
        {
            int d;
            int temp = min(ptr[i - 1][j] + 1, ptr[i][j - 1] + 1);
            if (str1[i - 1] == str2[j - 1])
            {
                d = 0;
            }
            else
            {
                d = 1;
            }
            ptr[i][j] = min(temp, ptr[i - 1][j - 1] + d);
        }
    }

    int dis = ptr[max1][max2];

    for (int i = 0; i < max1 + 1; i++)
    {
        delete[] ptr[i];
        ptr[i] = NULL;
    }

    delete[] ptr;
    ptr = NULL;

    return dis;
}


void character::compare_string(vector<string> &strr1, vector<string> &strr2, vector<Point2f> &cp)
{
    int length = strr1.size();
    int distance[length];
    for(int j = 0; j < strr2.size(); j++)
    {
        for (int i = 0; i < length; i++)
        {
            distance[i] = edit(strr1[i], strr2[j]);
        }
        int min = 0;
        for (int ii = 1; ii < length; ii++)
        {
            if (distance[ii] < distance[min])
            {
                min = ii;
            }
        }
        if (distance[min] <= 5 && distance[min] >= 0)
        {
            strr2[j] = strr1[min];
//            cout << "the string is : " << strr1[min] << endl;
//            cout << strr2[j] << endl;
//            cout << "the distance is : " << distance[min] << endl;
        }
        else
        {
            strr2.erase(strr2.begin() + j);
            cp.erase(cp.begin() + j);
            j--;
        }
    }
}


bool character::cmp(const label_and_centerpoint_and_score &lc1, const label_and_centerpoint_and_score &lc2)
{
    return lc1.centerpoint.x < lc2.centerpoint.x;
}


void character::detectOneImg(Mat img)
{
    // Process one image.
    CHECK(!img.empty())
            << "Error: This frame is empty!";

//    all_lc.clear();
    vector<vector<float>> detections = Detect(img);
    /* Print the detection results. */
    for (int i = 0; i < detections.size(); ++i)
    {
        const vector<float>& d = detections[i];
        // Detection format: [image_id, label, score, xmin, ymin, xmax, ymax].
        CHECK_EQ(d.size(), 7);
        const float score = d[2];
        if (score >= confidence_threshold)
        {
//            cout << setfill('0') << setw(6) << frame_count << " ";
//            cout << static_cast<int>(d[0]) << " ";
//            cout << static_cast<int>(d[1]) << " ";
//            cout << Alpha[static_cast<int>(d[1]) - 1] << " ";
//            cout << score << " " << endl;
//            cout << static_cast<int>(d[3] * img.cols) << " ";
//            cout << static_cast<int>(d[4] * img.rows) << " ";
//            cout << static_cast<int>(d[5] * img.cols) << " ";
//            cout << static_cast<int>(d[6] * img.rows) << endl;

            pt1.x = static_cast<int>(d[3] * img.cols);
            pt1.y = static_cast<int>(d[4] * img.rows);
            pt2.x = static_cast<int>(d[5] * img.cols);
            pt2.y = static_cast<int>(d[6] * img.rows);

            single_character_center_point.x = (pt1.x + pt2.x) / 2;
            single_character_center_point.y = (pt1.y + pt2.y) / 2;

            lc.label = Alpha[static_cast<int>(d[1]) - 1];
            lc.centerpoint = single_character_center_point;
            lc.score = score;
            all_lc.push_back(lc);

            if ((pt2.x-pt1.x) * (pt2.y-pt1.y) < 250000)
            {
                if (score > 0.4)
                {
                    rectangle(img, pt1, pt2, Scalar(0, 0, 255), 1, 1, 0); //用点画矩形窗
                }
            }
        }
    }

//    imshow("Detect", img);
//    waitKey(0);
}


void character::initializeDetector(const string& model_file, const string& weights_file, const string& mean_file, const string& mean_value)
{
    Caffe::SetDevice(0);
    Caffe::set_mode(Caffe::GPU);

    /* Load the network. */
    ssd_net_.reset(new caffe::Net<float>(model_file, caffe::TEST));
    ssd_net_->CopyTrainedLayersFrom(weights_file);

    CHECK_EQ(ssd_net_->num_inputs(), 1)
            << "Network should have exactly one input.";
    CHECK_EQ(ssd_net_->num_outputs(), 1)
            << "Network should have exactly one output.";

    Blob<float>* input_layer = ssd_net_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    CHECK(num_channels_ == 3 || num_channels_ == 1)
        << "Input layer should have 1 or 3 channels.";
    input_geometry_ = Size(input_layer->width(), input_layer->height());

    /* Load the binaryproto mean file. */
    SetMean(mean_file, mean_value);
}


vector<vector<float> > character::Detect(const Mat& img)
{
    Blob<float>* input_layer = ssd_net_->input_blobs()[0];
    input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
    /* Forward dimension change to all layers. */
    ssd_net_->Reshape();

    vector<Mat> input_channels;
    WrapInputLayer(&input_channels);

    Preprocess(img, &input_channels);

    ssd_net_->Forward();

    /* Copy the output layer to a vector */
    Blob<float>* result_blob = ssd_net_->output_blobs()[0];
    const float* result = result_blob->cpu_data();
    const int num_det = result_blob->height();
    vector<vector<float> > detections;
    for (int k = 0; k < num_det; ++k)
    {
        if (result[0] == -1)
        {
            // Skip invalid detection.
            result += 7;
            continue;
        }
        vector<float> detection(result, result + 7);
        detections.push_back(detection);
        result += 7;
    }
    return detections;
}


/* Load the mean file in binaryproto format. */
void character::SetMean(const string& mean_file, const string& mean_value)
{
    Scalar channel_mean;
    if (!mean_file.empty())
    {
        CHECK(mean_value.empty())
                << "Cannot specify mean_file and mean_value at the same time";
        BlobProto blob_proto;
        ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

        /* Convert from BlobProto to Blob<float> */
        Blob<float> mean_blob;
        mean_blob.FromProto(blob_proto);
        CHECK_EQ(mean_blob.channels(), num_channels_)
                << "Number of channels of mean file doesn't match input layer.";

        /* The format of the mean file is planar 32-bit float BGR or grayscale. */
        vector<Mat> channels;
        float* data = mean_blob.mutable_cpu_data();
        for (int i = 0; i < num_channels_; ++i)
        {
            /* Extract an individual channel. */
            Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
            channels.push_back(channel);
            data += mean_blob.height() * mean_blob.width();
        }

        /* Merge the separate channels into a single image. */
        Mat mean;
        merge(channels, mean);

        /* Compute the global mean pixel value and create a mean image
        * filled with this value. */
        channel_mean = cv::mean(mean);
        mean_ = Mat(input_geometry_, mean.type(), channel_mean);
    }
    if (!mean_value.empty())
    {
        CHECK(mean_file.empty())
                << "Cannot specify mean_file and mean_value at the same time";
        stringstream ss(mean_value);
        vector<float> values;
        string item;
        while (getline(ss, item, ','))
        {
            float value = atof(item.c_str());
            values.push_back(value);
        }
        CHECK(values.size() == 1 || values.size() == num_channels_)
                << "Specify either 1 mean_value or as many as channels: " << num_channels_;

        vector<Mat> channels;
        for (int i = 0; i < num_channels_; ++i)
        {
            /* Extract an individual channel. */
            Mat channel(input_geometry_.height, input_geometry_.width, CV_32FC1, Scalar(values[i]));
            channels.push_back(channel);
        }
        merge(channels, mean_);
    }
}


void character::WrapInputLayer(vector<Mat>* input_channels)
{
    Blob<float>* input_layer = ssd_net_->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();
    for (int i = 0; i < input_layer->channels(); ++i)
    {
        Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}


void character::Preprocess(const Mat& img, vector<Mat>* input_channels)
{
    /* Convert the input image to the input image format of the network. */
    Mat sample;
    if (img.channels() == 3 && num_channels_ == 1)
        cvtColor(img, sample, COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
        cvtColor(img, sample, COLOR_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels_ == 3)
        cvtColor(img, sample, COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
        cvtColor(img, sample, COLOR_GRAY2BGR);
    else
        sample = img;

    Mat sample_resized;
    if (sample.size() != input_geometry_)
        resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;

    Mat sample_float;
    if (num_channels_ == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);

    Mat sample_normalized;
    subtract(sample_float, mean_, sample_normalized);

    /* This operation will write the separate BGR planes directly to the
    * input layer of the network because it is wrapped by the Mat
    * objects in input_channels. */
    split(sample_normalized, *input_channels);

    CHECK(reinterpret_cast<float*>(input_channels->at(0).data) == ssd_net_->input_blobs()[0]->cpu_data())
            << "Input channels are not wrapping the input layer of the network.";
}


// 处理检测框
RotatedRect character::DealBox(Point2f ratio, Mat frame, RotatedRect box)
{
    Point2f vertices_0[4];
    box.points(vertices_0);
    for (int j = 0; j < 4; ++j)
    {
        vertices_0[j].x *= ratio.x;
        vertices_0[j].y *= ratio.y;
    }

    Mat black_map = Mat::zeros(frame.size(), CV_8UC3);
    for (int j = 0; j < 4; ++j)
    {
        line(black_map, vertices_0[j], vertices_0[(j + 1) % 4], Scalar(0, 255, 0), 1);
    }
    Mat GrayImage;
    Mat BinaryImage;
    cvtColor(black_map, GrayImage, COLOR_BGR2GRAY);
    threshold(GrayImage, BinaryImage, 100, 255, THRESH_BINARY);
    vector <vector<Point> > contours;
    findContours(BinaryImage, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    drawContours(BinaryImage, contours, -1, Scalar(255, 255, 255), -1, 8);
    Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
    dilate(BinaryImage, BinaryImage, element);
    vector <vector<Point> > outline;
    outline.clear();
    findContours(BinaryImage, outline, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    RotatedRect rect = minAreaRect(outline[0]);
    Size size = rect.size;
    if(size.width >= size.height)
    {
        size.width = size.width + increment_width;
        size.height = size.height + increment_height;
    }
    else
    {
        size.height = size.height + increment_width;
        size.width = size.width + increment_height;
    }
    rect.size = size;

    Point2f vertices[4];
    rect.points(vertices);
    for (int j = 0; j < 4; ++j)
    {
        line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 1);
    }

    return rect;
}


void character::decode(const Mat& scores, const Mat& geometry, float scoreThresh, vector<RotatedRect>& detections, vector<float>& confidences)
{
    detections.clear();
    CV_Assert(scores.dims == 4);                    CV_Assert(geometry.dims == 4);                  CV_Assert(scores.size[0] == 1);
    CV_Assert(geometry.size[0] == 1);               CV_Assert(scores.size[1] == 1);                 CV_Assert(geometry.size[1] == 5);
    CV_Assert(scores.size[2] == geometry.size[2]);  CV_Assert(scores.size[3] == geometry.size[3]);

    const int height = scores.size[2];
    const int width = scores.size[3];
    for (int y = 0; y < height; ++y)
    {
        const float* scoresData = scores.ptr<float>(0, 0, y);
        const float* x0_data = geometry.ptr<float>(0, 0, y);
        const float* x1_data = geometry.ptr<float>(0, 1, y);
        const float* x2_data = geometry.ptr<float>(0, 2, y);
        const float* x3_data = geometry.ptr<float>(0, 3, y);
        const float* anglesData = geometry.ptr<float>(0, 4, y);

        for (int x = 0; x < width; ++x)
        {
            float score = scoresData[x];
            if (score < scoreThresh)
                continue;

            // Decode a prediction.
            // Multiple by 4 because feature maps are 4 time less than input image.
            float offsetX = x * 4.0f;
            float offsetY = y * 4.0f;
            float angle = anglesData[x];
            float cosA = cos(angle);
            float sinA = sin(angle);
            float h = x0_data[x] + x2_data[x];
            float w = x1_data[x] + x3_data[x];

            Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x], offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
            Point2f p1 = Point2f(-sinA * h, -cosA * h) + offset;
            Point2f p3 = Point2f(-cosA * w, sinA * w) + offset;
            RotatedRect r(0.5f * (p1 + p3), Size2f(w, h), -angle * 180.0f / (float)CV_PI);

            detections.push_back(r);
            confidences.push_back(score);
        }
    }
}


float character::getDistance(Point point0, Point point1)
{
    float distance;
    distance = powf((point0.x - point1.x), 2) + powf((point0.y - point1.y), 2);
    distance = sqrtf(distance);

    return distance;
}

void Rotate(const Mat &srcImage,Mat &destImage,double angle)
{
    Point2f center(srcImage.cols/2,srcImage.rows/2);
    Mat M=getRotationMatrix2D(center,angle,1);
    M.at<double>(0,2)+=((srcImage.cols+srcImage.rows)/sqrt(2)-srcImage.cols)/2;
    M.at<double>(1,2)+=((srcImage.cols+srcImage.rows)/sqrt(2)-srcImage.cols)/2;
    warpAffine(srcImage,destImage,M,Size((srcImage.cols+srcImage.rows)/sqrt(2),(srcImage.cols+srcImage.rows)/sqrt(2)));
}

vector<Mat> character::East_crop(vector<Mat> Camera_Img)
{
    //    qDebug()<<"vector<Mat> character::Eastoutput(vector<Mat> Camera_Img)" ;
        vector<Mat> character_srcs;

        for (int k = 0; k < Camera_Img.size(); k++)
        {
            vector<Mat> outs;
            vector<String> outNames(2);
            outNames[0] = "feature_fusion/Conv_7/Sigmoid";
            outNames[1] = "feature_fusion/concat_3";

            Mat srcs;
            srcs = Camera_Img[k];


            Mat frame, blob;
            frame = srcs;
            if (frame.empty())
            {
                cout << "no image" << endl;
            }
            blobFromImage(frame, blob, 1.0, Size(east_inpWidth, east_inpHeight), Scalar(), true, false);
            east_net.setInput(blob);
            east_net.forward(outs, outNames);


            vector<Point2f> centerpoint;
            Mat scores = outs[0];
            Mat geometry = outs[1];
            vector<RotatedRect> boxes;
            vector<float> confidences;
            vector<int> indices;
            decode(scores, geometry, confThreshold, boxes, confidences);
            NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);


            // Render detections.
            Mat src_clone = frame.clone();
            Mat character_src;
            Point2f ratio((float)frame.cols / east_inpWidth, (float)frame.rows / east_inpHeight);


            for (size_t i = 0; i < indices.size(); ++i)
            {
                RotatedRect box = boxes[indices[i]];
                RotatedRect rect = DealBox(ratio, frame, box);
                Point2f center = rect.center; // 矩形中心点坐标
                centerpoint.push_back(center);
                Point2f rect_duandian[4];
                rect.points(rect_duandian);
                Mat rot_mat = getRotationMatrix2D(center, rect.angle, 1.0); // 求旋转矩阵
                Mat rot_image;
                Size dst_sz(src_clone.size());
                warpAffine(src_clone, rot_image, rot_mat, dst_sz); // 原图像旋转
                getRectSubPix(rot_image, rect.size, rect.center, character_src); // getRectSubPix: 提取原图像中字符的区域
                if (character_src.rows > character_src.cols)
                {
                    transpose(character_src, character_src);
                    flip(character_src, character_src, 0);// 旋转90度
                }

                character_srcs.push_back(character_src);
            }
        }

    //    qDebug()<<"return character_srcs" ;
        return character_srcs;
}


vector<Mat> character::Eastoutput(vector<Mat> Camera_Img)
{
    vector<Mat> Camera_Img2;
    vector<Mat> Camera_Img3;
    vector<Mat> character_srcs;
    vector<Mat> character_srcs1;
    vector<Mat> character_srcs2;
    vector<Mat> character_srcs3;
    double angle=-45;
    Rect rect1(0,1006,1920,1200);
    Rect rect2(286,0,1920,1200);

    for(int i=0;i<Camera_Img.size();i++)
    {
        Mat temp_rotate;
        Rotate(Camera_Img[i],temp_rotate,angle);
        Mat roi1=temp_rotate(rect1);
        Mat roi2=temp_rotate(rect2);
        Mat crop1;
        roi1.copyTo(crop1);
        Mat crop2;
        roi2.copyTo(crop2);
        Camera_Img2.push_back(crop1);
        Camera_Img3.push_back(crop2);
    }
    character_srcs1=East_crop(Camera_Img);
    character_srcs2=East_crop(Camera_Img2);
    character_srcs3=East_crop(Camera_Img3);
    for(int i=1;i<character_srcs1.size();i++)
    {
        character_srcs.push_back(character_srcs1[i]);
    }

    for(int i=1;i<character_srcs2.size();i++)
    {
        character_srcs.push_back(character_srcs2[i]);
    }

    for(int i=1;i<character_srcs3.size();i++)
    {
        character_srcs.push_back(character_srcs3[i]);
    }
    return character_srcs;

}

void character::detect_crop(Mat Camera_Img, vector<Mat> &character_srcs, vector<Point2f> &center_point)
{
    vector<Mat> outs;
    vector<String> outNames(2);
    outNames[0] = "feature_fusion/Conv_7/Sigmoid";
    outNames[1] = "feature_fusion/concat_3";

    Mat srcs;
    srcs = Camera_Img;

    Mat frame, blob;
    frame = srcs;
    if (frame.empty())
    {
        cout << "no image" << endl;
    }

    blobFromImage(frame, blob, 1.0, Size(east_inpWidth, east_inpHeight), Scalar(), true, false);
    east_net.setInput(blob);
    east_net.forward(outs, outNames);

    Mat scores = outs[0];
    Mat geometry = outs[1];
    vector<RotatedRect> boxes;
    vector<float> confidences;
    vector<int> indices;
    decode(scores, geometry, confThreshold, boxes, confidences);
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // Render detections.
    Mat src_clone = frame.clone();
    Mat character_src;
    Mat character_src_rot;
//    vector<Mat> character_srcs;
    center_point.clear();
//    duandian_1.clear();
//    duandian_2.clear();
//    duandian_3.clear();
//    duandian_4.clear();
    Point2f ratio((float)frame.cols / east_inpWidth, (float)frame.rows / east_inpHeight);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        RotatedRect box = boxes[indices[i]];
        RotatedRect rect = DealBox(ratio, frame, box);
        Point2f center = rect.center; // 矩形中心点坐标
        center_point.push_back(center);
        center_point.push_back(center);
        Point2f rect_duandian[4];
        rect.points(rect_duandian);

        // 矩形任意对角线的端点坐标
//        Point2f rect_duandian_1 = rect_duandian[0];
//        Point2f rect_duandian_2 = rect_duandian[1];
//        Point2f rect_duandian_3 = rect_duandian[2];
//        Point2f rect_duandian_4 = rect_duandian[3];

        Mat rot_mat = getRotationMatrix2D(center, rect.angle, 1.0); // 求旋转矩阵
        Mat rot_image;
        Size dst_sz(src_clone.size());
        warpAffine(src_clone, rot_image, rot_mat, dst_sz); // 原图像旋转
        getRectSubPix(rot_image, rect.size, rect.center, character_src); // getRectSubPix: 提取原图像中字符的区域
        if (character_src.rows > character_src.cols)
        {
            transpose(character_src, character_src);
            flip(character_src, character_src, 0);// 旋转90度
        }

//        duandian_1.push_back((rect_duandian_1));
//        duandian_2.push_back((rect_duandian_2));
//        duandian_3.push_back((rect_duandian_3));
//        duandian_4.push_back((rect_duandian_4));

        flip(character_src, character_src_rot, -1); // 旋转180度

        character_srcs.push_back(character_src);
        character_srcs.push_back(character_src_rot);
     }


    //     Mat image_resize;
    //     resize(frame, image_resize, Size(1600, 960));

    //     imwrite("/home/fenglin/图片/tmp000/1.jpg",image_resize);

    //     imshow("image_resize", image_resize);
    //     waitKey(0);

}





void character::Chara_Detec(Mat Camera_Img, vector<string> &alltext, vector<Point2f> &center_point)
{
   Mat frame2;
   frame2=Camera_Img;
   double angle=-45;
   Rotate(frame2,frame2,angle);
   Rect rect1(0,1006,1920,1200);
   Rect rect2(286,0,1920,1200);
   Mat roi1=frame2(rect1);
   Mat roi2=frame2(rect2);
   Mat crop1;
   roi1.copyTo(crop1);
   Mat crop2;
   roi2.copyTo(crop2);

   vector<Mat> character_srcs;
   vector<Mat> character_srcs1;
   vector<Mat> character_srcs2;
   vector<Mat> character_srcs3;
   vector<Point2f> center_point1;
   vector<Point2f> center_point2;
   vector<Point2f> center_point3;

   detect_crop(Camera_Img,character_srcs1,center_point1);
   detect_crop(crop1,character_srcs2,center_point2);
   detect_crop(crop2,character_srcs3,center_point3);

   for(int i=0;i<center_point2.size();i++)
   {
       center_point2[i].y=center_point2[i].y+1007;
       float temp_y=center_point2[i].y;
       float temp_x=center_point2[i].x;
       center_point2[i].y=(temp_y-temp_x+1200/sqrt(2))/sqrt(2);
       center_point2[i].x=sqrt(2)*temp_y-center_point2[i].y;
   }

   for(int i=0;i<center_point3.size();i++)
   {
       center_point3[i].y=center_point3[i].y+287;
       float temp_y=center_point3[i].y;
       float temp_x=center_point3[i].x;
       center_point3[i].y=(temp_y-temp_x+1200/sqrt(2))/sqrt(2);
       center_point3[i].x=sqrt(2)*temp_y-center_point3[i].y;
   }

   for(int i=0;i<character_srcs1.size();i++)
   {
      character_srcs.push_back(character_srcs1[i]);
   }

   for(int i=0;i<character_srcs2.size();i++)
   {
      character_srcs.push_back(character_srcs2[i]);
   }

   for(int i=0;i<character_srcs3.size();i++)
   {
      character_srcs.push_back(character_srcs3[i]);
   }

   for(int i=0;i<center_point1.size();i++)
   {
      center_point.push_back(center_point1[i]);
//      cout<<"centerpoint1 x:"<<center_point1[i].x<<" y;"<<center_point1[i].y<<endl;
   }

   for(int i=0;i<center_point2.size();i++)
   {
      center_point.push_back(center_point2[i]);
//      cout<<"centerpoint2 x:"<<center_point2[i].x<<" y;"<<center_point2[i].y<<endl;
   }

   for(int i=0;i<center_point3.size();i++)
   {
      center_point.push_back(center_point3[i]);
//      cout<<"centerpoint3 x:"<<center_point3[i].x<<" y;"<<center_point3[i].y<<endl;
   }

   // ssd识别字符
     string text;
     alltext.clear();
     for(unsigned k = 0; k < character_srcs.size(); k++)
     {
         Mat image = character_srcs[k];
         int add = image.cols - image.rows;
         copyMakeBorder(image, image, 0, add, 0, 0, BORDER_CONSTANT, Scalar(0, 0, 0));

         all_lc.clear();

         detectOneImg(image);

         sort(all_lc.begin(), all_lc.end(), cmp);

         if(all_lc.size() == 0)
         {
             alltext.push_back("0");
             continue;
         }

         for(int i = 0; i < int(all_lc.size()) - 1; i++)
         {
//             qDebug()<<"(all_lc.size()) - 1"<<(all_lc.size()) - 1<<"all_lc.size()"<<all_lc.size()<<'\n';
             float distance = getDistance(all_lc[i + 1].centerpoint, all_lc[i].centerpoint);

             int m = i;

             int hsize = all_lc.size();
             if(hsize == 1)
             {
                 qDebug()<<"123";
             }

             // 去重
//             qDebug()<<"1 all_lc.size()去重:  "<<all_lc.size()<<'\n';

             if(all_lc[m + 1].label == all_lc[m].label)
             {
                 if(distance == 0)
                 {
                     if(all_lc[m + 1].score < 0.5)
                     {
                         all_lc.erase(all_lc.begin() + m + 1);
                         i--;
                     }

                     if(all_lc[m].score < 0.5 && (m == i))
                     {
                         all_lc.erase(all_lc.begin() + m);
                         i--;
                     }
                 }

                 if(distance >= 8)
                 {
                     if((all_lc[m + 1].score) - (all_lc[m].score) > 0.15)
                     {
                         all_lc.erase(all_lc.begin() + m);
                         i--;
                     }

                     if((all_lc[m].score) - (all_lc[m + 1].score) > 0.15 && m == i)
                     {
                         all_lc.erase(all_lc.begin() + m + 1);
                         i--;
                     }

//                     if(abs((all_lc[m].score) - (all_lc[m + 1].score) < 0.15)
//                             && all_lc[m].score < 0.3
//                             && all_lc[m + 1].score < 0.3
//                             && m == i)
//                     {
//                         all_lc.erase(all_lc.begin() + m);
//                         all_lc.erase(all_lc.begin() + m + 1);
//                         i--;
//                     }
                 }

                 if(distance < 8 && distance > 0)
                 {
                     if(all_lc[m + 1].score < 0.75)
                     {
                         all_lc.erase(all_lc.begin() + m + 1);
                         i--;
                     }

                     if(all_lc[m].score < 0.75 && (m == i))
                     {
                         all_lc.erase(all_lc.begin() + m);
                         i--;
                     }
                 }
             }

             // 去除误检框
//             qDebug()<<"1 all_lc.size()去重後:  "<<all_lc.size()<<'\n';

             if(all_lc[m + 1].label != all_lc[m].label)
             {
                 if(distance == 0)
                 {
                     if(all_lc[m + 1].score < 0.5)
                     {
                         all_lc.erase(all_lc.begin() + m + 1);
                         i--;
                     }

                     if(all_lc[m].score < 0.5 && (m == i))
                     {
                         all_lc.erase(all_lc.begin() + m);
                         i--;
                     }
                 }

                 if(distance < 4.5 && distance > 0)
                 {
                     if(all_lc[m + 1].score < all_lc[m].score && all_lc[m + 1].score < 0.5)
                     {
                         all_lc.erase(all_lc.begin() + m + 1);
                         i--;
                     }

                     if(all_lc[m].score < all_lc[m + 1].score
                        && all_lc[m].score < 0.5
                        && m == i)
                     {
                         all_lc.erase(all_lc.begin() + m);
                         i--;
                     }
                 }
             }

         }

//         qDebug()<<"1 all_lc.size()去重去误检後:  "<<all_lc.size()<<'\n';

         text.erase();
         for(int kk = 0; kk < all_lc.size(); kk++)
         {
             text = text + all_lc[kk].label;
         }

         if(count(text.begin(), text.end(), '-') < 2 && text.size() > 4)
         {
             if(text[3] != '-')
             {
                 text.insert(3, "-");
             }
         }

         if(text.size() == 0)
         {
             qDebug()<<"!!!!!!!!!!!!!!!!!"<<'\n';

             text = text + '0';
         }

//         qDebug()<<"text.size():  "<<text.size()<<'\n';

//         cout << text << endl << endl;

         alltext.push_back(text);
     }

//     for(int l = 0; l < alltext.size(); l++)
//     {
//         cout << alltext[l] << endl;
//     }


     AllText = readtxt(txt_file_path); // 读取字符库
     compare_string(AllText, alltext,center_point); // 字符库比对


     for(int l = 0; l < alltext.size(); l++)
     {
         if(alltext[l].empty())
         {
             alltext.erase(alltext.begin()+l);
             l--;
         }
         else
         {
           cout << "比对后 : " << alltext[l] << endl;
         }

     }
}
