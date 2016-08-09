#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>

constexpr bool FAST_QUANTIZATION = true;

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
            // float pixelIntensity = static_cast<float>(elem) * ( ramp - std::min( ramp, ( treshold - reldiff ) ) ) / ramp;
            float pixelIntensity = static_cast<float>(elem) * std::max( 0.0f, static_cast<float>( ramp + reldiff - treshold ) / ramp );
            elem = static_cast<unsigned char>( pixelIntensity );
         } else {
            elem = 255;
         }
      }
   }

   return pixelIntensity;
}

cv::Mat
performBinaryGimpPhotocopyMaskFilter( const cv::Mat& image, int maskRadius, float C, float ramp ) {
   if ( maskRadius < 0 ) {
      maskRadius = 0; 
   }
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
         if ( reldiff < C + ramp ) {
            // float pixelIntensity = static_cast<float>(elem) * ( ramp - std::min( ramp, ( treshold - reldiff ) ) ) / ramp;
            float pixelIntensity = static_cast<float>(elem) * std::max( 0.0f, static_cast<float>( reldiff - C ) / ramp );
            elem = pixelIntensity < 128 ? 255 : 0;
         } else {
            elem = 0;
         }
      }
   }

   return pixelIntensity;
}

// http://www.pyimagesearch.com/2014/07/07/color-quantization-opencv-using-k-means-clustering/
// http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_ml/py_kmeans/py_kmeans_opencv/py_kmeans_opencv.html
// http://docs.opencv.org/3.1.0/de/d63/kmeans_8cpp-example.html#gsc.tab=0
cv::Mat
performColorQuantization( const cv::Mat& image, int blurRadius, int colors ) 
{
   cv::Mat gaussianBlur( image.size(), image.type() );
   cv::GaussianBlur( image, gaussianBlur, cv::Size(2 * blurRadius + 1, 2 * blurRadius + 1), 0, 0 ); 
  
   cv::Mat lab( gaussianBlur.size(),  CV_8UC3 );
   cv::cvtColor( gaussianBlur, lab, CV_BGR2Lab );

   cv::Mat floatLab;
   lab.convertTo( floatLab, CV_32FC3 );

   int K = colors;

   cv::Mat labelsFlat;
   cv::Mat centersFloatLabC1;
   cv::Mat points = floatLab.reshape(3, 1);
   // TermCriteria has the default values from the doc
   cv::kmeans(points, K, labelsFlat, cv::TermCriteria( cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centersFloatLabC1);
   cv::Mat centersFloatLabC3 = centersFloatLabC1.reshape( 3, colors );

   // Clumsy way to reconstruct the image from indexed format
   cv::Mat labels = labelsFlat.reshape(1, image.rows );
   cv::Mat centersLab;
   centersFloatLabC3.convertTo( centersLab, CV_8UC3 );
   cv::Mat centers;
   cv::cvtColor( centersLab, centers, CV_Lab2BGR );
   cv::Mat result( image.size(), CV_8UC3 );
   for( int y=0; y < result.rows; y++) {
      for( int x=0; x < result.cols; x++) {
         unsigned char& label = labelsFlat.at<unsigned char>( x +  y * result.cols, 0 );
         result.at<cv::Vec3b>( y, x )[0] = centers.at<cv::Vec3b>( label, 0 )[0];
         result.at<cv::Vec3b>( y, x )[1] = centers.at<cv::Vec3b>( label, 0 )[1];
         result.at<cv::Vec3b>( y, x )[2] = centers.at<cv::Vec3b>( label, 0 )[2];
      }
   }

   return result;
}

// Doing the quantization at most over 500,000 pixels
cv::Mat
performFastColorQuantization( const cv::Mat& image, int blurRadius, int colors ) 
{
   cv::Mat gaussianBlur( image.size(), image.type() );
   cv::GaussianBlur( image, gaussianBlur, cv::Size(2 * blurRadius + 1, 2 * blurRadius + 1), 0, 0 ); 
  
   cv::Mat lab( gaussianBlur.size(),  CV_8UC3 );
   cv::cvtColor( gaussianBlur, lab, CV_BGR2Lab );

   cv::Mat floatLab;
   lab.convertTo( floatLab, CV_32FC3 );

   int K = colors;

   cv::Mat labelsFlat;
   cv::Mat centersFloatLabC1;
   cv::Mat points = floatLab.reshape(3, 1);
   // TermCriteria has the default values from the doc
   cv::kmeans(points, K, labelsFlat, cv::TermCriteria( cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centersFloatLabC1);
   cv::Mat centersFloatLabC3 = centersFloatLabC1.reshape( 3, colors );

   // Clumsy way to reconstruct the image from indexed format
   cv::Mat labels = labelsFlat.reshape(1, image.rows );
   cv::Mat centersLab;
   centersFloatLabC3.convertTo( centersLab, CV_8UC3 );
   cv::Mat centers;
   cv::cvtColor( centersLab, centers, CV_Lab2BGR );
   cv::Mat result( image.size(), CV_8UC3 );
   for( int y=0; y < result.rows; y++) {
      for( int x=0; x < result.cols; x++) {
         unsigned char& label = labelsFlat.at<unsigned char>( x +  y * result.cols, 0 );
         result.at<cv::Vec3b>( y, x )[0] = centers.at<cv::Vec3b>( label, 0 )[0];
         result.at<cv::Vec3b>( y, x )[1] = centers.at<cv::Vec3b>( label, 0 )[1];
         result.at<cv::Vec3b>( y, x )[2] = centers.at<cv::Vec3b>( label, 0 )[2];
      }
   }

   return result;
}

// Facade for cartonization
cv::Mat
performCartoonization( const cv::Mat& image, int photocopyRadius, float photocopyC, float photocopyR, int colorRadius, int numberOfColors )
{
   cv::Mat photocopyMask = performBinaryGimpPhotocopyMaskFilter( image, photocopyRadius, photocopyC, photocopyR );
   cv::Mat result        = FAST_QUANTIZATION
                           ? performFastColorQuantization( image, colorRadius, numberOfColors - 1 )
                           : performColorQuantization( image, colorRadius, numberOfColors - 1 );

   result.setTo( cv::Scalar( 0, 0, 0), photocopyMask ); 
   return result;
}

int main( int argc, char** argv )
{
   const std::string progName = std::string(basename(argv[0]));
   if( argc != 8) {
      std::cout << "USAGE  : " << progName << " <input_file> <output_file> <photocopy_radius> <photocopy_C> <photocopy_R> <color_radius> <number_of_colors>" << std::endl;
      std::cout << "EXAMPLE: " << progName << " testimage.jpg cartoon.png 8 0.5 0.28 8 45" << std::endl;
      return -1;
   }
   int   photocopyRad = atoi( argv[3] );
   float photocopyC   = atof( argv[4] );
   float photocopyR   = atof( argv[5] );
   int   colorRad     = atoi( argv[6] );
   int   colorNum     = atoi( argv[7] );

   // Checking values
   if ( colorNum < 4 ) {
      std::cerr << "Error: Number of colors must be at least 4." << std::endl;
      return -1;
   }

   // Reading the image
   cv::Mat image;
   image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

   if( !image.data ) {
      std::cout <<  "Cannot not open image" << std::endl;
      return -1;
   }

   // Cartoonize!
   cv::Mat cartoon = performCartoonization( image, photocopyRad, photocopyC, photocopyR, colorRad, colorNum );

   // Saving the result
   cv::imwrite( argv[2], cartoon );

/*   cv::namedWindow( "Original", cv::WINDOW_NORMAL );
   cv::namedWindow( "Cartoon", cv::WINDOW_NORMAL );
   cv::imshow( "Original", image );
   cv::imshow( "Cartoon", cartoon );

   cv::waitKey(0);*/
   return 0;
}
