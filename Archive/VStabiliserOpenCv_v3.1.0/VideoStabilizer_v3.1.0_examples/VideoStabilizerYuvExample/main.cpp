#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>

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
    cr::vsource::Frame srcYuvFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::YUV1);
    cr::vsource::Frame dstYuvFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::YUV1);

    // Create video stabilizer object.
    cr::vstab::VideoStabilizer videoStabilizer;

    // Set video stabilizer parameters.
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::SCALE_FACTOR, 2);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::X_OFFSET_LIMIT, 150);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::Y_OFFSET_LIMIT, 150);

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

        // Copy video frame data from OpenCV image to image.
        memcpy(srcYuvFrame.data, opencvSrcYuvFrame.data, srcYuvFrame.size);

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcYuvFrame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcYuvFrame, dstYuvFrame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Copy image to OpenCV image.
        memcpy(opencvDstYuvFrame.data, dstYuvFrame.data, dstYuvFrame.size);

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
