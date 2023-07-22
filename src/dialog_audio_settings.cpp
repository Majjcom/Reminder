#include "dialog_audio_settings.h"
#include "ui_dialog_audio_settings.h"

#include "audioconfig.h"

#include <QSoundEffect>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QUrl>
#include <QDir>

bool check_path(QString path)
{
    QFileInfo info(path);
    if (info.isFile() && info.suffix().toLower() == "wav")
    {
        return true;
    }
    return false;
}

Dialog_audio_settings::Dialog_audio_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_audio_settings),
    effect(new QSoundEffect(this))
{
    ui->setupUi(this);

    connect(
        ui->button_play, SIGNAL(clicked()),
        this, SLOT(button_play_clicked())
    );

    connect(
        ui->button_del, SIGNAL(clicked()),
        this, SLOT(button_del_clicked())
    );

    connect(
        ui->button_file, SIGNAL(clicked()),
        this, SLOT(button_open_clicked())
    );

    connect(
        ui->button_play_temp, SIGNAL(clicked()),
        this, SLOT(button_play_temp_clicked())
    );

    connect(
        ui->button_add, SIGNAL(clicked()),
        this, SLOT(button_add_clicked())
    );

    connect(
        ui->line_path, SIGNAL(textChanged(QString)),
        this, SLOT(path_changed(QString))
    );

    connect(
        ui->button_stop, SIGNAL(clicked()),
        effect, SLOT(stop())
    );
}

Dialog_audio_settings::~Dialog_audio_settings()
{
    delete ui;
}

void Dialog_audio_settings::setup_data(const QList<AudioConfig>& configs)
{

    for (const AudioConfig& config : configs)
    {
        if (!QFileInfo::exists(config.path))
        {
            continue;
        }
        ui->box_audio->addItem(config.name);
        name_path_map.insert(config.name, config.path);
    }
}

QList<AudioConfig> Dialog_audio_settings::get_data() const
{
    QList<AudioConfig> ret;

    for (auto iter = name_path_map.begin(); iter != name_path_map.end(); iter++)
    {
        QString name = iter.key();
        QString path = iter.value();
        if (!QFileInfo::exists(path))
        {
            continue;
        }
        AudioConfig a {
            name,
            path
        };
        ret.append(a);
    }

    return ret;
}

void Dialog_audio_settings::button_play_clicked()
{
    if (ui->box_audio->currentText() == "")
    {
        return;
    }
    QString path = name_path_map.value(ui->box_audio->currentText());
    if (!check_path(path))
    {
        return;
    }

    effect->setSource(QUrl::fromLocalFile(path));
    effect->play();
}

void Dialog_audio_settings::button_del_clicked()
{
    if (ui->box_audio->currentText() == "")
    {
        return;
    }
    QString name = ui->box_audio->currentText();
    ui->box_audio->removeItem(ui->box_audio->currentIndex());
    name_path_map.remove(name);
    effect->stop();
}

void Dialog_audio_settings::button_play_temp_clicked()
{
    QString path = ui->line_path->text();
    if (!check_path(path))
    {
        QMessageBox::warning(this, "Waring", "Nothing to play.");
        return;
    }
    effect->setSource(QUrl::fromLocalFile(path));
    effect->play();
}

void Dialog_audio_settings::button_open_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Audio File", QDir::homePath(), "wav files (*.wav)");
    if (path.length() == 0)
    {
        return;
    }
    ui->line_path->setText(path);
    ui->line_name->setText(QFileInfo(path).baseName());
}

void Dialog_audio_settings::button_add_clicked()
{
    QString path = ui->line_path->text();
    QString name = ui->line_name->text();
    if (!check_path(path))
    {
        QMessageBox::critical(this, "Error", "Path Error.");
        return;
    }
    if (name.length() == 0)
    {
        QMessageBox::critical(this, "Error", "Name is empty, please set a name.");
        return;
    }
    if (name_path_map.contains(name))
    {
        QMessageBox::critical(this, "Error", "Name exists, please, try other name");
        return;
    }
    if (name == "None")
    {
        QMessageBox::critical(this, "Error", "Name can't be \"None\".");
        return;
    }
    ui->box_audio->addItem(name);
    ui->box_audio->setCurrentText(name);
    name_path_map.insert(name, path);
}

void Dialog_audio_settings::path_changed(const QString& text)
{
    if (check_path(text))
    {
        ui->line_path->setStyleSheet("");
    }
    else
    {
        ui->line_path->setStyleSheet("color: rgb(255, 0, 0);");
    }
}
