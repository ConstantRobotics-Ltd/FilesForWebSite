#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>

// Function to convert YUV to NV12.
void YUV_to_NV12(cv::Mat& yuv, cv::Mat& nv12)
{
    // Init variables.
    int width = yuv.size().width;
    int height = yuv.size().height;

    // Copy gray data.
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            nv12.data[y * width + x] = yuv.data[y * width * 3 + x * 3];
        }
    }

    // Copy UV data.
    int i = height;
    for (int y = 0; y < height; y = y + 2)
    {
        for (int x = 0; x < width; x = x + 2)
        {
            nv12.data[i * width + x] = yuv.data[y * width * 3 + x * 3 + 1];
            nv12.data[i * width + x + 1] = yuv.data[y * width * 3 + x * 3 + 2];
        }
        ++i;
    }
}



// Function to convert NV12 to YUV.
void NV12_to_YUV(cv::Mat& nv12, cv::Mat& yuv)
{
    // Init variables.
    int width = yuv.size().width;
    int height = yuv.size().height;

    // Copy data.
    int i = height;
    for (int y = 0; y < height; y = y + 2)
    {
        for (int x = 0; x < width; x = x + 2)
        {
            // Copy Y data.
            yuv.data[y * width * 3 + x * 3] = nv12.data[y * width + x];
            yuv.data[y * width * 3 + x * 3 + 3] = nv12.data[y * width + x + 1];
            yuv.data[(y + 1) * width * 3 + x * 3] = nv12.data[(y + 1) * width + x];
            yuv.data[(y + 1) * width * 3 + x * 3 + 3] = nv12.data[(y + 1) * width + x + 1];

            // Copy U data.
            yuv.data[y * width * 3 + x * 3 + 1] = nv12.data[i * width + x];
            yuv.data[y * width * 3 + x * 3 + 4] = nv12.data[i * width + x];
            yuv.data[(y + 1) * width * 3 + x * 3 + 1] = nv12.data[i * width + x];
            yuv.data[(y + 1) * width * 3 + x * 3 + 4] = nv12.data[i * width + x];

            // Copy V data.
            yuv.data[y * width * 3 + x * 3 + 2] = nv12.data[i * width + x + 1];
            yuv.data[y * width * 3 + x * 3 + 5] = nv12.data[i * width + x + 1];
            yuv.data[(y + 1) * width * 3 + x * 3 + 2] = nv12.data[i * width + x + 1];
            yuv.data[(y + 1) * width * 3 + x * 3 + 5] = nv12.data[i * width + x + 1];
        }
        ++i;
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
    cv::Mat opencvSrcNv12Frame(cv::Size(frameWidth, frameHeight + frameHeight / 2), CV_8UC1);
    cv::Mat opencvDstNv12Frame(cv::Size(frameWidth, frameHeight + frameHeight / 2), CV_8UC1);
    cr::vsource::Frame srcNv12Frame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::NV12);
    cr::vsource::Frame dstNv12Frame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::NV12);

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
        YUV_to_NV12(opencvSrcYuvFrame, opencvSrcNv12Frame);

        // Copy video frame data from OpenCV image to image.
        memcpy(srcNv12Frame.data, opencvSrcNv12Frame.data, srcNv12Frame.size);

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcNv12Frame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcNv12Frame, dstNv12Frame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Copy image to OpenCV image.
        memcpy(opencvDstNv12Frame.data, dstNv12Frame.data, dstNv12Frame.size);

        // Convert UYVY pixel format to YUV.
        NV12_to_YUV(opencvDstNv12Frame, opencvDstYuvFrame);

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
