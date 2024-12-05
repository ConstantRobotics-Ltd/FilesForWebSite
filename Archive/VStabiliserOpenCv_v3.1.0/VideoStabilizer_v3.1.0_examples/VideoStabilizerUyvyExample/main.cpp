#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>

// Function to convert UYVY format to YUV pixel format.
void UYVY_to_YUV(cv::Mat& src, cv::Mat& dst)
{
    size_t p = 0;
    size_t dst_size = src.size().width * src.size().height * 3;
    for (size_t i = 0; i < dst_size; i = i + 6)
    {
        dst.data[i] = src.data[p + 1];
        dst.data[i + 1] = src.data[p];
        dst.data[i + 2] = src.data[p + 2];
        dst.data[i + 3] = src.data[p + 3];
        dst.data[i + 4] = src.data[p];
        dst.data[i + 5] = src.data[p + 2];
        p = p + 4;
    }
}

// Function to convert YUV to UYVY pixel format.
void YUV_to_UYVY(cv::Mat& src, cv::Mat& dst)
{
    size_t p = 0;
    size_t src_size = src.size().width * src.size().height * 3;
    for (size_t i = 0; i < src_size; i = i + 6)
    {

        dst.data[p + 1] = src.data[i];
        dst.data[p] = src.data[i + 1];
        dst.data[p + 2] = src.data[i + 2];
        dst.data[p + 3] = src.data[i + 3];
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
    cv::Mat opencvSrcUyvyFrame(cv::Size(frameWidth, frameHeight), CV_8UC2);
    cv::Mat opencvDstUyvyFrame(cv::Size(frameWidth, frameHeight), CV_8UC2);
    cr::vsource::Frame srcUyvyFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::UYVY);
    cr::vsource::Frame dstUyvyFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::UYVY);

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
        YUV_to_UYVY(opencvSrcYuvFrame, opencvSrcUyvyFrame);

        // Copy video frame data from OpenCV image to image.
        memcpy(srcUyvyFrame.data, opencvSrcUyvyFrame.data, srcUyvyFrame.size);

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcUyvyFrame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcUyvyFrame, dstUyvyFrame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Copy image to OpenCV image.
        memcpy(opencvDstUyvyFrame.data, dstUyvyFrame.data, dstUyvyFrame.size);

        // Convert UYVY pixel format to YUV.
        UYVY_to_YUV(opencvDstUyvyFrame, opencvDstYuvFrame);

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
