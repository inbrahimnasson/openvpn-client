#include "importconfig.h"
#include "ui_importconfig.h"

ImportConfig::ImportConfig(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ImportConfig)
{
    m_ui->setupUi(this);
}

ImportConfig::~ImportConfig()
{
    delete m_ui;
}

void ImportConfig::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ImportConfig::on_cmdOpenCryptFile_clicked()
{
    QFileDialog certFileDialog;
    QString filename = certFileDialog.getOpenFileName(this, tr("Find import file"), QApplication::applicationDirPath(), "Import files (*.crypt)");
    if (filename != "") {
        m_ui->txtImportPath->setText(filename);
    }
}

void ImportConfig::on_cmdCancel_clicked()
{
    this->close();
}

void ImportConfig::on_rbSaveAsName_toggled(bool checked)
{
    if (checked) {
        m_ui->txtNewName->setEnabled(true);
    } else {
        m_ui->txtNewName->setText("");
        m_ui->txtNewName->setEnabled(false);
    }
}

void ImportConfig::resetFields() {
    m_ui->txtImportPath->setText("");
    m_ui->txtNewName->setText("");
    m_ui->txtNewName->setEnabled(false);
    m_ui->rbSaveAsFile->setChecked(true);
}

void ImportConfig::on_cmdImport_clicked()
{
    if (m_ui->txtPassword->text() == "") {
        QMessageBox::critical(0, QString("OpenVPN Client"), QString ("No password specify!"));
        return;
    }
    if (m_ui->txtImportPath->text() != "") {
        if (m_ui->rbSaveAsName->isChecked() && m_ui->txtNewName->text() == "") {
            QMessageBox::critical(0, QString("OpenVPN Client"), QString ("No import name specify!"));
            return;
        }

        // Portale ode rinstall
        AppFunc app;
        QString dirPath;
        QString configName;

        if (!m_ui->rbSaveAsName->isChecked()) {
            configName = m_ui->txtImportPath->text().right(m_ui->txtImportPath->text().size() - m_ui->txtImportPath->text().lastIndexOf("/") -1);
            configName = configName.left(configName.size()-6);
        } else {
            configName = m_ui->txtNewName->text().trimmed();
        }

        if (!app.isAppPortable()) {
            // Installierte Version
            // Dateien ins Homeverzeichnis/securepoint/OpenVPN kopieren
            dirPath = QDir::homePath()
                       + QString("/securepoint/OpenVPN/")
                       + configName;

        } else {
            // Portable Version
            // Dateien ins App Verzeichnis /data kopieren
            dirPath = QApplication::applicationDirPath()
                       + QString("/data/")
                       + configName;
        }

        // Verzeichnis da?
        QDir dirobj (dirPath);
        if (!dirobj.exists(dirPath)){
            //Verzeichnis existiert nicht
            // Pfad erstellen
            if (!dirobj.mkpath(dirPath)) {
                // Pfad konnte nicht erstellt werden
                QMessageBox::critical(0,"Securepoint OpenVPN Client", "Unable to create directory!");
                return;
            }
        } else {
            // Verzeichnis existiert
            QMessageBox::critical(0, QString("OpenVPN Client"), QString ("A diretory with this name already exists!"));
            return;
        }
        // Datei ins neue Verzeichnis kopieren
        //QFile importFileCrypt (m_ui->txtImportPath->text());
        QString packFile = dirPath + QString("/") + configName + QString(".7z");

        QProcess packCrypt;
        QStringList argCrypt;
        QString programCrypt = "./app/bin/openssl.exe";

        argCrypt << QString("des3");
        argCrypt << QString("-in");
        argCrypt << m_ui->txtImportPath->text();
        argCrypt << QString("-out");
        argCrypt << packFile;
        argCrypt << QString("-d");
        argCrypt << QString("-salt");
        argCrypt << QString("-k");
        argCrypt << m_ui->txtPassword->text().trimmed();
        packCrypt.start(programCrypt, argCrypt);


        if (!packCrypt.waitForFinished()) {
                QMessageBox::critical(0,QString("OpenVPN Client"), QString("OpenSSL process still running!"));
                return;
        }

        QProcess packProc;
        QStringList arguments;
        QString program = "./app/bin/7za.exe";

        arguments << QString("e");
        arguments << packFile;
        arguments << QString("-p") + m_ui->txtPassword->text().trimmed();
        arguments << QString("-mhe");
        arguments << QString("-o") + dirPath;
        packProc.start(program, arguments);

        if (!packProc.waitForFinished()) {
            QMessageBox::critical(0,QString("OpenVPN CLient"), QString("7z process still running!"));
            return;
        }

        // Datei l�schen
        QFile configZip (packFile);
        if (!configZip.remove()) {
            QMessageBox::critical(0, QString("OpenVPN Client"), configZip.errorString());
        }
        if (m_ui->rbSaveAsName->isChecked()) {
            // ovpn umbennen
            QString ovpnFilePath = m_ui->txtImportPath->text().right(m_ui->txtImportPath->text().size() - m_ui->txtImportPath->text().lastIndexOf("/") -1);
                    ovpnFilePath = dirPath + QString("/") + ovpnFilePath.left(ovpnFilePath.size()-6) + QString(".ovpn");
            QFile ovpnFile (ovpnFilePath);
            qDebug() << ovpnFilePath;
            if (ovpnFile.exists()) {
                // umbenennen
                ovpnFile.rename(dirPath + QString("/") + configName + QString(".ovpn"));
            }
        }
        QMessageBox::information(0, QString("OpenVPN Client"), QString("Import successfully ended!"));
        this->close();


    } else {
        QMessageBox::critical(0, QString("OpenVPN Client"), QString ("No import file selected!"));
        return;
    }
}
