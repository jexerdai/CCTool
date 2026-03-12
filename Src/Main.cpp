#include <QApplication>
#include "MainWindow.h"
#include "Data/Database.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("CCTool");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("CCTool");

    if (!Database::instance().init()) {
        return 1;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
