#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>

// The recipe of GIMP/Artistic Filters/Photocopy:
// http://www.imagemagick.org/discourse-server/viewtopic.php?t=14441
// * Mask radius = radius of pixel neighborhood for intensity comparison
// * Threshold = relative intensity difference which will result in darkening
// * Ramp = amount of relative intensity difference before total black

cv::Mat
performGimpPhotocopyFilter( const cv::Mat& image, int maskRadius, float treshold, float ramp ) {
   int blurRadius = maskRadius / 3;

   cv::Mat gray( image.size(), CV_8U);
   cv::cvtColor( image, gray, CV_BGR2GRAY );

   cv::Mat avgBlur( gray.size(), gray.type() );
   cv::blur( gray, avgBlur, cv::Size(2 * blurRadius + 1, 2 * blurRadius + 1) ); 

   cv::Mat avgMask( gray.size(), gray.type() );
   cv::blur( gray, avgMask, cv::Size(2 * maskRadius + 1, 2 * maskRadius + 1) ); 

   // creating the result per-element, according to the recipe
   cv::Mat pixelIntensity( gray.size(), CV_8U );
   avgBlur.copyTo( pixelIntensity );

   for( int y=0; y < pixelIntensity.rows; y++) {
      for( int x=0; x < pixelIntensity.cols; x++) {
         unsigned char& elem = pixelIntensity.at<unsigned char>( y, x );
         float reldiff = static_cast<float>( avgBlur.at<unsigned char>( y, x ) ) / avgMask.at<unsigned char>( y, x );
         if ( reldiff < treshold ) {
            float pixelIntensity = static_cast<float>(elem) * ( ramp - std::min( ramp, ( treshold - reldiff ) ) ) / ramp;
            if ( pixelIntensity >= 255.0 ) {
               elem = 255;
            } else {
               elem = static_cast<unsigned char>( pixelIntensity );
            }
         } else {
            elem = 255;
         }
      }
   }

   return pixelIntensity;
}

int main( int argc, char** argv )
{
   if( argc != 2) {
      std::cout <<"USAGE: " << std::string(basename(argv[0])) << " <filename>" << std::endl;
      return -1;
   }

   cv::Mat image;
   image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

   if( !image.data ) {
      std::cout <<  "Cannot not open image" << std::endl;
      return -1;
   }

   cv::Mat photocopy = performGimpPhotocopyFilter( image, 20, 1, 2 );

   cv::namedWindow( "Original", cv::WINDOW_NORMAL );
   cv::namedWindow( "Photocopy", cv::WINDOW_NORMAL );
   cv::imshow( "Original", image );
   cv::imshow( "Photocopy", photocopy );

   cv::waitKey(0);
   return 0;
}
