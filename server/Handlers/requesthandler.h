#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <QString>

class RequestHandler
{
public:
    static QString processRequest(const QString& request,
                                  QString& currentRole,
                                  int& currentUserId,
                                  int& currentClientId);
};

#endif // REQUESTHANDLER_H
