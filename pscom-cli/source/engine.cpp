#include "engine.h"

#include <cmath>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QString>

#include <pscom.h>

#include "main.h"
#include "support.h"
#include "verbosity.h"


#define QDATETIME_MIN QDateTime()
#define QDATETIME_MAX QDateTime::fromSecsSinceEpoch(185542587187199999)


PscomEngine::PscomEngine(PscomApp & app, IPscomCore & core)
    : _app(&app), _core(&core)
{
}

void PscomEngine::pscom(
    const QList<QString> & arguments,
    int argOffset
) const {
    const auto adapter = dynamic_cast<PscomAdapter*>(_core);
    if (adapter == nullptr) {
        qFatal("Cannot access library in dry-run mode");
    }
    auto stream = _app->cout();
    adapter->pscom(arguments, argOffset, stream);
}

void PscomEngine::showVersion() const {
    const auto version = _core->version();
    _app->cout() << version << Qt::endl;
}

void PscomEngine::showSupportedFormats() const {
    const auto supportedFormats = _core->supportedFormats();
    for (auto format : supportedFormats) {
        _app->cout() << format << Qt::endl;
    }
}

void PscomEngine::findFiles(
    QString directory, bool recursive,
    const std::optional<QDateTime> & dateMin,
    const std::optional<QDateTime> & dateMax,
    const std::optional<QRegExp> & regex
) {
    QStringList allFiles;

    if (directory == "-") {
        if (bool(dateMin) || bool(dateMax) || bool(regex)) {
            qFatal("Filter options are not available for standard input");
        }
        qDebug() << "Reading file list from standard input ..." << Qt::endl;
        allFiles = readFileList(_app->cin());
    } else {
        qDebug() << "Searching files according to filters ..." << Qt::endl;
        allFiles = searchFiles(directory, recursive, dateMin, dateMax, regex);
    }
    qDebug().noquote()
        << QString("Found %1 matching files").arg(allFiles.size())
        << Qt::endl;

    const auto formats = _core->supportedFormats();
    for (const auto file : allFiles) {
        if (_core->supportsFile(file)) {
            _files.append(file);
        }
    }
}

const QStringList PscomEngine::readFileList(QTextStream stream) const {
    QStringList files;
    while (!stream.atEnd()) {
        files.append(stream.readLine());
    }
    return files;
}

const QStringList PscomEngine::searchFiles(
    const QString & directory, bool recursive,
    const std::optional<QDateTime> & dateMin,
    const std::optional<QDateTime> & dateMax,
    const std::optional<QRegExp> & regex) const
{
    QList<QStringList> fileLists;
    if (bool(dateMin) || bool(dateMax)) {
        fileLists.append(_core->findFiles(
            directory,
            dateMin.value_or(QDATETIME_MIN),
            dateMax.value_or(QDATETIME_MAX),
            recursive));
    }
    if (regex) {
        fileLists.append(_core->findFiles(
            directory,
            regex.value(),
            recursive));
    }
    if (fileLists.isEmpty()) {
        return _core->findFiles(directory, QRegExp(".*"), recursive);
    }
    if (fileLists.size() > 1) {
        qDebug("Intersecting files from %i sources", fileLists.size());
    }
    return intersection(fileLists);
}

void PscomEngine::processFiles(
    std::function<void(const QString &)> function
) const {
    const auto n = _files.size();

    const auto verbosityLevel = getVerbosityLevel();
    if (verbosityLevel < VerbosityLevel::Info) {
        for (const auto file : _files) {
            function(file);
        }
        return;
    }

    int i = 0;
    if (_app->isCerrInteractive()
        // debug outputs could conflict with progress bar
        && verbosityLevel <= VerbosityLevel::Debug
    ) {
        const auto shades = QString("░▒▓"); /* Inspired by
https://github.com/James-Yu/LaTeX-Workshop/blob/
c03ef00241944c4c481f22a5e28d440b61b2fb66/src/components/buildinfo.ts#L226 */
        auto stream = _app->cerr();
        const auto maxWidth = 20;
        for (const auto file : _files) {
            const auto width = static_cast<double>(++i) / n;
            auto bar = shades.right(1).repeated(floor(width * maxWidth));
            const auto rem = fmod(width, 1);
            if (rem) {
                bar += shades.at(floor(rem * shades.size()));
            }
            bar += shades.left(1).repeated(
                maxWidth - floor(width * maxWidth) - 1);
            stream << QString(
                "[%1] (%2/%3)\r"
            ).arg(bar).arg(i).arg(n);
            stream.flush();
            function(file);
        }
        stream << '\n';
    } else {
        for (const auto file : _files) {
            const auto utf8 = QString(
                    "Processing \"%1\" ... (%2/%3)"
                ).arg(file).arg(++i).arg(n)
                .toUtf8();
            qInfo("%s", utf8.constData());
            function(file);
        }
    }
}

