#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

#include <QDialog>
#include <QList>
#include <QMap>

namespace Ui {
class Dialog_audio_settings;
}

class QSoundEffect;
struct AudioConfig;

class Dialog_audio_settings : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_audio_settings(QWidget *parent = nullptr);
    ~Dialog_audio_settings();
    void setup_data(const QList<AudioConfig>& configs);
    QList<AudioConfig> get_data() const;

private:
    Ui::Dialog_audio_settings *ui;
    QMap<QString, QString> name_path_map;
    QSoundEffect* effect;

private slots:
    void button_play_clicked();
    void button_del_clicked();
    void button_play_temp_clicked();
    void button_open_clicked();
    void button_add_clicked();
    void path_changed(const QString& text);
};

#endif // DIALOG_SETTINGS_H
