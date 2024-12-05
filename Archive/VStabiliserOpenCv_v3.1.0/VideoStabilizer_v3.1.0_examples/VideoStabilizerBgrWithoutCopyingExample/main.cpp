#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>

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
    cv::Mat opencvSrcFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvDstFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);

    cr::vsource::Frame srcFrame;
    srcFrame.fourcc = (uint32_t)cr::vsource::ValidFourccCodes::BGR24;
    srcFrame.width = frameWidth;
    srcFrame.height = frameHeight;
    srcFrame.size = frameWidth * frameHeight * 3;
    srcFrame.data = opencvSrcFrame.data;

    cr::vsource::Frame dstFrame;
    dstFrame.fourcc = (uint32_t)cr::vsource::ValidFourccCodes::BGR24;
    dstFrame.width = frameWidth;
    dstFrame.height = frameHeight;
    dstFrame.size = frameWidth * frameHeight * 3;
    dstFrame.data = opencvDstFrame.data;

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
        cap >> opencvSrcFrame;
        if (opencvSrcFrame.empty())
        {
            // If we have video we can set initial video position.
            cap.set(cv::CAP_PROP_POS_FRAMES, 1);
            continue;
        }

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcFrame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcFrame, dstFrame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Show video.
        cv::imshow("SOURCE VIDEO", opencvSrcFrame);
        cv::imshow("RESULT VIDEO", opencvDstFrame);

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
