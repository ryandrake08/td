#pragma once

#include <QString>
#include <stdexcept>

class TBRuntimeError : public std::runtime_error
{
    QString message;

public:
    explicit TBRuntimeError(const QString& message);

    // return explanatory QString
    QString q_what() const;
};
