# FearVision
Android-based Vision System

## To install OpenCV
* Download http://sourceforge.net/projects/opencvlibrary/files/opencv-android/3.1.0/OpenCV-3.1.0-android-sdk.zip/download
* Unzip OpenCV-3.1.0-android-sdk.zip (OpenCV-android-sdk)
* Create directory app/src/main/jniLibs
* Copy OpenCV-android-sdk/sdk/native/libs/* to app/src/main/jniLibs/

## To provision a device for robot use
* Enable device admin
1. Settings App > Security > Device Administrators > Click box 'on' for fearvision

* Enable device owner
1. adb shell
2. dpm set-device-owner com.team4786.fearvision/.ChezyDeviceAdminReceiver

## How to Install ADB on the RoboRIO
https://github.com/team8/RIOdroid/


Notes for Android Studio:

Project Structure:
Android NDK (download from Android Studio)
SDK 23

Add NDK path to PATH Environmental Variables
"Advanced System Properties"

Also might need to add the JDK to Path and/or JDK path to JAVA_HOME variable

Open the Terminal in Android Studio (Alt+F12 in Windows).
1. cd app/src/main/jni
2. ndk-build V=1
(This will tell you exactly why the build is failing)