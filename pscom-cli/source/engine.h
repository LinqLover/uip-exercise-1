#pragma once

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QString>

#include <pscom.h>

#include "main.h"
#include "support.h"


class PscomEngine {
private:
    PscomCli *_app;
    QStringList _files;
public:
    PscomEngine(PscomCli &app);

    void pscom(QList<QString> arguments, int argOffset = 0) const;
    int showSupportedFormats() const;
    int showVersion() const;
    void listFiles() const;
    void copyFiles(QString target) const;
    void moveFiles(QString target) const;
    void renameFiles(QString schema) const;

    void findFiles(
        QString directory = ".",
        bool recursive = false,
        std::optional<QDateTime> dateMin = std::nullopt,
        std::optional<QDateTime> dateMax = std::nullopt,
        std::optional<QRegExp> regex = std::nullopt);
};
