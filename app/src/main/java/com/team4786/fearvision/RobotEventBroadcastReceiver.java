package com.team4786.fearvision;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;


public class RobotEventBroadcastReceiver extends BroadcastReceiver {
    public static final String ACTION_SHOT_TAKEN = "ACTION_SHOT_TAKEN";
    public static final String ACTION_WANT_VISION = "ACTION_WANT_VISION";
    public static final String ACTION_WANT_INTAKE = "ACTION_WANT_INTAKE";
    public static final String ACTION_CAMERA_FRONT = "ACTION_CAMERA_FRONT";
    public static final String ACTION_CAMERA_BACK = "ACTION_CAMERA_BACK";


    private RobotEventListener m_listener;

    public RobotEventBroadcastReceiver(Context context, RobotEventListener listener) {
        this.m_listener = listener;

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_SHOT_TAKEN);
        intentFilter.addAction(ACTION_WANT_VISION);
        intentFilter.addAction(ACTION_WANT_INTAKE);
        intentFilter.addAction(ACTION_CAMERA_FRONT);
        intentFilter.addAction(ACTION_CAMERA_BACK);
        context.registerReceiver(this, intentFilter);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (ACTION_SHOT_TAKEN.equals(intent.getAction())) {
            m_listener.shotTaken();
        }
        if (ACTION_WANT_VISION.equals(intent.getAction())) {
            m_listener.wantsVisionMode();
        }
        if (ACTION_WANT_INTAKE.equals(intent.getAction())) {
            m_listener.wantsIntakeMode();
        }
        if (ACTION_CAMERA_FRONT.equals(intent.getAction())) {
            m_listener.switchCameraFront();
        }
        if (ACTION_CAMERA_BACK.equals(intent.getAction())) {
            m_listener.switchCameraBack();
        }
    }
}
