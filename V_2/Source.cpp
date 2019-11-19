#include <iostream>
#include <chrono>
#include <thread>
#include <opencv2\opencv.hpp>

using namespace std::chrono_literals;
using namespace cv;
using namespace std;



Mat makeMask(Mat image, int upLim[3], int lowLim[3], int eroding) {
	
	if (image.empty()) {
		//cout << "Could no find image" << endl;
	}
	else {


		int h = image.rows;
		int w = image.cols;
		Mat image2;
		cvtColor(image, image2, COLOR_BGR2HSV);
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				Vec3b hsv = image2.at<Vec3b>(y, x);
				float hue = hsv[0];
				float sat = hsv[1];
				float vel = hsv[2];
				//(hue > 22 & hue < 40 & sat > 0 & sat < 100 & * vel > 80 & vel < 255)
				if (upLim[0] < lowLim[0]) {
					if ((hue > lowLim[0] || hue < upLim[0]) & sat > lowLim[1] & sat < upLim[1] & vel > lowLim[2] & vel < upLim[2]) {
						image.at<Vec3b>(y, x) = { 255, 255, 255 };
					}
					else {
						image.at<Vec3b>(y, x) = 0;
					}
				}
				else {
					if (hue > lowLim[0] & hue < upLim[0] & sat > lowLim[1] & sat < upLim[1] & vel > lowLim[2] & vel < upLim[2]) {
						image.at<Vec3b>(y, x) = { 255, 255,  255 };
					}
					else {
						image.at<Vec3b>(y, x) = 0;
					}
				}
			}
		}
		//cvtColor(image, image, COLOR_BGR2HSV);
		//inRange(image, Scalar(lowLim[0], lowLim[1], lowLim[2]), Scalar(upLim[0], upLim[1], upLim[2]), image);
		erode(image, image, Mat(), Point(-1,-1), eroding);
		dilate(image, image, Mat(), Point(-1, -1), 4);
	}
	return image;
}


struct Blob{
	vector < vector<int> > pixels;
};

struct Grid {
	int x_top;
	int y_top;
	int x_butt;
	int y_butt;
	bool found;
};

struct Boat{
	int x;
	int y;
	int size;
	bool direction;
};

bool checkPixel(int x, int y, Mat image) {
	//cout << "Imagesize is: " << image.size() << "Acessing: " << x << " " << y << endl;
	//this_thread::sleep_for(1s);
	//cout << "Checking pixel: " << x << " " << y << endl;
	Vec3b bgr = image.at<Vec3b>(y, x);
	if (bgr[0] > 1) {
		return true;
	}
	return false;
}

bool checkBlob(int x, int y, Blob blob) {
	for (int i = 0; i < blob.pixels.size(); i++) {
		//cout << "Blob x & y: " << blob.pixels.at(i).at(0) << "," << blob.pixels.at(i).at(1) << " Pixel coords: " << x << "," << y << endl;
		if (blob.pixels.at(i).at(0) == x & blob.pixels.at(i).at(1) == y) {
			return false;
		}
	}
	return true;
}

bool checkBlobs(int x, int y, vector<Blob> blobs) {
	for (int i = 0; i < blobs.size(); i++) {
		if (checkBlob(x, y, blobs.at(i)) == false) {
			return true;
		}
	}
	return false;
}

Blob updateBlobData(Blob blob, vector<int> pixel) {
	for (int i = 0; i < blob.pixels.size(); i++) {
		if (pixel.at(0) == blob.pixels.at(i).at(0) & pixel.at(1) == blob.pixels.at(i).at(1)) {
			blob.pixels.at(i) = pixel;
			break;
		}
	}
	return blob;
}

