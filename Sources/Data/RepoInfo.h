#pragma once
#include <QString>
#include <QDateTime>
#include <QMetaType>

struct RepoInfo {
    int      id           = -1;
    QString  name;
    QString  path;
    QString  lastSessionId;  // 上次打开的 CC session id
    bool     isOpen       = false;  // 上次退出时是否打开了 Tab
    QDateTime lastActiveAt;         // 最后激活时间（用于恢复时确定焦点 Tab）

    bool isValid() const { return id >= 0 && !path.isEmpty(); }
};

Q_DECLARE_METATYPE(RepoInfo)
