#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>

// Function to convert YUYV format to YUV pixel format.
void YUYV_to_YUV(cv::Mat& src, cv::Mat& dst)
{
    size_t p = 0;
    size_t dst_size = src.size().width * src.size().height * 3;
    for (size_t i = 0; i < dst_size; i = i + 6)
    {
        dst.data[i] = src.data[p];
        dst.data[i + 1] = src.data[p + 1];
        dst.data[i + 2] = src.data[p + 3];
        dst.data[i + 3] = src.data[p + 2];
        dst.data[i + 4] = src.data[p + 1];
        dst.data[i + 5] = src.data[p + 3];
        p = p + 4;
    }
}

// Function to convert YUV to YUYV pixel format.
void YUV_to_YUYV(cv::Mat& src, cv::Mat& dst)
{
    size_t p = 0;
    size_t src_size = src.size().width * src.size().height * 3;
    for (size_t i = 0; i < src_size; i = i + 6)
    {
        dst.data[p] = src.data[i];
        dst.data[p + 1] = src.data[i + 1];
        dst.data[p + 3] = src.data[i + 2];
        dst.data[p + 2] = src.data[i + 3];
        p = p + 4;
    }
}

// Entry point.
int main(void)
{
    // Init video source.
    cv::VideoCapture cap;
    if (!cap.open("test.mp4"))
        return -1;

    // Get video frame size.
    int frameWidth = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frameHeight = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Init images.
    cv::Mat opencvSrcBgrFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvDstBgrFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvSrcYuvFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvDstYuvFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvSrcYuyvFrame(cv::Size(frameWidth, frameHeight), CV_8UC2);
    cv::Mat opencvDstYuyvFrame(cv::Size(frameWidth, frameHeight), CV_8UC2);
    cr::vsource::Frame srcYuyvFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::YUY2);
    cr::vsource::Frame dstYuyvFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::YUY2);

    // Create video stabilizer object.
    cr::vstab::VideoStabilizer videoStabilizer;

    // Set video stabilizer parameters.
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::SCALE_FACTOR, 2);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::X_OFFSET_LIMIT, 150);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::Y_OFFSET_LIMIT, 150);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::ONLY_2D_STABILIZATION_FLAG, 1);

    // Main loop.
    while (true)
    {
        // Capture next video frame.
        cap >> opencvSrcBgrFrame;
        if (opencvSrcBgrFrame.empty())
        {
            // If we have video we can set initial video position.
            cap.set(cv::CAP_PROP_POS_FRAMES, 1);
            continue;
        }

        // Convert BGR pixel format to YUV.
        cv::cvtColor(opencvSrcBgrFrame, opencvSrcYuvFrame, cv::COLOR_BGR2YUV);

        // Convert YUV pixel format to UYVY.
        YUV_to_YUYV(opencvSrcYuvFrame, opencvSrcYuyvFrame);

        // Copy video frame data from OpenCV image to image.
        memcpy(srcYuyvFrame.data, opencvSrcYuyvFrame.data, srcYuyvFrame.size);

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcYuyvFrame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcYuyvFrame, dstYuyvFrame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Copy image to OpenCV image.
        memcpy(opencvDstYuyvFrame.data, dstYuyvFrame.data, dstYuyvFrame.size);

        // Convert UYVY pixel format to YUV.
        YUYV_to_YUV(opencvDstYuyvFrame, opencvDstYuvFrame);

        // Convert YUV pixel fornat to BGR.
        cv::cvtColor(opencvDstYuvFrame, opencvDstBgrFrame, cv::COLOR_YUV2BGR);

        // Show video.
        cv::imshow("SOURCE VIDEO", opencvSrcBgrFrame);
        cv::imshow("RESULT VIDEO", opencvDstBgrFrame);

        // Process keyboard events.
        switch (cv::waitKey(1))
        {
        case 27: // ESC - exit.
            exit(0);
        case 32: // SPACE - reset video stabilizer.
            videoStabilizer.executeCommand(cr::vstab::VideoStabilizerCommand::RESET);
            break;
        }
    }

    return 1;
}
