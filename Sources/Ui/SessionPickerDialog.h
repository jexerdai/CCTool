#pragma once
#include <QDialog>
#include <QString>
#include "../Core/CCSessionReader.h"

class QListWidget;

class SessionPickerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SessionPickerDialog(const QList<CCSession>& sessions,
                                 const QString& lastSessionId,
                                 QWidget* parent = nullptr);

    QString selectedSessionId() const;

private:
    QListWidget* m_list;
};
