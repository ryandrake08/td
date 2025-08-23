#include "TBRuntimeError.hpp"

#include <QString>

TBRuntimeError::TBRuntimeError(const QString& message) : std::runtime_error(message.toLocal8Bit().constData()), message(message)
{
}

QString TBRuntimeError::q_what() const
{
    return this->message;
}
