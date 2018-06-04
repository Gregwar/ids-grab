#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ueye.h>
#include <opencv2/highgui/highgui.hpp>

int main()
{
    HIDS hCam = 1;

    INT nRet = is_InitCamera(&hCam, NULL);
    int imgWidth = 1280;
    int imgHeight = 720;

    if (nRet == IS_SUCCESS)
    {
        // Searching for format
        uint32_t entries;
        is_ImageFormat(hCam, IMGFRMT_CMD_GET_NUM_ENTRIES, &entries, sizeof(entries));

        char formats[sizeof(IMAGE_FORMAT_LIST) + (entries-1)*sizeof(IMAGE_FORMAT_INFO)];
        IMAGE_FORMAT_LIST *formatList = (IMAGE_FORMAT_LIST*)formats;
        formatList->nNumListElements = entries;
        formatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
        is_ImageFormat(hCam, IMGFRMT_CMD_GET_LIST, formats, sizeof(formats));
        bool found = false;
        for (int k=0; k<entries; k++) {
            IMAGE_FORMAT_INFO *info = &formatList->FormatInfo[k];
            printf("w: %d, h: %d\n", info->nWidth, info->nHeight);
            if (info->nWidth == imgWidth && info->nHeight == imgHeight) {
                is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &info->nFormatID, sizeof(info->nFormatID));
                found = true;
            }
        }
        if (!found) {
            std::cerr << "Unsupported resolution: " << imgWidth << "x" << imgHeight << std::endl;
        }

        // Image memory allocation
        is_ClearSequence(hCam);
        for (int k=0; k<4; k++) {
            char *mem;
            int memId;
            is_AllocImageMem(hCam, imgWidth, imgHeight, 24, &mem, &memId);
            is_AddToSequence(hCam, mem, memId);
        }

        // Exposure
        double exposure = 5;
        is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exposure, sizeof(exposure));

        // White balance
        is_SetHardwareGain(hCam, 80, 1, 0, 70);

        // Enabling anti-flicker
        double flicker = ANTIFLCK_MODE_SENS_50_FIXED;
        is_SetAutoParameter(hCam, IS_SET_ANTI_FLICKER_MODE, &flicker, NULL);

        // Setting framerate
        is_SetColorMode(hCam, IS_CM_RGB8_PACKED);
        double fps = 60, newFps = 60;
        is_SetFrameRate(hCam, fps, &newFps);

        // is_SetDisplayMode(hCam, IS_SET_DM_DIB);
        // is_SetExternalTrigger(hCam, IS_SET_TRIGGER_SOFTWARE);

        is_CaptureVideo(hCam, IS_WAIT);
        is_EnableEvent(hCam, IS_SET_EVENT_FRAME);

        int k = 0;
        while (true) {
            is_WaitEvent(hCam, IS_SET_EVENT_FRAME, 100);
            int memId;
            char *memCur, *memLast;
            is_GetActSeqBuf(hCam, &memId, &memCur, &memLast);
            k++;
            printf("%d ) Mem: %p\n", k, memLast);

            IplImage * img;
            img=cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 3);

            for (int y=0; y<imgHeight; y++) {
                uchar* ptr = (uchar*) (img->imageData + y * img->widthStep );
                for (int x=0; x<imgWidth; x++) {
                    ptr[3*x + 0] = memLast[3*(y*imgWidth + x) + 2];
                    ptr[3*x + 1] = memLast[3*(y*imgWidth + x) + 1];
                    ptr[3*x + 2] = memLast[3*(y*imgWidth + x) + 0];
                }
            }

            //now you can use your img just like a normal OpenCV image
            cvNamedWindow("IDS", 1);
            cvShowImage("IDS",img);
            cv::waitKey(1);
        }
        /*
        while (is_FreezeVideo(hCam, IS_WAIT) == IS_SUCCESS) {
        }
        */

        printf("Openning success!\n");
    }
}
