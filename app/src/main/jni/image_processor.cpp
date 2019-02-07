#include "image_processor.h"

#include <algorithm>
#include <stdlib.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>

#include "common.hpp"

enum DisplayMode {
  DISP_MODE_RAW = 0,
  DISP_MODE_THRESH = 1,
  DISP_MODE_TARGETS = 2,
  DISP_MODE_TARGETS_PLUS = 3
};

struct TargetInfo {
  double centroid_x;
  double centroid_y;
  double width;
  double height;
  double angle;
  std::vector<cv::Point> points;
};

std::vector<TargetInfo> processImpl(int w, int h, int texOut, DisplayMode mode,
                                    int h_min, int h_max, int s_min, int s_max,
                                    int v_min, int v_max) {
  LOGD("Image is %d x %d", w, h);
  LOGD("H %d-%d S %d-%d V %d-%d", h_min, h_max, s_min, s_max, v_min, v_max);
  int64_t t;

  static cv::Mat input;
  input.create(h, w, CV_8UC4);

  // read
  t = getTimeMs();
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, input.data);
  LOGD("glReadPixels() costs %d ms", getTimeInterval(t));

  // modify
  t = getTimeMs();
  static cv::Mat hsv;
  cv::cvtColor(input, hsv, CV_RGBA2RGB);
  cv::cvtColor(hsv, hsv, CV_RGB2HSV);
  LOGD("cvtColor() costs %d ms", getTimeInterval(t));

  t = getTimeMs();
  static cv::Mat thresh;
  cv::inRange(hsv, cv::Scalar(h_min, s_min, v_min),
              cv::Scalar(h_max, s_max, v_max), thresh);
  LOGD("inRange() costs %d ms", getTimeInterval(t));

  t = getTimeMs();
  static cv::Mat contour_input;
  contour_input = thresh.clone();
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Point> convex_contour;
  std::vector<cv::Point> poly;
  std::vector<TargetInfo> accepted_targets;
  std::vector<TargetInfo> targets;
  std::vector<TargetInfo> rejected_targets;
  cv::findContours(contour_input, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_TC89_KCOS);
  for (auto &contour : contours) {
    convex_contour.clear();
    cv::convexHull(contour, convex_contour, false);
    poly.clear();

    if (cv::isContourConvex(convex_contour)) {
      TargetInfo target;

      cv::RotatedRect minRect = minAreaRect(convex_contour); // ANGLED
      cv::Rect bounding_rect = cv::boundingRect(convex_contour);

      cv::Point2f centerMinRect = minRect.center;


      target.centroid_x = centerMinRect.x;// + (bounding_rect.width / 2);
      // centroid Y is top of target because it changes shape as you move

      target.centroid_y = centerMinRect.y;// + bounding_rect.height;
      target.angle = minRect.angle; // angle (always negative from right side to hirizontal axis)

      if (abs(target.angle) < 44) {
          target.width = minRect.size.height;//bounding_rect.width;
          target.height = minRect.size.width;
      } else {
          target.width = minRect.size.width;//bounding_rect.width;
          target.height = minRect.size.height;
      }

      target.points = convex_contour;


        LOGD("target width : %.2lf", target.width); // double
        LOGD("target height : %.2lf", target.height); // double

      // Filter based on size
      // Keep in mind width/height are in imager terms...
      const double kMinTargetWidth = 0;
      const double kMaxTargetWidth = 1000;
      const double kMinTargetHeight = 0;
      const double kMaxTargetHeight = 1000;
      if (target.width < kMinTargetWidth || target.width > kMaxTargetWidth ||
          target.height < kMinTargetHeight ||
          target.height > kMaxTargetHeight) {
        LOGD("Rejecting target due to size. H: %.2lf | W: %.2lf",
          target.height, target.width);
        rejected_targets.push_back(std::move(target));
        continue;
      }

      // Filter based on shape
      const double kMaxWideness = 20;
      const double kMinWideness = 0.05;
      double wideness = target.width / target.height;
      if (wideness < kMinWideness || wideness > kMaxWideness) {
        LOGD("Rejecting target due to shape : %.2lf", wideness);
        rejected_targets.push_back(std::move(target));
        continue;
      }

      //Filter based on fullness
      const double kMinFullness = .9;
      const double kMaxFullness = 1;
      double original_contour_area = cv::contourArea(convex_contour);
      double area = target.width * target.height * 1.0;
      double fullness = original_contour_area / area;
      if (fullness < kMinFullness || fullness > kMaxFullness) {
        LOGD("Rejecting target due to fullness : %.2lf", fullness);
        rejected_targets.push_back(std::move(target));
        continue;
      }

        // angle of RotatedRect
        target.angle = minRect.angle;
        LOGD("target angle : %.2lf", target.angle);

      // We found a target
      LOGD("Found target at %.2lf, %.2lf %.2lf, %.2lf, angle %.2lf", target.centroid_x, target.centroid_y, target.width, target.height, target.angle);
      accepted_targets.push_back(std::move(target));
    }
  }
  LOGD("Contour analysis costs %d ms", getTimeInterval(t));




  const double kMaxOffset = 50;
  const double kMinOffset = 0;
  bool found = false;
  for (int i = 0; !found && i < accepted_targets.size(); i++) {
    for (int j = i; !found && j < accepted_targets.size(); j++) {
      if (i == j) {
        continue;
      }
      TargetInfo targetI = accepted_targets[i];
      TargetInfo targetJ = accepted_targets[j];
      double vertOffset = abs(targetI.centroid_y - targetJ.centroid_y);
      if (vertOffset < kMaxOffset) {
        TargetInfo leftTarget = targetI.centroid_x < targetJ.centroid_x ? targetI : targetJ;
        TargetInfo rightTarget = targetI.centroid_x < targetJ.centroid_x ? targetJ : targetI;
        if (abs(leftTarget.angle) < 45 && abs(rightTarget.angle) > 45) {
          TargetInfo combinedTarget;
          combinedTarget.centroid_x = (leftTarget.centroid_x + rightTarget.centroid_x)/2;
          combinedTarget.centroid_y = (leftTarget.centroid_y + rightTarget.centroid_y)/2;
          combinedTarget.width = rightTarget.centroid_x - leftTarget.centroid_x;
          combinedTarget.height = (sqrt(leftTarget.width * leftTarget.width + leftTarget.height + leftTarget.height) + sqrt(rightTarget.width * rightTarget.width + rightTarget.height + rightTarget.height)) / 2;
          // combinedTarget.angle = 0; // calculated later when sending to RIO
          if (leftTarget.width > rightTarget.width) { // must be a left turn
            // multiply by distance
            // big divided by little
            combinedTarget.angle =  -(leftTarget.height * leftTarget.width) / (rightTarget.height * rightTarget.width);
          } else {           // must be a right turn
            combinedTarget.angle = (rightTarget.height * rightTarget.width) / (leftTarget.height * leftTarget.width);
            //combinedTarget.angle = (-2);

          }
          targets.push_back(std::move(combinedTarget));
          //targets.push_back(std::move(rightTarget));

          /*
          double centroid_x;
            double centroid_y;
            double width;
            double height;
            double angle;
          */

          found = true;
          break;
        } else {
          rejected_targets.push_back(std::move(targetI));
          rejected_targets.push_back(std::move(targetJ));
        }
      } else {
        rejected_targets.push_back(std::move(targetI));
        rejected_targets.push_back(std::move(targetJ));
      }
    }
  }


  // write back
  t = getTimeMs();
  static cv::Mat vis;
  if (mode == DISP_MODE_RAW) {
    vis = input;
  } else if (mode == DISP_MODE_THRESH) {
    cv::cvtColor(thresh, vis, CV_GRAY2RGBA);
  } else {
    vis = input;
    // Render the targets
    for (auto &target : targets) {
      cv::polylines(vis, target.points, true, cv::Scalar(0, 112, 255), 3);
      cv::circle(vis, cv::Point(target.centroid_x, target.centroid_y), 4,
                 cv::Scalar(255, 50, 255), 3);
    }
  }
  if (mode == DISP_MODE_TARGETS_PLUS) {
    for (auto &target : rejected_targets) {
      cv::polylines(vis, target.points, true, cv::Scalar(255, 0, 0), 3);
    }
  }
  LOGD("Creating vis costs %d ms", getTimeInterval(t));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texOut);
  t = getTimeMs();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                  vis.data);
  LOGD("glTexSubImage2D() costs %d ms", getTimeInterval(t));
  return targets;
}

