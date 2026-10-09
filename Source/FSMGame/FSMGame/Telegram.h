#pragma once

enum msgTypes
{
    MSG_ITEM_STOLEN,
    MSG_THIEF_SPOTTED,
    MSG_GUARD_CHASING,
    MSG_GUARD_RETURNING
};

struct Telegram
{
    int          Sender;
    int          Receiver;
    int          Msg;
    double       DispatchTime;
    void*        ExtraInfo;


    Telegram()
        : DispatchTime(-1), Sender(-1), Receiver(-1), Msg(-1), ExtraInfo(nullptr) {}


    Telegram(double time, int sender, int receiver, int msg, void* info = nullptr)
        : DispatchTime(time), Sender(sender), Receiver(receiver), Msg(msg), ExtraInfo(info) {}

};
