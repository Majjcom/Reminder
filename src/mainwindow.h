#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QMap>

#include "reminder.h"
#include "singleappservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Config;
struct AudioConfig;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Reminder* save_config(const bool active, const ReminderData& data, const Reminder* rem_);
    QString get_audio_path(QString name);

protected:
    void changeEvent(QEvent* event) override;

private:
    Ui::MainWindow* ui;
    QSystemTrayIcon* icon;
    QMenu* menu;
    QMap<QString, Reminder*> reminders;
    Config* current_config;
    SingleAppService* service;
    QList<AudioConfig> audio_configs;
    QMap<QString, QString> audio_path_map;
    void load_audio_settings();
    void save_audio_settings();
    void update_audio_map();

public slots:
    void show_window();

private slots:
    void about();
    void icon_activated(QSystemTrayIcon::ActivationReason reason);
    void new_reminder();
    void delete_reminder();
    void timeout(bool hysteresis, QString msg);
    void select_changed();
    void remove_current();
    void close_clicked();
    void load_clicked();
    void save_clicked();
    void audio_settings();

};
#endif // MAINWINDOW_H
