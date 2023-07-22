#ifndef REMINDER_H
#define REMINDER_H

#include <QObject>

#include "reminder_data.h"

class QSoundEffect;
class MainWindow;

class Reminder : public QObject
{
    Q_OBJECT
public:
    explicit Reminder(MainWindow* parent = nullptr);
    const ReminderData& get_data() const;
    void set_data(const ReminderData& data_);
    int time_remain() const;
    void set_active(bool active_);
    bool get_active() const;
    bool get_hysteresis() const;
    QString to_json() const;
    void from_json(QString json);

private:
    int repeat;
    bool active;
    bool hysteresis;
    ReminderData data;
    QTimer* timer;
    QSoundEffect* effect;
    void set_interval();

private slots:
    void confirm();
    void timer_timeout();

signals:
    void timeout(bool hysteresis, QString msg);
};

#endif // REMINDER_H
