/***********************************************************************
 * Author:       Franziska Krüger
 * Requirements: OpenCV 3
 *
 * Description:
 * ------------
 * Reference Implemenatation of the Depth-Aware Motion Deblurring
 * Algorithm by Xu and Jia.
 * 
 ************************************************************************
*/

#include <iostream>   // cout, cerr, endl
#include <stdexcept>  // throw exception
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace std;
using namespace cv;

namespace DepthAwareDeblurring {

    /**
     * Disparity estimation of two blurred images.
     *  
     */
    void disparityEstimation(const Mat &blurredLeft, const Mat &blurredRight,
                             Mat &disparityMap) {
        // down sample images to roughly reduce blur for disparity estimation
        // (down sampling ratio is 2)
        Mat blurredLeftSmall, blurredRightSmall;

        // because we checked that both images are of the same size
        // the new size is the same for both too
        Size downsampledSize = Size(blurredLeftSmall.cols / 2, blurredLeftSmall.rows / 2);

        // downsample with Gaussian pyramid
        pyrDown(blurredLeft, blurredLeftSmall, downsampledSize);
        pyrDown(blurredRight, blurredRightSmall, downsampledSize);

        imshow("blurred left image", blurredLeftSmall);
        imshow("blurred right image", blurredRightSmall);

        // the following steps aren't exactly the ones from the paper.
        // We will use a different algorithm to obtain the disparity map
        // because this shouldn't have any effect of the overall result,
        // 
        // convert color images to gray images
        cvtColor(blurredLeftSmall, blurredLeftSmall, CV_BGR2GRAY);
        cvtColor(blurredRightSmall, blurredRightSmall, CV_BGR2GRAY);

        // set up stereo block match algorithm
        int ndisparities = 16*5;  
        int SADWindowSize = 21;   
        Ptr<StereoSGBM> sgbm = StereoSGBM::create(-64,   // minimum disparity
                                                  16*12, // Range of disparity
                                                  5,     // Size of the block window. Must be odd
                                                  800,   // P1 disparity smoothness (default: 0)
                                                         // penalty for disparity changes +/- 1
                                                  2400,  // P2 disparity smoothness (default: 0)
                                                         // penalty for disparity changes more than 1
                                                  10,    // disp12MaxDiff (default: 0)
                                                  4,     // preFilterCap (default: 0)
                                                  1,     // uniquenessRatio (default: 0)
                                                  150,   // speckleWindowSize (default: 0, 50-200)
                                                  2);    // speckleRange (default: 0)
                                                  

        sgbm->compute(blurredLeftSmall, blurredRightSmall, disparityMap);

        // check its extreme values
        double minVal; double maxVal;
        minMaxLoc( disparityMap, &minVal, &maxVal );
        cout << "Min disp: " << minVal << " Max value: " << maxVal<< endl;

        // convert disparity map to grayvalues
        disparityMap.convertTo(disparityMap, CV_8UC1, 255/(maxVal - minVal));
        imshow("disparity", disparityMap);
    }

    /**
     * Starts alogrithm with given blurred images as
     * OpenCV matrices
     * 
     */
    void runAlgorithm(const Mat &blurredLeft, const Mat &blurredRight) {
        // check if images have the same size
        if (blurredLeft.cols != blurredRight.cols || blurredLeft.rows != blurredRight.rows) {
            throw runtime_error("ParallelTRDiff::runAlgorithm():Images aren't of same size!");
        }

        // initial disparity estimation of blurred images
        Mat disparityMap;
        disparityEstimation(blurredLeft, blurredRight, disparityMap);
        

        // Wait for a key stroke
        waitKey(0);
    }

    /**
     * Loads images from given filenames and starts the algorithm
     * 
     */
    void runAlgorithm(string filenameLeft, string filenameRight) {
        cout << "loads images..." << endl;

        Mat blurredLeft, blurredRight;
        blurredLeft = imread(filenameLeft, 1);
        blurredRight = imread(filenameRight, 1);

        if (!blurredLeft.data || !blurredRight.data) {
            throw runtime_error("ParallelTRDiff::runAlgorithm():Can not load images!");
        }

        runAlgorithm(blurredLeft, blurredRight);
    }
}