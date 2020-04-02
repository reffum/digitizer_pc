#include "digitizerexception.h"

DigitizerException::DigitizerException(QString msg)
{
    errorMessage = msg;
}

QString DigitizerException::GetErrorMessage()
{
    return errorMessage;
}