void PscomEngine::listFiles() const {
    for (auto file : _files) {
        _app->cout() << file << Qt::endl;
    }
};

void PscomEngine::copyFiles(const QString & target) {
    _core->assertDirectory(target);
    const auto dir = QDir(target); // WORKAROUND: we should not use QFileInfo etc.
    processFiles([&](const QString & file){
        const auto newPath = dir.filePath(QFileInfo(file).fileName());
        copyFile(file, newPath);
    });
};

void PscomEngine::moveFiles(const QString & target) {
    _core->assertDirectory(target);
    const auto dir = QDir(target); // WORKAROUND: we should not use QFileInfo etc.
    processFiles([&](const QString & file){
        const auto newPath = dir.filePath(QFileInfo(file).fileName());
        moveFile(file, newPath);
    });
};

void PscomEngine::renameFiles(const QString & schema) {
    processFiles([&](const QString & file){
        const auto date = _core->getDate(file);
        const auto newPath = _core->makeFilePath(file, date, schema);
        moveFile(file, newPath);
    });
};

void PscomEngine::groupFiles(const QString & schema) {
    processFiles([&](const QString & file){
        const auto date = _core->getDate(file);
        const auto newPath = _core->makeDirectoryPath(file, date.date(), schema);
        if (file == newPath) { return; }

        const auto dir = QFileInfo(newPath).absolutePath();
        if (!_core->existsDirectory(dir)) { // WORKAROUND: we should not use QFileInfo etc.
                                            // If this is not ok, we could call createDirectory once initially, but this would make many restrictions ...
            _core->createDirectory(newPath);
        }
        moveFile(file, newPath);
    });
};

void PscomEngine::resizeFiles(int width, int height) const {
    if (width == -1 && height == -1) {
        qFatal("Either width or height must be specified");
    }

    std::function<void(const QString &)> function;
    if (width != -1 && height != -1) {
        function = [&](const QString & file){
            _core->scaleImage(file, width, height);
        };
    } elif (width != -1) {
        assert(height == -1);
        function = [&](const QString & file){
            _core->scaleImageIntoWidth(file, width);
        };
    } elif (height != -1) {
        function = [&](const QString & file){
            _core->scaleImageIntoHeight(file, height);
        };
    } else {
        UNREACHABLE;
    }
    processFiles(function);
}

void PscomEngine::convertFiles(QString format, int quality) {
    if (format == nullptr && quality == -1) {
        qFatal("No conversion options specified");
    }

    if (format != nullptr) {
        _core->assertFormat(format);
    }
    processFiles([&](const QString & file){
        const auto newPath = _core->makeSuffix(file, format);
        if (!denyExists(newPath)) { return; }
        _core->convertImage(file, format, quality);
    });
}

void PscomEngine::copyFile(const QString & oldPath, const QString & newPath) {
    if (oldPath == newPath) { return; }
    if (!denyExists(newPath)) { return; }
    _core->copyFile(oldPath, newPath);
}

void PscomEngine::moveFile(const QString & oldPath, const QString & newPath) {
    if (oldPath == newPath) { return; }
    if (!denyExists(newPath)) { return; }
    _core->moveFile(oldPath, newPath);
}

bool PscomEngine::denyExists(const QString & path) {
    if (!_core->exists(path)) { return true; }

    switch (getFileExistsReaction(path)) {
        case FileExistsReaction::Skip:
            return false;
        case FileExistsReaction::Overwrite:
            _core->removeFile(path);
            return true;
        case FileExistsReaction::Backup:
            {
                const auto backupPath = path + "~";
                if (!denyExists(backupPath)) {
                    return false;
                }
                _core->moveFile(path, backupPath);
                return true;
            }
        default:
            UNREACHABLE;
    }
}

FileExistsReaction PscomEngine::getFileExistsReaction(
    const QString & path
) {
    if (fileExistsReaction.has_value()) {
        return fileExistsReaction.value();
    }

    const auto message = QString("File already exists: \"%1\"").arg(path);
    if (!_app->isCinInteractive()) {
        const auto utf8 = message.toUtf8();
        qFatal("%s", utf8.constData());
    };
    switch (_app->interactiveRequest(message, {
        {'o', "overwrite"},
        {'O', "overwrite all"},
        {'s', "skip"},
        {'S', "skip all"},
        {'b', "backup"},
        {'B', "backup all"}
    })) {
        case 2:
            fileExistsReaction = FileExistsReaction::Overwrite;
            [[fallthrough]];
        case 1:
            return FileExistsReaction::Overwrite;
        case 4:
            fileExistsReaction = FileExistsReaction::Skip;
            [[fallthrough]];
        case 3:
            return FileExistsReaction::Skip;
        case 6:
            fileExistsReaction = FileExistsReaction::Backup;
            [[fallthrough]];
        case 5:
            return FileExistsReaction::Backup;
    }
    UNREACHABLE;
}
