#pragma once

#include <memory>

class QString;

class TournamentDocument
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    TournamentDocument();
    ~TournamentDocument();

    // load document from file
    bool load(const QString& filename);

    // save document, returning false if document has no filename
    bool save() const;

    // save document as file
    bool save_as(const QString& filename);
};