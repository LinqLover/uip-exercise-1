#include "engine.h"


#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QString>

#include <pscom.h>

#include "main.h"
#include "support.h"


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
            qFatal("Filter options and stdin cannot be combined");
        }
        allFiles = readFileList(_app->cin());
    } else {
        allFiles = searchFiles(directory, recursive, dateMin, dateMax, regex);
    }

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
    return fileLists.isEmpty()
        ? _core->findFiles(directory, QRegExp(".*"), recursive)
        : intersection(fileLists);
}

void PscomEngine::listFiles() const {
    for (auto file : _files) {
        _app->cout() << file << Qt::endl;
    }
};

void PscomEngine::copyFiles(const QString & target) const {
    _core->assertDirectory(target);
    const auto dir = QDir(target); // WORKAROUND: we should not use QFileInfo etc.
    for (auto file : _files) {
        const auto newPath = dir.filePath(QFileInfo(file).fileName());
        copyFile(file, newPath);
    }
};

void PscomEngine::moveFiles(const QString & target) const {
    _core->assertDirectory(target);
    const auto dir = QDir(target); // WORKAROUND: we should not use QFileInfo etc.
    for (auto file : _files) {
        const auto newPath = dir.filePath(QFileInfo(file).fileName());
        moveFile(file, newPath);
    }
};

void PscomEngine::renameFiles(const QString & schema) const {
    for (auto file : _files) {
        const auto date = _core->getDate(file);
        const auto newPath = _core->makeFilePath(file, date, schema);
        moveFile(file, newPath);
    }
};

void PscomEngine::groupFiles(const QString & schema) const {
    for (auto file : _files) {
        const auto date = _core->getDate(file);
        const auto newPath = _core->makeDirectoryPath(file, date.date(), schema);
        if (file == newPath) { continue; }

        const auto dir = QFileInfo(newPath).absolutePath();
        if (!_core->existsDirectory(dir)) { // WORKAROUND: we should not use QFileInfo etc.
                                            // If this is not ok, we could call createDirectory once initially, but this would make many restrictions ...
            _core->createDirectory(newPath);
        }
        moveFile(file, newPath);
    }
};

void PscomEngine::resizeFiles(int width, int height) const {
    if (width == -1 && height == -1) {
        qFatal("Either width or height must be specified");
    }

    if (width != -1 && height != -1) {
        for (auto file : _files) {
            _core->scaleImage(file, width, height);
        }
    } else if (width != -1) {
        assert(height == -1);
        for (auto file : _files) {
            _core->scaleImageIntoWidth(file, width);
        }
    } else if (height != -1) {
        assert(width == -1);
        for (auto file : _files) {
            _core->scaleImageIntoHeight(file, height);
        }
    } else {
        assert(false); // Should not reach here
    }
}

void PscomEngine::convertFiles(QString format, int quality) const {
    if (format == nullptr && quality == -1) {
        qFatal("No conversion options specified");
    }

    if (format != nullptr) {
        _core->assertFormat(format);
    }
    for (auto file : _files) {
        const auto newPath = _core->makeSuffix(file, format);
        if (_core->exists(newPath)) {
            const auto utf8 = newPath.toUtf8();
            qFatal("File already exists: \"%s\"", utf8.constData()); // TODO: Add interactivity
            // TODO: If confirmed, delete old file ...
        }
        _core->convertImage(file, format, quality);
    }
}

void PscomEngine::copyFile(
    const QString & oldPath, const QString & newPath
) const {
    if (oldPath == newPath) { return; }
    if (_core->exists(newPath)) {
        const auto utf8 = newPath.toUtf8();
        qFatal("File already exists: \"%s\"", utf8.constData()); // TODO: Add interactivity
        // TODO: If confirmed, delete old file ...
    }
    _core->copyFile(oldPath, newPath);
}

void PscomEngine::moveFile(
    const QString & oldPath, const QString & newPath
) const {
    if (oldPath == newPath) { return; }
    if (_core->exists(newPath)) {
        const auto utf8 = newPath.toUtf8();
        qFatal("File already exists: \"%s\"", utf8.constData()); // TODO: Add interactivity
        // TODO: If confirmed, delete old file ...
    }
    _core->moveFile(oldPath, newPath);
}
