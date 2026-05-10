#include "myNativeWrapperFunctions.h"
#include <UIKit/UIKit.h>
#include <CoreLocation/CoreLocation.h>

void setIosParams()
{
   // Configure the phone to application to keep the phone screen awake
   [UIApplication sharedApplication].idleTimerDisabled = YES;

   // Set the GPS to maximum accuracy
//   [[CLLocationManager alloc] init].desiredAccuracy = kCLLocationAccuracyBestForNavigation;
}

