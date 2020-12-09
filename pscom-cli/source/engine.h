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


enum FileExistsReaction {
    Skip,
    Overwrite,
    Backup
};

// Engine object for pscom-cli that provides all main file operations and
// maintains all state that is relevant during the execution.
class PscomEngine {
public:
    PscomEngine(PscomApp & app, IPscomCore & core);

    std::optional<FileExistsReaction> fileExistsReaction;

    void pscom(const QList<QString> & arguments, int argOffset = 0) const;
    void showSupportedFormats() const;
    void showVersion() const;
    void listFiles() const;
    void copyFiles(const QString & target);
    void moveFiles(const QString & target);
    void renameFiles(const QString & schema);
    void groupFiles(const QString & schema);
    void resizeFiles(int width = -1, int height = -1);
    void convertFiles(QString format = nullptr, int quality = -1);

    void findFiles(
        QString directory = ".",
        bool recursive = false,
        const std::optional<QDateTime> & dateMin = std::nullopt,
        const std::optional<QDateTime> & dateMax = std::nullopt,
        const QRegExp & regex = QRegExp());
private:
    PscomApp *_app;
    IPscomCore *_core;
    QStringList _files;

    bool copyFile(const QString & oldPath, const QString & newPath);
    bool moveFile(const QString & oldPath, const QString & newPath);
    bool confirmOverwrite(const QString & path);
    bool denyExists(const QString & path);
    FileExistsReaction getFileExistsReaction(
        const QString & message,
        const QString & path);
    void processFiles(
        std::function<bool(const QString &)> function,
        const QString & completeMessage = nullptr) const;
    const QStringList readFileList(QTextStream stream) const;
    const QStringList searchFiles(
        const QString & directory,
        bool recursive,
        const std::optional<QDateTime> & dateMin,
        const std::optional<QDateTime> & dateMax,
        const QRegExp & regex) const;
};