Blob scanBlob(int x, int y, Mat image) {
	Blob blob;
	blob.pixels.insert(blob.pixels.begin(), vector<int> { x,y,1,0 });
	vector<int> activePixel = blob.pixels.at(0);
	bool scanComplete = false;
	int upSearchedRange = 1;
	int lowSearchedRange = 0;
	//cout << "scan completeness " << scanComplete << " how false looks: " << false << endl;
	while (scanComplete == false) {
		//cout << "Entered loop" << endl;
		if (activePixel.at(2) != activePixel.at(3) & activePixel.at(2) < 5) {
			vector<int> pixelOfInterest;
			if (activePixel.at(2) == 1) {
				pixelOfInterest = { activePixel.at(0) + 1, activePixel.at(1), 3 };
				if (pixelOfInterest.at(0) >= image.cols) {
					//cout << "ERROR: out of collums" << endl;
					pixelOfInterest.at(0) = image.cols-1;
				}
			}
			else if(activePixel.at(2) == 2) {
				pixelOfInterest = { activePixel.at(0), activePixel.at(1) + 1, 4 };
				if (pixelOfInterest.at(1) >= image.rows) {
					//cout << "ERROR: Out of rows" << endl;
					pixelOfInterest.at(1) = image.rows-1;
				}
			}
			else if (activePixel.at(2) == 3) {
				pixelOfInterest = { activePixel.at(0) - 1, activePixel.at(1), 1 };
				if (pixelOfInterest.at(0) < 0) {
					pixelOfInterest.at(0) = 0;
				}
			}
			else if (activePixel.at(2) == 4) {
				pixelOfInterest = {activePixel.at(0), activePixel.at(1) - 1, 2};
				if (pixelOfInterest.at(1) < 0) {
					pixelOfInterest.at(1) = 0;
				}
			}
			/*if (pixelOfInterest.empty() == false) {
				cout << "pixel of incest" << pixelOfInterest.at(2) << endl;
			}*/
			activePixel.at(2) += 1;

			//cout << "Vibe check!" << checkPixel(pixelOfInterest.at(0), pixelOfInterest.at(1), image) << checkBlob(pixelOfInterest.at(0), pixelOfInterest.at(1), blob) << endl;

			if (checkPixel(pixelOfInterest.at(0), pixelOfInterest.at(1), image) & checkBlob(pixelOfInterest.at(0), pixelOfInterest.at(1), blob)) {
				blob.pixels.push_back({ pixelOfInterest.at(0), pixelOfInterest.at(1), 1, pixelOfInterest.at(2) });
				//cout << "appened pixel" << pixelOfInterest.at(0) << pixelOfInterest.at(1) << endl;
				activePixel = blob.pixels.at(blob.pixels.size() - 1);
			}
		}
		else if (activePixel.at(2) < 5) {
			activePixel.at(2) += 1;
		}
		//cout << "Active pixel is at: " << activePixel.at(2) << endl;
		if (activePixel.at(2) > 4) {
			//cout << "Finding new active pixel" << endl;
			blob = updateBlobData(blob, activePixel);
			bool noPixelActive = true;
			int i = blob.pixels.size() -1;
			while (noPixelActive == true & scanComplete == false) {
				if (i<upSearchedRange & i>lowSearchedRange) {
					i = lowSearchedRange;
				}
				if (i >= 1) {
					//cout << "Working with i: " << i << endl;
					activePixel = blob.pixels.at(i);
					noPixelActive = (activePixel.at(2) > 4);
					i -= 1;
				}
				else {
					scanComplete = true;
				}
				if (noPixelActive == false) {
					upSearchedRange = blob.pixels.size();
					lowSearchedRange = i;
					//cout << "set new sizes: " << upSearchedRange << " " << lowSearchedRange << endl;
				}
			}
		}
	}
	//cout << "Returning blob of length: " << blob.pixels.size() << endl;
	return blob;
}

vector<Blob> detectBlobs(Mat image, vector<Blob> blobs) {
	int h = image.rows;
	int w = image.cols;
	for (int x = 0; x < w; x += 1) {
		for (int y = 0; y < h; y += 1) {
			if (checkPixel(x, y, image) == true) {
				if (blobs.empty() || checkBlobs(x, y, blobs) == false) {
					//cout << "Starting scan!" << endl;
					blobs.push_back(scanBlob(x, y, image));
				}
			}
		}
	}
	return blobs;
}

vector<int> blobanalysis(Mat img, Blob blobanal){

	int lowestx = blobanal.pixels.at(0).at(0);
	int biggestx = blobanal.pixels.at(0).at(0);
	int lowesty = blobanal.pixels.at(0).at(1);
	int biggesty = blobanal.pixels.at(0).at(1);

	vector<int> coordinates;
	for (int i = 0; i < 4; i++) {
		coordinates.push_back(0);
	}

	for (int j = 0; j < blobanal.pixels.size(); j++) {

		if (blobanal.pixels.at(j).at(0) < coordinates.at(0)) {
			coordinates.at(0) = blobanal.pixels.at(j).at(0);
		}
		if (blobanal.pixels.at(j).at(0) > coordinates.at(1)) {
			coordinates.at(1) = blobanal.pixels.at(j).at(0);
		}
		if (blobanal.pixels.at(j).at(1) < coordinates.at(2)) {
			coordinates.at(0) = blobanal.pixels.at(j).at(1);
		}
		if (blobanal.pixels.at(j).at(1) > coordinates.at(3)) {
			coordinates.at(3) = blobanal.pixels.at(j).at(1);
		}
	}
	//cout << "Found x's: " << biggestx << " " << lowestx << endl;
	//cout << "Found y's: " << biggesty << " " << lowesty << endl;
	/*
	for (int x = lowestx; x < biggestx; x++) {
		img.at<Vec3b>(lowesty, x) = { 0, 0, 255 };
	}
	for (int x = lowestx; x < biggestx; x++) {
		img.at<Vec3b>(biggesty, x) = { 0, 0, 255 };
	}
	for (int y = lowesty; y < biggesty; y++) {
		img.at<Vec3b>(y, lowestx) = { 0, 0, 255 };
	}
	for (int y = lowesty; y < biggesty; y++) {
		img.at<Vec3b>(y, biggestx) = { 0, 0, 255 };
	}
	*/
	return coordinates;
}

