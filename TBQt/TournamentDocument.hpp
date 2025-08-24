#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

class TournamentDocument : public QObject
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    TournamentDocument();
    virtual ~TournamentDocument();

    // load document from file
    bool load(const QString& filename);

    // save document, returning false if document has no filename
    bool save() const;

    // save document as file
    bool save_as(const QString& filename);

    // accessors
    const QString& filename() const;
    const QVariantMap& configuration() const;
    
    // mutators
    void setConfiguration(const QVariantMap& configuration);

Q_SIGNALS:
    void filenameChanged(const QString& name);
    void configurationChanged(const QVariantMap& config);
};
