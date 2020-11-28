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


PscomEngine::PscomEngine(PscomCli &app)
    : _app(&app)
{
}

void PscomEngine::pscom(QList<QString> arguments, int argOffset) const {
    auto argCounter = argOffset;
    if (argCounter >= arguments.length()) {
        _app->showError("usage: pscom <symbol> <argument>*");
    }
    const auto symbol = arguments[argCounter++];

{// INSANE_MACRO_MAGIC_TO_COMPENSATE_THE_ABSENCE_OF_REFLECTION_IN_CPP_AAAH
    #define ASSERT_ARGC(NUMBER) \
        if (arguments.length() - 1 - argOffset != NUMBER) { \
        _app->showError(QString( \
            "Invalid number of arguments (expected %1, but got %2)" \
        ).arg(NUMBER).arg(arguments.length() - argOffset - 1)); \
    }

    #define COMMA() ,
    #define EMPTY()
    #define _INIT_ARG(I, X, C) auto arg ## I = X(arguments[argCounter++]);
    #define _PRN_ARG(I, X, C) arg ## I C()
    #define _STRM_ARG(I, X, C) << arg ## I C
    #define HANDLE_SYMBOL(NAME, ...) if (symbol == #NAME) { \
        ASSERT_ARGC(__VA_NARG__(__VA_ARGS__)); \
        _FOR_EACH(_INIT_ARG, EMPTY, EMPTY, ##__VA_ARGS__); \
        _app->cout() << "< " << symbol; \
        qInfo().nospace() << "(" _FOR_EACH( \
                _STRM_ARG, << ", ", , ##__VA_ARGS__ \
            ) << ")"; \
        qInfo().nospace() << "> " << pscom::NAME( \
            _FOR_EACH(_PRN_ARG, COMMA, EMPTY, ##__VA_ARGS__)); \
    }

    #define TYPE_BOOL [](QString s) { return QVariant(s).toBool(); }
    #define TYPE_INT [](QString s) { return s.toInt(); }
    #define TYPE_QDATE(DATE_TYPE) [](QString s){ \
        return DATE_TYPE::fromString(s, Qt::ISODateWithMs); \
    }

    if (false); // syntactic sugar for indentation only
    else HANDLE_SYMBOL(vi)
    else HANDLE_SYMBOL(sf)
    else HANDLE_SYMBOL(de, QString)
    else HANDLE_SYMBOL(fe, QString)
    else HANDLE_SYMBOL(ne, QString)
    else HANDLE_SYMBOL(fs, QString)
    else HANDLE_SYMBOL(cs, QString, QString)
    else HANDLE_SYMBOL(fn, QString, TYPE_QDATE(QDateTime), QString)
    else HANDLE_SYMBOL(fp, QString, TYPE_QDATE(QDate), QString)
    else HANDLE_SYMBOL(mv, QString, QString)
    else HANDLE_SYMBOL(cp, QString, QString)
    else HANDLE_SYMBOL(rm, QString)
    else HANDLE_SYMBOL(mk, QString)
    else HANDLE_SYMBOL(et, QString)
    else HANDLE_SYMBOL(sw, QString, TYPE_INT)
    else HANDLE_SYMBOL(sh, QString, TYPE_INT)
    else HANDLE_SYMBOL(ss, QString, TYPE_INT, TYPE_INT)
    else HANDLE_SYMBOL(cf, QString, QString, TYPE_INT)
    else HANDLE_SYMBOL(re, QString, QRegExp, TYPE_BOOL)
    else HANDLE_SYMBOL(dt, QString, TYPE_QDATE(QDateTime), TYPE_QDATE(QDateTime), TYPE_BOOL)
    else {
        _app->showError("pscom: " + QString("Unknown symbol: ") + symbol);
    }
}//
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

void PscomEngine::renameFiles(QString schema) const {
    for (auto file : _files) {
        const auto date = pscom::et(file);
        const auto newName = pscom::fn(file, date, schema);
        qDebug() << "mv" << file << newName;
        if (file != newName) {
            pscom::mv(file, newName);
        }
    }
};

void PscomEngine::groupFiles(QString schema) const {
    for (auto file : _files) {
        const auto date = pscom::et(file);
        const auto newName = pscom::fp(file, date.date(), schema);
        if (file == newName) {
            continue;
        }
        const auto dir = QFileInfo(newName).absolutePath();
        if (!pscom::de(dir)) {
            qDebug() << "mkdir" << dir;
            pscom::mk(newName);
        }
        qDebug() << "mv" << file << newName;
        pscom::mv(file, newName);
    }
};

void PscomEngine::resizeFiles(int width, int height) const {
    if (width == -1 && height == -1) {
        _app->showError("Either width or height must be specified");
    }
    if (width != -1 && height != -1) {
        for (auto file : _files) {
            pscom::ss(file, width, height);
        }
    } else if (width != -1) {
        assert(height == -1);
        for (auto file : _files) {
            pscom::sw(file, width);
        }
    } else if (height != -1) {
        assert(width == -1);
        for (auto file : _files) {
            pscom::sh(file, height);
        }
    } else {
        assert(false); // Should not reach here
    }
}

void PscomEngine::convertFiles(QString format, int quality) const {
    if (format == nullptr && quality == -1) {
        _app->showError("No conversion options specified");
    }
    if (format == nullptr) {
        for (auto file : _files) {
            // Workaround with double conversion to compensate inconvenient
            // pscom interface. See
            // https://moodle.hpi3d.de/mod/forum/discuss.php?d=2135#p4151.
            const auto oldFormat = pscom::fs(file);
            const auto tmpFormat = oldFormat == "jpg" ? "png" : "jpg";
            assert(tmpFormat != oldFormat);
            qDebug() << "cf" << file << tmpFormat << quality;
            pscom::cf(file, tmpFormat, quality);
            const auto tmpFile = pscom::cs(file, tmpFormat);
            qDebug() << "rm" << file;
            pscom::rm(file);
            qDebug() << "cf" << tmpFile << oldFormat << quality;
            pscom::cf(tmpFile, oldFormat, quality);
            qDebug() << "rm" << tmpFile;
            pscom::rm(tmpFile);
        }
    } else {
        for (auto file : _files) {
            qDebug() << "cf" << file << format << quality;
            pscom::cf(file, format, quality);
        }
    }
}
