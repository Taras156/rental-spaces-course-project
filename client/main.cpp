#include <QApplication>
#include <QMessageBox>

#include "Network/singletonclient.h"
#include "Views/authwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SingletonClient::getInstance()->connectToServer("127.0.0.1", 33334);

    AuthWindow window;
    window.show();

    return app.exec();
}
