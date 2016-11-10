#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <unistd.h>

void print_usage(const std::string& path_to_exe) {
    auto prog = path_to_exe.substr(path_to_exe.find_last_of('/') + 1); // get basename
    std::cerr << "Usage: \n" << prog << " <videoPath> [-scale=SCALE] [-saveDir=SAVEDIR]\n\n"
              "  Description:\n"
              "      videoPath             File path to the input movie file\n"
              "      scale                 Scale factor of resizing\n"
              "      saveDir               Path to the directory where a result will be saved\n"
              "\n"
              "  Example:\n      "
              << prog << " /home/madoka/video1.avi -scale=2.0 -saveDir=/home/homura/\n"
              << std::endl;
}

bool isExists(const char* path) {
    struct stat st;
    int ret = stat(path, &st);
    return ret == 0;
}

std::string getSavePath(const std::string& videoPath, const std::string& saveDir, const std::string& saveExt) {

    // Set saved file name(it doesn't include the file extension)
    std::string saveName = videoPath.substr(
            videoPath.find_last_of("/")+1,
            videoPath.find_last_of(".")-videoPath.find_last_of("/")-1
    );

    // Get current time
    char buff[100];
    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    sprintf(buff,
            "%04d-%02d-%02d-%02d-%02d-%02d",
            pnow->tm_year + 1900,
            pnow->tm_mon + 1,
            pnow->tm_mday,
            pnow->tm_hour,
            pnow->tm_min,
            pnow->tm_sec
    );

    std::string nowTime = buff;

    std::string savePath = saveDir + "/" + saveName + '-' + nowTime + "." + saveExt;

    return savePath;
}

int main(int argc, char *argv[]) {
    const char* keys =
    {
        "{help h usage |   |print help}"
        "{scale        |2.0|Scale factor of resizing}"
        "{saveDir      |   |Path to the directory where the result will be saved.}"
    };
    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("h")) {
        print_usage(argv[0]);
        return 0;
    }

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse a image path
    std::string videoPath = argv[1];
    if (!isExists(videoPath.c_str())) {
        std::cerr << "Video path does not exist: " << videoPath << std::endl;
        return 1;
    }

    // Parse the scale
    double scale = parser.get<double>("scale");
    if (scale <= 0) {
        std::cerr << "Invalid scale value: " << scale << std::endl;
        return 1;
    }

    // Parse save path
    std::string saveDir = parser.get<std::string>("saveDir");
    if (saveDir.empty()) {
        auto BUF_SIZE = 255;
        char dir[BUF_SIZE];
        getcwd(dir, BUF_SIZE);
        saveDir = dir;
    }
    if (!isExists(saveDir.c_str())) {
        std::cerr << "Save path does not exist: " << videoPath << std::endl;
        return 1;
    }

    // Init cv::VideoCapture
    cv::VideoCapture cap(videoPath);
    if(!cap.isOpened()) {
        std::cerr << "Error: Video file could not be opened." << std::endl;
        return -1;
    }
    // Show video info
    std::cout << (int)cap.get(cv::CAP_PROP_FRAME_WIDTH) << " , " << (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    // Set save path
    const std::string SAVE_EXT = "avi";
    std::string savePath = getSavePath(videoPath, saveDir, SAVE_EXT);

    // Init cv::VideoWriter
    auto writerWidth  = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH)*scale;
    auto writerHeight = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)*scale;
    cv::Size writerSize(writerWidth, writerHeight);
    cv::VideoWriter writer(savePath,
                           static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)),
                           cap.get(cv::CAP_PROP_FPS),
                           writerSize,
                           true);
    if (!writer.isOpened())
    {
        std::cerr << "Error: cv::VideoWriter could not be opened. Please try to another extension of video file." << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::Mat resizedFrame;
    for (;;) {
        // Get a frame from video stream
        cap >> frame;
        if (frame.empty()) break;

        // Resize frame
        cv::resize(frame, resizedFrame, cv::Size(), scale, scale);

        writer << resizedFrame;

        // Show progress
        std::cout << cap.get(CV_CAP_PROP_POS_FRAMES) << " / " << cap.get(CV_CAP_PROP_FRAME_COUNT) << std::endl;
    }
    cap.release();
    writer.release();
    return 0;
}