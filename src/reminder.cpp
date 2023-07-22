#include "reminder.h"

#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

#include "mainwindow.h"

#include <QSoundEffect>
#include <QFileInfo>

Reminder::Reminder(MainWindow* parent)
    : QObject{parent}
    , active(false)
    , hysteresis(false)
    , effect(new QSoundEffect(this))
{
    timer = new QTimer(this);
    effect->setLoopCount(0);

    connect(
        timer, SIGNAL(timeout()),
        this, SLOT(timer_timeout())
    );
}

const ReminderData& Reminder::get_data() const
{
    return data;
}

bool Reminder::get_hysteresis() const
{
    return hysteresis;
}

void Reminder::set_data(const ReminderData& data_)
{
    data = data_;
}

QString Reminder::to_json() const
{
    using rapidjson::StringBuffer;
    using rapidjson::Writer;

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    writer.String("message");
    writer.String(data.message.toUtf8());
    writer.String("mode");
    writer.Int(data.mode);
    writer.String("interval");
    writer.Int64(data.interval.msecsSinceStartOfDay());
    writer.String("alarm");
    writer.Int64(data.alarm.msecsSinceStartOfDay());
    writer.String("hysteresis");
    writer.Int64(data.hysteresis.msecsSinceStartOfDay());
    writer.String("repeat");
    writer.Bool(data.repeat);
    writer.String("repeat_time");
    writer.Int(data.repeat_time);
    writer.String("audio_name");
    writer.String(data.audio_name.toUtf8());
    writer.EndObject();

    return QString::fromUtf8(buffer.GetString(), (int)buffer.GetLength());
}

void Reminder::from_json(QString json)
{
    using namespace rapidjson;
    Document doc;
    doc.Parse(json.toUtf8());
    if (doc.HasParseError())
    {
        return;
    }

    if (!doc.HasMember("message")     ||
        !doc.HasMember("mode")        ||
        !doc.HasMember("interval")    ||
        !doc.HasMember("alarm")       ||
        !doc.HasMember("hysteresis")  ||
        !doc.HasMember("repeat")      ||
        !doc.HasMember("repeat_time") ||
        !doc.HasMember("audio_name"))
    {
        return;
    }

    ReminderData new_data;
    new_data.message = QString::fromUtf8(doc["message"].GetString(), doc["message"].GetStringLength());
    new_data.mode = (ReminderMode)doc["mode"].GetInt();
    new_data.interval = QTime(0, 0).addMSecs(doc["interval"].GetInt64());
    new_data.alarm = QTime(0, 0).addMSecs(doc["alarm"].GetInt64());
    new_data.hysteresis = QTime(0, 0).addMSecs(doc["hysteresis"].GetInt64());
    new_data.repeat = doc["repeat"].GetBool();
    new_data.repeat_time = doc["repeat_time"].GetInt();
    new_data.audio_name = QString::fromUtf8(doc["audio_name"].GetString(), doc["audio_name"].GetStringLength());

    data = new_data;
}

void Reminder::set_interval()
{
    QTime current = QTime::currentTime();
    switch (data.mode)
    {
    case Interval:
        timer->setInterval(data.interval.msecsSinceStartOfDay());
        break;
    case Alarm:
        qint64 interval;
        if (data.alarm >= current)
        {
            interval = data.alarm.msecsSinceStartOfDay() - current.msecsSinceStartOfDay();
        }
        else
        {
            interval = data.alarm.msecsSinceStartOfDay() - current.msecsSinceStartOfDay() + QTime(23, 59, 59, 999).msecsSinceStartOfDay();
        }
        timer->setInterval(interval);
        break;
    default:
        ;
    }
}

void Reminder::set_active(bool active_)
{
    active = active_;
    if (!active_)
    {
        timer->stop();
        hysteresis = false;
        return;
    }
    repeat = data.repeat_time;
    this->set_interval();
    timer->start();
}

bool Reminder::get_active() const
{
    return active;
}

void Reminder::confirm()
{
    effect->stop();
    hysteresis = false;
    if (repeat > 0)
    {
        repeat -= 1;
    }
    if (data.repeat && repeat != 0)
    {
        this->set_interval();
        return;
    }
    timer->stop();
    active = false;
}

int Reminder::time_remain() const
{
    return timer->remainingTime();
}

void Reminder::timer_timeout()
{
    emit this->timeout(hysteresis, data.message);
    if (data.audio_name != "None")
    {
        QString path = static_cast<MainWindow*>(parent())->get_audio_path(data.audio_name);
        if (QFileInfo::exists(path))
        {
            effect->setSource(QUrl::fromLocalFile(path));
            effect->play();
        }
    }
    if (!hysteresis)
    {
        hysteresis = true;
        timer->setInterval(data.hysteresis.msecsSinceStartOfDay());
    }
}
