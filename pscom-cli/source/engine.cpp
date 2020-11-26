#include "engine.h"


#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QString>

#include <pscom.h>

#include "main.h"
#include "support.h"


#define QDATETIME_MIN QDateTime()
#define QDATETIME_MAX QDateTime::fromSecsSinceEpoch(185542587187199999)


PscomEngine::PscomEngine(PscomCli &app)
    : _app(&app)
{
}

int PscomEngine::showVersion() const {
    _app->cout() << pscom::vi() << Qt::endl;
    return EXIT_SUCCESS;
}

int PscomEngine::showSupportedFormats() const {
    auto supportedFormats = pscom::sf();
    for (auto format : supportedFormats) {
        _app->cout() << format << Qt::endl;
    }
    return EXIT_SUCCESS;
}

void PscomEngine::findFiles(
    QString directory, bool recursive,
    std::optional<QDateTime> dateMin,
    std::optional<QDateTime> dateMax,
    std::optional<QRegExp> regex
) {
    if (directory == "-") {
        if (bool(dateMin) || bool(dateMax) || bool(regex)) {
            _app->showError("Filter options and stdin cannot be combined");
        }
        QTextStream cin(stdin);
        while (!cin.atEnd()) {
            _files.append(cin.readLine());
        }
        return;
    }

    QList<QStringList> fileLists;
    if (bool(dateMin) || bool(dateMax)) {
        fileLists.append(pscom::dt(
            directory,
            dateMin.value_or(QDATETIME_MIN),
            dateMax.value_or(QDATETIME_MAX),
            recursive));
    }
    if (regex) {
        fileLists.append(pscom::re(
            directory,
            regex.value(),
            recursive));
    }
    _files = fileLists.isEmpty()
        ? pscom::re(directory, QRegExp(".*"), recursive)
        : intersection(fileLists);
}

void PscomEngine::listFiles() const {
    for (auto file : _files) {
        _app->cout() << file << Qt::endl;
    }
};

void PscomEngine::copyFiles(QString target) const {
    const auto path = target + QDir::separator();
    for (auto file : _files) {
        pscom::cp(file, path + QFileInfo(file).fileName());
    }
};

void PscomEngine::moveFiles(QString target) const {
    const auto path = target + QDir::separator();
    for (auto file : _files) {
        pscom::mv(file, path + QFileInfo(file).fileName());
    }
};
