#include "config.h"
#include "ui_config.h"

#include "reminder.h"
#include "mainwindow.h"
#include "audioconfig.h"

#include <QMessageBox>
#include <QTimer>

Config::Config(QWidget* parent, MainWindow* mw_) :
    QWidget(parent),
    ui(new Ui::Config),
    mw(mw_),
    timer(new QTimer),
    prem(nullptr)
{
    ui->setupUi(this);

    ui->box_audio->addItem("None");

    timer->setInterval(100);
}

Config::~Config()
{
    delete ui;
}

void Config::set_config(const Reminder& reminder)
{
    const ReminderData& data = reminder.get_data();
    ui->check_active->setChecked(reminder.get_active());
    ui->line_message->setText(data.message);
    ui->time_interval->setTime(data.interval);
    ui->time_alarm->setTime(data.alarm);
    ui->time_hysteresis->setTime(data.hysteresis);
    ui->check_repeat->setChecked(data.repeat);
    ui->spin_repeat->setValue(data.repeat_time);
    ui->button_confirm->setEnabled(reminder.get_hysteresis());
    ui->button_save->setDisabled(reminder.get_active());
    ui->box_audio->setCurrentText(data.audio_name);

    switch (data.mode)
    {
    case Interval:
        ui->radioButton_interval->setChecked(true);
        break;
    case Alarm:
        ui->radioButton_clock->setChecked(true);
        break;
    default:
        ;
    }

    prem = &reminder;

    connect(
        ui->button_confirm, SIGNAL(clicked()),
        prem, SLOT(confirm())
    );
}

void Config::set_audio(const QList<AudioConfig>& audio_configs)
{
    ui->box_audio->clear();
    ui->box_audio->addItem("None");
    for (const AudioConfig& config : audio_configs)
    {
        ui->box_audio->addItem(config.name);
    }
    if (prem)
    {
        ui->box_audio->setCurrentText(prem->get_data().audio_name);
    }
}

void Config::setup_finish()
{
    connect(
        timer, SIGNAL(timeout()),
        this, SLOT(timer_timeout())
    );
    timer->start();

    connect(
        ui->button_save, SIGNAL(clicked()),
        this, SLOT(save_config())
    );

    connect(
        ui->check_active, SIGNAL(stateChanged(int)),
        this, SLOT(active_change(int))
    );
}

void Config::save_config()
{
    ReminderData data;

    if (ui->radioButton_interval->isChecked())
    {
        data.mode = Interval;
    }
    else if (ui->radioButton_clock->isChecked())
    {
        data.mode = Alarm;
    }

    data.interval = ui->time_interval->time();
    data.alarm = ui->time_alarm->time();
    data.hysteresis = ui->time_hysteresis->time();
    data.repeat_time = ui->spin_repeat->value();
    data.message = ui->line_message->text();
    data.repeat = ui->check_repeat->isChecked();
    data.audio_name = ui->box_audio->currentText();
    if (data.message.length() == 0)
    {
        QMessageBox::critical(mw, "Error",
            "Message is Empty.\n"
            "Please Set Any Message."
        );
        return;
    }
    if (data.repeat_time == 0)
    {
        QMessageBox::critical(this, "Error",
            "Repeat times can't be ZERO."
        );
        return;
    }
    prem = mw->save_config(ui->check_active->isChecked(), data, prem);
    if (prem)
    {
        ui->button_confirm->disconnect();
        connect(
            ui->button_confirm, SIGNAL(clicked()),
            prem, SLOT(confirm())
        );
    }
}

void Config::active_change(int state)
{
    bool a = state == Qt::Checked;
    if (!prem)
    {
        QMessageBox::critical(mw, "Error", "Not saved, please save first.");
        ui->check_active->setChecked(false);
        return;
    }
    ((Reminder*)prem)->set_active(a);
}

void Config::timer_timeout()
{
    if (!prem)
    {
        return;
    }
    if (prem->get_hysteresis())
    {
        ui->button_confirm->setEnabled(true);
        ui->button_confirm->setStyleSheet("color: rgb(255, 0, 0);");
    }
    else
    {
        ui->button_confirm->setEnabled(false);
        ui->button_confirm->setStyleSheet(QString());
    }
    if (prem->get_active())
    {
        int remain = prem->time_remain();
        QTime time(0, 0);
        QTime t = time.addMSecs(remain);
        ui->label_remain->setText(QString("Time Remain %1").arg(t.toString("HH:mm:ss")));
    }
    else
    {
        ui->label_remain->setText(QString());
        if (ui->check_active->isChecked())
            ui->check_active->setChecked(false);
    }
}
