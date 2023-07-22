#ifndef CONFIG_H
#define CONFIG_H

#include <QWidget>

class MainWindow;

namespace Ui {
class Config;
}

class Reminder;
struct AudioConfig;

class Config : public QWidget
{
    Q_OBJECT

public:
    explicit Config(QWidget* parent = nullptr, MainWindow* mw_ = nullptr);
    ~Config();
    void set_config(const Reminder& reminder);
    void set_audio(const QList<AudioConfig>& audio_configs);
    void setup_finish();

private:
    Ui::Config* ui;
    MainWindow* mw;
    QTimer* timer;
    const Reminder* prem;

private slots:
    void save_config();
    void active_change(int state);
    void timer_timeout();
};

#endif // CONFIG_H
