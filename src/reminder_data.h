#ifndef REMINDER_DATA_H
#define REMINDER_DATA_H

#include <QString>
#include <QTimer>
#include <QTime>

enum ReminderMode
{
    Interval = 0,
    Alarm = 1,
};

struct ReminderData
{
    QString message;
    ReminderMode mode;
    QTime interval;
    QTime alarm;
    QTime hysteresis;
    bool repeat;
    int repeat_time;
    QString audio_name;
};

#endif // REMINDER_DATA_H
