package com.team4786.fearvision.comm.messages;

import com.team4786.fearvision.comm.VisionUpdate;

public class TargetUpdateMessage extends VisionMessage {

    VisionUpdate mUpdate;
    long mTimestamp;

    public TargetUpdateMessage(VisionUpdate update, long timestamp) {
        mUpdate = update;
        mTimestamp = timestamp;
    }
    @Override
    public String getType() {
        return "targets";
    }

    @Override
    public String getMessage() {
        return mUpdate.getSendableJsonString(mTimestamp);
    }
}
