#include <stdio.h>
#include <stdlib.h>
#include <ueye.h>
#include <opencv2/highgui/highgui.hpp>

int main()
{
    HIDS hCam = 1;

    INT nRet = is_InitCamera (&hCam, NULL);

    if (nRet == IS_SUCCESS)
    {
	is_SetExternalTrigger(hCam, IS_SET_TRIGGER_SOFTWARE);

	// Set the flash to a high active pulse for each image in the trigger mode
	UINT nMode = IO_FLASH_MODE_TRIGGER_HI_ACTIVE;
	is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));

	char *mem;
	int memId;
	is_AllocImageMem(hCam, 1280, 1024, 24, &mem, &memId);
	is_SetImageMem(hCam, mem, memId);
	is_GetImageMem(hCam, (void**)&mem);
	printf("Mem: %p\n", mem);

	// Exposure
	double exposure = 5;
	is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exposure, sizeof(exposure));

	// White balance
	is_SetHardwareGain(hCam, 80, 1, 0, 70);

	double flicker = ANTIFLCK_MODE_SENS_50_FIXED;
	is_SetAutoParameter(hCam, IS_SET_ANTI_FLICKER_MODE, &flicker, NULL);

	is_SetDisplayMode(hCam, IS_SET_DM_DIB);
	is_SetColorMode(hCam, IS_CM_RGB8_PACKED);
	double fps = 60, newFps = 60;
	is_SetFrameRate(hCam, fps, &newFps);

	while (is_FreezeVideo(hCam, IS_WAIT) == IS_SUCCESS) {
	    IplImage * img;
	    img=cvCreateImage(cvSize(1280, 1024), IPL_DEPTH_8U, 3);

	    for (int y=0; y<1024; y++) {
		uchar* ptr = (uchar*) (img->imageData + y * img->widthStep ); 
		for (int x=0; x<1280; x++) {
		    ptr[3*x + 0] = mem[3*(y*1280 + x) + 2];
		    ptr[3*x + 1] = mem[3*(y*1280 + x) + 1];
		    ptr[3*x + 2] = mem[3*(y*1280 + x) + 0];
		}
	    }

	    //now you can use your img just like a normal OpenCV image
	    cvNamedWindow("IDS", 1);
	    cvShowImage("IDS",img);
	    cv::waitKey(1); 
	}

	printf("Openning success!\n");
    }
}
