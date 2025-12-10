#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "JabraDeviceConfig.h"
#include "Interface_Bluetooth.h"

void play_media();
void pause_media();

void HeadDetectionCallback(unsigned short deviceID, HeadDetectionStatus headDetectionStatus) {
    bool isHeadDetected = headDetectionStatus.leftOn || headDetectionStatus.rightOn;

    printf("Head detection callback - Device ID: %hu - Head detected: %s\n",
           deviceID,
           (isHeadDetected ? "true" : "false"));

    if(isHeadDetected) {
        play_media();
    } else {
        pause_media();
    }

}

void DeviceAttachedFunc(Jabra_DeviceInfo deviceInfo)
{
    printf("Device attached: ID %hu - Name: %s\n",
           deviceInfo.deviceID,
           deviceInfo.deviceName);
    
    if(deviceInfo.isDongle) return; // Ignore dongles for head detection

    Jabra_SetHeadDetectionStatusListener(deviceInfo.deviceID, HeadDetectionCallback);
}

void signal_handler(int sig) {
    printf("Exiting program\n");
    exit(0);
}

int main()
{
    Jabra_SetAppID("app");
    Jabra_InitializeV2(NULL, DeviceAttachedFunc, NULL, NULL, NULL, false, NULL);

    signal(SIGINT, signal_handler);

    pause();
    return 0;
}
