#ifndef DIGITIZEREXCEPTION_H
#define DIGITIZEREXCEPTION_H

#include <QException>
#include <QString>

// Exception for Digitizer methods
class DigitizerException : public QException
{
private:
    QString errorMessage;
public:

    DigitizerException(QString msg);

    QString GetErrorMessage();
};

#endif // DIGITIZEREXCEPTION_H
