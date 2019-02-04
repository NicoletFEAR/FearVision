package com.team4786.fearvision;

public interface RobotEventListener {
    public void shotTaken();
    public void wantsVisionMode();
    public void wantsIntakeMode();
    public void switchCameraFront();
    public void switchCameraBack();
}
