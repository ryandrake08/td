#include "TournamentDocument.hpp"
#include "TBRuntimeError.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

#include <stdexcept>

struct TournamentDocument::impl
{
    // file name associated with window/document
    QString filename;

    // configuration represented by the window/document
    QVariantMap configuration;
};

TournamentDocument::TournamentDocument() : pimpl(new impl)
{
}

TournamentDocument::~TournamentDocument() = default;

// load document from file
bool TournamentDocument::load(const QString& filename)
{
    // create and open file
    QFile file_obj(filename);
    if(!file_obj.open(QFile::ReadOnly | QFile::Text))
    {
        // handle file open failure
        throw TBRuntimeError(QObject::tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(filename), file_obj.errorString()));
    }

    // read from file
    auto json_data(file_obj.readAll());

    // close file
    file_obj.close();

    // convert to json document
    auto json_doc(QJsonDocument::fromJson(json_data));

    // ensure root of document is an object
    if(!json_doc.isObject())
    {
        // handle invalid json
        throw TBRuntimeError(QObject::tr("Invalid document %1").arg(QDir::toNativeSeparators(filename)));
    }

    // convert to json object
    auto json_obj(json_doc.object());

    // convert to variant map
    this->pimpl->configuration = json_obj.toVariantMap();

    // record filename
    this->pimpl->filename = filename;

    // success
    return true;
}

// save document, returning false if document has no filename
bool TournamentDocument::save() const
{
    // check filename
    auto filename(this->pimpl->filename);
    if(filename.isEmpty())
    {
        // return false on empty filename (window should prompt)
        return false;
    }

    // convert from variant map
    auto json_obj(QJsonObject::fromVariantMap(this->pimpl->configuration));

    // convert to json document
    QJsonDocument json_doc(json_obj);

    // ensure root of document is an object
    if(!json_doc.isObject())
    {
        // handle invalid json
        throw TBRuntimeError(QObject::tr("Invalid document"));
    }

    // convert to data
    auto json_data(json_doc.toJson());

    // create and open file
    QFile file_obj(filename);
    if(!file_obj.open(QFile::WriteOnly | QFile::Text))
    {
        // handle file open failure
        throw TBRuntimeError(QObject::tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(filename), file_obj.errorString()));
    }

    // write to file
    file_obj.write(json_data);

    // close file
    file_obj.close();

    // success
    return true;
}

// save document as file
bool TournamentDocument::save_as(const QString& filename)
{
    // set new filename
    this->pimpl->filename = filename;

    // save
    return this->save();
}
