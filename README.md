# FearVision
Android-based Vision System

## Prepare Android Studio for this project
1. OpenCV
* Download http://sourceforge.net/projects/opencvlibrary/files/opencv-android/3.1.0/OpenCV-3.1.0-android-sdk.zip/download
* Unzip OpenCV-3.1.0-android-sdk.zip (OpenCV-android-sdk)
* Create directory app/src/main/jniLibs
* Copy OpenCV-android-sdk/sdk/native/libs/* to app/src/main/jniLibs/

2. Install NDK
* Go to Project Structure, click "Download" under Android NDK Location
* Copy the NDK path (typically: C:\Users\fear8\AppData\Local\Android\Sdk\ndk-bundle)
and add it to the PATH Environmental Variables
* This project requires the MIPS libraries from the 16b NDK. Download this and copy the
two MIPS libraries from the 16b NDK and copy them into the local NDK path
https://developer.android.com/ndk/downloads/older_releases
* Restart Android to have the NDK changes updated

3. You might need to add the JDK to Path and/or JDK path to JAVA_HOME variable

## Install ADB on the RoboRIO
Clone https://github.com/NicoletFEAR/RIOdroid
Unzip "release/RIOdroid v0.1.2.zip" and follow the instructions in the README


## Having trouble?
Open the Terminal in Android Studio (Alt+F12 in Windows).
1. cd app/src/main/jni
2. ndk-build V=1
(This will tell you why the build is failing)