void processImage(Mat image, double presision) {
	Mat maskImage;
	imshow("camera1", image);
	int upLim[] = { 90,240,200 };
	int lowLim[] = { 40,50,0 };
	maskImage = makeMask(image, upLim, lowLim, 2);
	imshow("Mask", maskImage);

	resize(maskImage, maskImage, Size(), 1 / presision, 1 / presision);
	/*Mat newMask;
	resize(maskImage, newMask, Size(), 1 * presision, 1 * presision);
	imshow("new mask", newMask);*/
	//cout << "started blob detection" << maskImage.size() << endl;
	
	vector<Blob> blobs;
	blobs = detectBlobs(maskImage, blobs);
	
	//cout << "Detected blobs" << endl;
	/*if (blobs.empty() == false) {
		Mat analImage = maskImage;
		for (int i = 0; i < blobs.size(); i++) {
			analImage = blobanalysis(maskImage, blobs.at(i));
		}
		resize(analImage, analImage, Size(), 1 * presision, 1 * presision);
		imshow("blobAnal", analImage);
	}*/
}

Blob detectGrid(Mat image) {
	Blob corners;
	int h = image.rows;
	int w = image.cols;
	for (int x = 0; x < w; x += 1) {
		for (int y = 0; y < h; y += 1) {
			if (checkPixel(x, y, image) == true) {
				corners.pixels.push_back({ x, y, 1, 0 });
			}
		}
	}
	return corners;
}

Mat contrastBrightness(Mat image, double contrast, int brightness) {
	Mat new_image = Mat::zeros(image.size(), image.type());
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			for (int c = 0; c < image.channels(); c++) {
				new_image.at<Vec3b>(y, x)[c] =
					saturate_cast<uchar>(contrast*image.at<Vec3b>(y, x)[c] + brightness);
			}
		}
	}
	return new_image;
}

Grid findGrid(Mat image, Grid grid1, double presision) {
	image = contrastBrightness(image, 1.2, -50);
	int upLim[] = { 90,240,240 };
	int lowLim[] = { 40,0,0 };
	imshow("grid", imread("gridTestImage.png"));
	imshow("camera1", image);
	Mat imageMask;
	image.copyTo(imageMask);
	imageMask = makeMask(imageMask, upLim, lowLim, 3);
	resize(imageMask, imageMask, Size(), 1 / presision, 1 / presision);
	Blob corners = detectGrid(imageMask);
	if (corners.pixels.empty() == false) {
		for (int i = 0; i < corners.pixels.size(); i++) {
			corners.pixels.at(i).at(0) *= presision;
			corners.pixels.at(i).at(1) *= presision;
		}
		//image = blobanalysis(image, corners);
	}
	imshow("greensort", image);
	waitKey(1);
	return grid1;
}

vector<Boat> boatFinder(vector<Blob> blobs) {
	vector<Boat> boats;

	return boats;
}

int main(){
	double presision = 7;
	Mat image;
	VideoCapture cap(1);
	Grid grid1;
	Grid grid2;
	grid1.found = false;
	grid2.found = false;
	while (true) {
		Mat image;
		cap >> image;
		//image = contrastBrightness(image, 7.0, -1000);
		//imshow("yeeeeee", image);
		//waitKey(1);
		//image = imread("shapes.png");
		processImage(image, presision);
		/*
		if (grid1.found == true & grid2.found == true) {
			if(player1.active == true){
				image = cropImage(image, grid1);
			}
			else{
				image = cropImage(image, grid2);
			}
			processImage(image, presision);
		}
		else if(grid1.found == false){
			grid1 = findGrid(image, grid1, presision);
		}
		else {
			grid2 = findGrid(image, grid2, presision);
		}*/
		waitKey(1);
	}
	return 0;
}