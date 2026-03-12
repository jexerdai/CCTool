#pragma once
#include <QString>
#include <QMetaType>

struct RepoInfo {
    int     id   = -1;
    QString name;
    QString path;

    bool isValid() const { return id >= 0 && !path.isEmpty(); }
};

Q_DECLARE_METATYPE(RepoInfo)
