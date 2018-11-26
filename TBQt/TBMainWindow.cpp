#include "TBMainWindow.hpp"
#include "ui_TBMainWindow.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QString>
#include <QVariant>

#include <QDebug>

struct TBMainWindow::impl
{
    // file name associated with window/document
    QString filename;

    // configuration represented by the window/document
    QVariantHash configuration;

    // load configuration from a file
    void load_configuration(const QString& file)
    {
        // create and open file
        QFile file_obj(file);
        if(!file_obj.open(QIODevice::ReadOnly))
        {
            // TODO: handle file open failure
        }

        // read from file
        auto json_data(file_obj.readAll());

        // close file
        file_obj.close();

        // convert to json document
        auto json_doc(QJsonDocument::fromJson(json_data));

        // check for valid json
        if(json_doc.isNull())
        {
            // TODO: handle json parse error
        }

        // ensure root of document is an object
        if(!json_doc.isObject())
        {
            // TODO: handle json not an object
        }

        // convert to json object
        auto json_obj(json_doc.object());

        // check for non-empty object
        if(json_obj.empty())
        {
            // TODO: handle json object is empty
        }

        // convert to variant map
        this->configuration = json_obj.toVariantHash();

        // record filename
        this->filename = file;
    }

    void save_configuration()
    {
        // convert from variant map
        auto json_obj(QJsonObject::fromVariantHash(this->configuration));

        // check for non-empty object
        if(json_obj.empty())
        {
            // TODO: handle json object is empty
        }

        // convert to json document
        QJsonDocument json_doc(json_obj);

        // check for valid json
        if(json_doc.isNull())
        {
            // TODO: handle json parse error
        }

        // ensure root of document is an object
        if(!json_doc.isObject())
        {
            // TODO: handle json not an object
        }

        // convert to data
        auto json_data(json_doc.toJson());

        // check filename
        if(this->filename.isEmpty())
        {
            // TODO: handle empty filename (prompt user?)
        }

        // create and open file
        QFile file_obj(this->filename);
        if(!file_obj.open(QIODevice::WriteOnly))
        {
            // TODO: handle file open failure
        }

        // write to file
        file_obj.write(json_data);

        // close file
        file_obj.close();
    }

public:
    impl(const QString& file)
    {
        // load file if not empty
        if(!file.isEmpty())
        {
            load_configuration(file);
        }
    }
};

TBMainWindow::TBMainWindow(const QString& file) : pimpl(new impl(file)), ui(new Ui::TBMainWindow)
{
    // set up moc
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setUnifiedTitleAndToolBarOnMac(true);
}

TBMainWindow::~TBMainWindow() = default;

void TBMainWindow::on_actionExit_triggered()
{
    this->close();
}