static bool sFieldsRegistered = false;

static jfieldID sNumTargetsField;
static jfieldID sTargetsField;

static jfieldID sCentroidXField;
static jfieldID sCentroidYField;
static jfieldID sWidthField;
static jfieldID sHeightField;
static jfieldID sAngleField;

static void ensureJniRegistered(JNIEnv *env) {
  if (sFieldsRegistered) {
    return;
  }
  sFieldsRegistered = true;
  jclass targetsInfoClass =
      env->FindClass("com/team4786/fearvision/NativePart$TargetsInfo");
  sNumTargetsField = env->GetFieldID(targetsInfoClass, "numTargets", "I");
  sTargetsField = env->GetFieldID(
      targetsInfoClass, "targets",
      "[Lcom/team4786/fearvision/NativePart$TargetsInfo$Target;");
  jclass targetClass =
      env->FindClass("com/team4786/fearvision/NativePart$TargetsInfo$Target");

  sCentroidXField = env->GetFieldID(targetClass, "centroidX", "D");
  sCentroidYField = env->GetFieldID(targetClass, "centroidY", "D");
  sWidthField = env->GetFieldID(targetClass, "width", "D");
  sHeightField = env->GetFieldID(targetClass, "height", "D");
  sAngleField = env->GetFieldID(targetClass, "angle", "D");
}

extern "C" void processFrame(JNIEnv *env, int tex1, int tex2, int w, int h,
                             int mode, int h_min, int h_max, int s_min,
                             int s_max, int v_min, int v_max,
                             jobject destTargetInfo) {
  auto targets = processImpl(w, h, tex2, static_cast<DisplayMode>(mode), h_min,
                             h_max, s_min, s_max, v_min, v_max);
  int numTargets = targets.size();
  //double center_x = ( (targets[numTargets - 1]).centroid_x + (targets[numTargets - 2]).centroid_x)/2;

       // LOGD("center_x : %.2lf", center_x);
  ensureJniRegistered(env);
  env->SetIntField(destTargetInfo, sNumTargetsField, numTargets);
  if (numTargets == 0) {
    return;
  }
  jobjectArray targetsArray = static_cast<jobjectArray>(
      env->GetObjectField(destTargetInfo, sTargetsField));
  for (int i = 0; i < std::min(numTargets, 3); ++i) {
    jobject targetObject = env->GetObjectArrayElement(targetsArray, i);
    const auto &target = targets[i];
    env->SetDoubleField(targetObject, sCentroidXField, target.centroid_x);
    env->SetDoubleField(targetObject, sCentroidYField, target.centroid_y);
    env->SetDoubleField(targetObject, sWidthField, target.width);
    env->SetDoubleField(targetObject, sHeightField, target.height);
    env->SetDoubleField(targetObject, sAngleField, target.angle);
  }
}
