# Simple OpenCV cartoonizer

##Introduction

I used GIMP for applying cartoon effect on photos. I found the following recipe once, ages ago:
* Take the black component of the binarized output of **Filters** - **Artistic** - **Photocopy**.
* Go back to the original image and quantize the colors agressively after applying some blur. 
* Combine the both.

Although it is not a very professional approach, just for fun I coded this recipe in OpenCV and built it into a command line C++ app. The details were easy to find.
* http://www.imagemagick.org/discourse-server/viewtopic.php?t=14441
* http://www.pyimagesearch.com/2014/07/07/color-quantization-opencv-using-k-means-clustering/
* http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_ml/py_kmeans/py_kmeans_opencv/py_kmeans_opencv.html
* http://docs.opencv.org/3.1.0/de/d63/kmeans_8cpp-example.html#gsc.tab=0

##Hints 
Example of the command line:

> **cartoonizer testimage.jpg cartoon.png 8 0.5 0.28 8 45**

8 0.5 0.28 are the params of the photocopy. They are not identical to GIMP. I wanted to avoid adding a third parameter because of the binarization, hence I changed them a bit. 8 is mask radius. 0.28 expresses how thick the contours are. I am not aware of such a simple explanation for 0.5.
8 and 45 are the params of the color quantizer. 8 is the blur radius, 45 is the total number of colors on the result. (The quantizer uses 44, one color is reserved for the black contours.)

##Examples
* **testimage.jpg** before / after cartoonization
![testimage.jpg](https://github.com/Lyapunov/simple-cartoonizer/blob/master/testimage.jpg)
![cartoon.png](https://github.com/Lyapunov/simple-cartoonizer/blob/master/cartoon.png)
* **testimage2.jpg** before / after cartoonization
![testimage2.jpg](https://github.com/Lyapunov/simple-cartoonizer/blob/master/testimage2.jpg)
![cartoon2.png](https://github.com/Lyapunov/simple-cartoonizer/blob/master/cartoon2.png)
* **testimage3.jpg** before / after cartoonization
![testimage3.jpg](https://github.com/Lyapunov/simple-cartoonizer/blob/master/testimage3.jpg)
![cartoon3.png](https://github.com/Lyapunov/simple-cartoonizer/blob/master/cartoon3.png)
