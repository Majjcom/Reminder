#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QIODevice>
#include <QFileInfo>
#include <QDebug>
#include <QIcon>
#include <QTime>
#include <QFile>
#include <QDir>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "dialog_audio_settings.h"
#include "audioconfig.h"
#include "config.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , current_config(nullptr)
{
    ui->setupUi(this);

    service = new SingleAppService(this);

    icon = new QSystemTrayIcon(this);
    icon->setToolTip("Reminder App");
    icon->setIcon(QIcon(":/res/icon.svg"));
    icon->show();

    menu = new QMenu(this);
    QAction* act = new QAction("Exit", this);
    menu->addAction(act);

    this->setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    this->load_audio_settings();

    connect(
        act, SIGNAL(triggered()),
        this, SLOT(close())
    );

    connect(
        icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(icon_activated(QSystemTrayIcon::ActivationReason))
    );

    connect(
        icon, SIGNAL(messageClicked()),
        this, SLOT(show_window())
    );

    connect(
        ui->actionAbout, SIGNAL(triggered()),
        this, SLOT(about())
    );

    connect(
        ui->actionAudioSettings, SIGNAL(triggered()),
        this, SLOT(audio_settings())
    );

    connect(
        ui->button_new, SIGNAL(clicked()),
        this, SLOT(new_reminder())
    );

    connect(
        ui->button_delete, SIGNAL(clicked()),
        this, SLOT(delete_reminder())
    );

    connect(
        ui->listWidget, SIGNAL(itemSelectionChanged()),
        this, SLOT(select_changed())
    );

    connect(
        ui->button_close, SIGNAL(clicked()),
        this, SLOT(close_clicked())
    );

    connect(
        ui->button_save, SIGNAL(clicked()),
        this, SLOT(save_clicked())
    );

    connect(
        ui->button_load, SIGNAL(clicked()),
        this, SLOT(load_clicked())
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::about()
{
    QMessageBox::information(this, "About Reminder",
        "Reminder v1.2\n\n"
        "It's an app build for reminding important things for you.\n"
        "Including health, work, study and so on.\n"
        "I sincerely hope that this app can help you a lot.\n\n"
        "--Majjcom"
    );
}

void MainWindow::show_window()
{
    this->setVisible(true);
    this->setWindowState(Qt::WindowActive);
    this->setWindowState(Qt::WindowNoState);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    if (this->windowState() == Qt::WindowMinimized)
    {
        this->setVisible(false);
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::icon_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        this->show_window();
        break;
    case QSystemTrayIcon::Context:
        menu->exec(cursor().pos());
        break;
    default:
        break;
    }
}

void MainWindow::load_audio_settings()
{
    QString config_path = QDir::homePath() + "/reminder/audio.json";
    if (!QFileInfo::exists(config_path))
    {
        return;
    }
    QFile f(config_path);
    f.open(QIODevice::ReadOnly);
    QByteArray data = f.readAll();

    using namespace rapidjson;

    Document doc;
    doc.Parse(data.data(), data.length());
    if ( doc.HasParseError()      ||
        !doc.HasMember("version") ||
        !doc.HasMember("audios"))
    {
        f.close();
        f.remove();
        return;
    }

    if (QString(doc["version"].GetString()) != "1.1")
    {
        f.close();
        f.remove();
        return;
    }

    GenericArray audios = doc["audios"].GetArray();

    audio_configs.clear();

    for (auto& audio : audios)
    {
        AudioConfig cfg;
        auto& val = audio["name"];
        cfg.name = QString::fromUtf8(val.GetString(), val.GetStringLength());
        if (cfg.name == "None")
        {
            continue;
        }
        val = audio["path"];
        cfg.path = QString::fromUtf8(val.GetString(), val.GetStringLength());
        audio_configs.append(cfg);
    }

    this->update_audio_map();
}

void MainWindow::save_audio_settings()
{
    QString config_path = QDir::homePath() + "/reminder/audio.json";
    if (!QFileInfo::exists(config_path))
    {
        QDir home(QDir::homePath());
        if (!home.exists("reminder"))
        {
            home.mkdir("reminder");
        }
    }

    using rapidjson::StringBuffer;
    using rapidjson::Writer;

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();

    writer.String("version");
    writer.String("1.1");
    writer.String("audios");

    writer.StartArray();
    for (AudioConfig& config : audio_configs)
    {
        QByteArray s = config.name.toUtf8();
        writer.StartObject();

        writer.String("name");
        writer.String(s.data(), s.length(), true);

        s = config.path.toUtf8();
        writer.String("path");
        writer.String(s.data(), s.length(), true);

        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    QFile f(config_path);
    f.open(QIODevice::WriteOnly);
    f.write(buffer.GetString(), buffer.GetLength());
    f.waitForBytesWritten(1000);
    f.close();
}

void MainWindow::update_audio_map()
{
    audio_path_map.clear();
    for (AudioConfig& config : audio_configs)
    {
        audio_path_map.insert(config.name, config.path);
    }
}

QString MainWindow::get_audio_path(QString name)
{
    return audio_path_map.value(name);
}

void MainWindow::remove_current()
{
    if (!current_config)
        return;
    ui->gridLayout_2->removeWidget(current_config);
    current_config->disconnect();
    current_config->setParent(nullptr);
    current_config->close();
    current_config->deleteLater();
    current_config = nullptr;
}

Reminder* MainWindow::save_config(const bool active, const ReminderData& data, const Reminder* rem_)
{
    Reminder* rem = (Reminder*)rem_;
    bool contain_msg = reminders.contains(data.message);
    if (!rem)
    {
        if (contain_msg)
        {
            QMessageBox::critical(this, "Error", "Message exists. Please set other message.");
            return nullptr;
        }
        rem = new Reminder(this);
        connect(
            rem, SIGNAL(timeout(bool,QString)),
            this, SLOT(timeout(bool,QString))
        );
        reminders.insert(data.message, rem);
        ui->listWidget->addItem(data.message);
    }
    else
    {
        QString msg = rem_->get_data().message;
        if (!contain_msg && reminders.contains(msg))
        {
            reminders.remove(msg);
            QList<QListWidgetItem*> items = ui->listWidget->findItems(msg, Qt::MatchExactly);
            for (QListWidgetItem* item : items)
            {
                int row = ui->listWidget->row(item);
                ui->listWidget->takeItem(row);
            }
            reminders.insert(data.message, rem);
            ui->listWidget->addItem(data.message);
        }
    }
    rem->set_data(data);
    rem->set_active(active);
    return rem;
}

void MainWindow::new_reminder()
{
    remove_current();
    ui->listWidget->setCurrentItem(nullptr);
    current_config = new Config(ui->remindBox, this);
    current_config->set_audio(audio_configs);
    ui->gridLayout_2->addWidget(current_config);
    current_config->setup_finish();
}

void MainWindow::delete_reminder()
{
    remove_current();
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
    for (QListWidgetItem* item : items)
    {
        QString msg = item->text();
        int row = ui->listWidget->row(item);
        ui->listWidget->takeItem(row);
        Reminder* rem = reminders.take(msg);
        if (rem)
        {
            rem->disconnect();
            rem->set_active(false);
            rem->deleteLater();
        }
    }
}

void MainWindow::timeout(bool hysteresis, QString msg)
{
    QSystemTrayIcon::MessageIcon ico = hysteresis ? QSystemTrayIcon::Warning : QSystemTrayIcon::NoIcon;
    icon->showMessage("Reminder", msg, ico);
    if (!current_config)
    {
        current_config = new Config(ui->remindBox, this);
        ui->gridLayout_2->addWidget(current_config);
        current_config->set_audio(audio_configs);
        current_config->set_config(*reminders.value(msg));
        current_config->setup_finish();
    }
}

void MainWindow::select_changed()
{
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
    if (items.empty())
    {
        return;
    }
    for (QListWidgetItem* item : items)
    {
        remove_current();
        current_config = new Config(ui->remindBox, this);
        ui->gridLayout_2->addWidget(current_config);
        current_config->set_audio(audio_configs);
        current_config->set_config(*reminders.value(item->text()));
        current_config->setup_finish();
        break;
    }
}

void MainWindow::close_clicked()
{
    ui->listWidget->setCurrentItem(nullptr);
    remove_current();
}

void MainWindow::load_clicked()
{
    using namespace rapidjson;
    QString path = QFileDialog::getOpenFileName(this, "Load Reminder", QDir::homePath(), "Json file (*.json)");
    if (path.length() == 0)
    {
        return;
    }

    if (!QFileInfo::exists(path))
    {
        QMessageBox::critical(this, "Error", "File not exists.");
        return;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray json = file.readAll();

    Document doc;
    doc.Parse(json.data(), json.length());

    if (doc.HasParseError())
    {
        QMessageBox::critical(this, "Error", "Can't parse data.");
        return;
    }

    if (!doc.HasMember("version") || !doc.HasMember("reminds"))
    {
        QMessageBox::critical(this, "Error", "Missing data.");
        return;
    }

    if (QString::fromUtf8(doc["version"].GetString(), doc["version"].GetStringLength()) != "1.2")
    {
        QMessageBox::critical(this, "Error", "Version don't match.");
        return;
    }

    reminders.clear();
    ui->listWidget->clear();

    GenericArray array = doc["reminds"].GetArray();
    for (auto& v : array)
    {
        QString sub_json = v["data"].GetString();
        Reminder* rem = new Reminder(this);
        connect(
            rem, SIGNAL(timeout(bool,QString)),
            this, SLOT(timeout(bool,QString))
        );
        rem->from_json(sub_json);
        QString msg = rem->get_data().message;
        reminders.insert(msg, rem);
        ui->listWidget->addItem(msg);
    }

    QMessageBox::information(this, "info", "Load success.");
}

void MainWindow::save_clicked()
{
    using rapidjson::StringBuffer;
    using rapidjson::Writer;

    if (reminders.empty())
    {
        QMessageBox::information(this, "info", "Nothing to save.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "Save Reminds", QDir::homePath() + "/reminder.json", "Json file (*.json)");
    if (path.length() == 0)
    {
        return;
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();

    writer.String("version");
    writer.String("1.2");

    writer.String("reminds");
    writer.StartArray();
    for (Reminder* item : qAsConst(reminders))
    {
        writer.StartObject();
        writer.String("data");
        writer.String(item->to_json().toUtf8());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    QString json = QString::fromUtf8(buffer.GetString(), (int)buffer.GetLength());
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(json.toUtf8());
    file.waitForBytesWritten(10000);
    file.close();

    QMessageBox::information(this, "info", "Save success.");
}

void MainWindow::audio_settings()
{
    Dialog_audio_settings* dialog = new Dialog_audio_settings(this);
    dialog->setup_data(audio_configs);
    int res = dialog->exec();
    if (res == QDialog::Accepted)
    {
        audio_configs = dialog->get_data();
        if (current_config)
        {
            current_config->set_audio(audio_configs);
        }
        this->update_audio_map();
        this->save_audio_settings();
    }
    dialog->deleteLater();
}
