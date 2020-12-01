#pragma once

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include <pscom.h>

#include "adapter.h"
#include "main.h"
#include "support.h"


class PscomEngine {
private:
    PscomApp *_app;
    IPscomCore *_core;
    QStringList _files;

    void copyFile(const QString & oldPath, const QString & newPath) const;
    void moveFile(const QString & oldPath, const QString & newPath) const;
public:
    PscomEngine(PscomApp & app, IPscomCore & core);

    void pscom(const QList<QString> & arguments, int argOffset = 0) const;
    void showSupportedFormats() const;
    void showVersion() const;
    void listFiles() const;
    void copyFiles(const QString & target) const;
    void moveFiles(const QString & target) const;
    void renameFiles(const QString & schema) const;
    void groupFiles(const QString & schema) const;
    void resizeFiles(int width = -1, int height = -1) const;
    void convertFiles(QString format = nullptr, int quality = -1) const;

    void findFiles(
        QString directory = ".",
        bool recursive = false,
        const std::optional<QDateTime> & dateMin = std::nullopt,
        const std::optional<QDateTime> & dateMax = std::nullopt,
        const std::optional<QRegExp> & regex = std::nullopt);
    const QStringList readFileList(QTextStream stream) const;
    const QStringList searchFiles(
        const QString & directory,
        bool recursive,
        const std::optional<QDateTime> & dateMin,
        const std::optional<QDateTime> & dateMax,
        const std::optional<QRegExp> & regex) const;
};
