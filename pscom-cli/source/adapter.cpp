#include "adapter.h"

#include <QDebug>

#include <pscom.h>

#include "support.h"


#define PSCOM pscom

#define COMMA() ,
#define EMPTY()
#define _INIT_ARG(I, X, C) auto arg ## I = (X);
#define _PRN_ARG(I, X, C) arg ## I C()
#define _STRM_ARG(I, X, C) << arg ## I C
#define _PSCOM(SYMBOL, ...) [&](){ \
    _FOR_EACH(_INIT_ARG, , , ##__VA_ARGS__); \
    qDebug().nospace() << STRINGIFY(PSCOM) "::" #SYMBOL << "(" _FOR_EACH( \
        _STRM_ARG, << ", ", , ##__VA_ARGS__ \
    ) << ")"; \
    return PSCOM::SYMBOL( \
        _FOR_EACH(_PRN_ARG, COMMA, EMPTY, ##__VA_ARGS__)); \
}()


void PscomAdapter::pscom(
    const QList<QString> & arguments,
    int argOffset,
    QTextStream & outputStream
) const {
    auto argCounter = argOffset;
    if (argCounter >= arguments.length()) {
        qFatal("usage: %s <symbol> <argument>*", STRINGIFY(PSCOM));
    }
    const auto symbol = arguments[argCounter++];

{// INSANE_MACRO_MAGIC_TO_COMPENSATE_THE_ABSENCE_OF_REFLECTION_IN_CPP_AAAH
    #define ASSERT_ARGC(NUMBER) \
        if (arguments.length() - 1 - argOffset != NUMBER) { \
        qFatal("Invalid number of arguments (expected %i, but got %i)", \
            NUMBER, arguments.length() - argOffset - 1); \
    }

    #define _INIT_ARG_2(I, X, C) auto arg ## I = X(arguments[argCounter++]);
    #define _STRM_ARG_2(I, X, C) + variantToString(arg ## I) C
    #define HANDLE_SYMBOL(NAME, ...) if (symbol == #NAME) { \
        ASSERT_ARGC(__VA_NARG__(__VA_ARGS__)); \
        _FOR_EACH(_INIT_ARG_2, EMPTY, EMPTY, ##__VA_ARGS__); \
        outputStream << "< " << symbol + "(" _FOR_EACH( \
                _STRM_ARG_2, + ", ", , ##__VA_ARGS__ \
            ) + ")" << '\n'; \
        outputStream << "> " << variantToString(pscom::NAME( \
            /* NOTE: It would be nice to reuse _PSCOM here, but then macro \
               expansion won't work as expected ... ;-( */ \
            _FOR_EACH(_PRN_ARG, COMMA, EMPTY, ##__VA_ARGS__))) << '\n'; \
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
    else HANDLE_SYMBOL(dt, QString, TYPE_QDATE(QDateTime),
        TYPE_QDATE(QDateTime), TYPE_BOOL)
    else {
        const auto utf8 = symbol.toUtf8();
        qFatal("%s: Unknown symbol: %s", STRINGIFY(PSCOM), utf8.constData());
    }
}//
}

const QString PscomAdapter::version(void) const {
    return _PSCOM(vi);
}

const QStringList PscomAdapter::supportedFormats(void) const {
    return _PSCOM(sf);
}

void PscomAdapter::copyFile(
    const QString & source, const QString & destination
) const {
    assertFile(source);
    if (source == destination) { return; }
    denyExists(destination);

    if (_PSCOM(cp, source, destination)) { return; }

    const auto
        utf8Source = source.toUtf8(),
        utf8Destination = destination.toUtf8();
    qFatal(
        "Could not copy file: \"%s\" to: \"%s\"",
        utf8Source.constData(), utf8Destination.constData());
}

void PscomAdapter::createDirectory(const QString & pathInDirectory) const {
    denyExists(pathInDirectory);
    if (_PSCOM(mk, pathInDirectory)) { return; }

    const auto utf8 = pathInDirectory.toUtf8();
    qFatal("Could not create directory: \"%s\"", utf8.constData());
}

bool PscomAdapter::exists(const QString & path) const {
    return !_PSCOM(ne, path);
}

bool PscomAdapter::existsDirectory(const QString & path) const {
    return _PSCOM(de, path);
}

bool PscomAdapter::existsFile(const QString & path) const {
    return _PSCOM(fe, path);
}

const QStringList PscomAdapter::findFiles(
    const QString & path,
    const QDateTime & dateMin,
    const QDateTime & dateMax,
    bool recursive
) const {
    assertDirectory(path);
    return _PSCOM(dt, path, dateMin, dateMax, recursive);
}

const QStringList PscomAdapter::findFiles(
    const QString & path, const QRegExp & regex, bool recursive
) const {
    assertDirectory(path);
    return _PSCOM(re, path, regex, recursive);
}

const QDateTime PscomAdapter::getDate(const QString & path) const {
    assertFile(path);
    return _PSCOM(et, path);
}

const QString PscomAdapter::getSuffix(const QString & path) const {
    assertFile(path);
    return _PSCOM(fs, path);
}

const QString PscomAdapter::makeDirectoryPath(
    const QString & path,
    const QDate & date,
    const QString & format
) const {
    return _PSCOM(fp, path, date, format);
}

const QString PscomAdapter::makeFilePath(
    const QString & path,
    const QDateTime & dateTime,
    const QString & format
) const {
    return _PSCOM(fn, path, dateTime, format);
}

const QString PscomAdapter::makeSuffix(
    const QString & path,
    const QString & format
) const {
    assertFormat(format);
    return _PSCOM(cs, path, format);
}

void PscomAdapter::moveFile(
    const QString & source, const QString & destination
) const {
    assertFile(source);
    if (source == destination) { return; }
    denyExists(destination);

    if (_PSCOM(mv, source, destination)) { return; }

    const auto
        utf8Source = source.toUtf8(),
        utf8Destination = destination.toUtf8();
    qFatal(
        "Could not move file: \"%s\" to: \"%s\"",
        utf8Source.constData(), utf8Destination.constData());
}

bool PscomAdapter::supportsFile(const QString & path) const {
    const auto format = getSuffix(path);
    return supportsFormat(format);
}

bool PscomAdapter::supportsFormat(const QString & format) const {
    const auto formats = supportedFormats();
    return formats.contains(format, Qt::CaseInsensitive);
}

void PscomAdapter::removeFile(const QString & path) const {
    if (!exists(path)) { return; }
    if (!(existsFile(path) || existsDirectory(path))) {
        const auto utf8 = path.toUtf8();
        qFatal("Cannot remove non-file path: %s", utf8.constData());
    }

    if (_PSCOM(rm, path)) { return; }
    
    const auto utf8 = path.toUtf8();
    qFatal("Could not remove file: %s", utf8.constData());
}

const QString PscomAdapter::convertImage(
    const QString & path,
    const QString & suffix,
    int quality
) const {
    // TODO: Handle same format separately!
    if (suffix == nullptr) {
        // Workaround with double conversion to compensate inconvenient pscom
        // interface. See
        // https://moodle.hpi3d.de/mod/forum/discuss.php?d=2135#p4151.
        const auto oldSuffix = getSuffix(path);
        const auto formats = supportedFormats();
        auto tmpSuffix = formats[0];
        if (oldSuffix == tmpSuffix) {
            tmpSuffix = formats[1];
        }
        assert(tmpSuffix != oldSuffix);
        const auto tmpFile = convertImage(
            path, tmpSuffix, quality);
        assert(existsFile(tmpFile));
        removeFile(path);
        const auto newPath = convertImage(tmpFile, oldSuffix, quality);
        removeFile(tmpFile);
        return newPath;
    }

    assertFile(path);
    assertFormat(suffix);
    const auto newPath = makeSuffix(path, suffix);
    denyExists(newPath); // TODO: Allow to overwrite

    if (_PSCOM(cf, path, suffix, quality)) { return newPath; }

    const auto utf8 = path.toUtf8();
    qFatal("Could not convert file: %s", utf8.constData());
}        

void PscomAdapter::scaleImage(
    const QString & path, int width, int height
) const {
    assertFileFormat(path);

    if (_PSCOM(ss, path, width, height)) { return; }

    const auto utf8 = path.toUtf8();
    qFatal("Could not resize file: \"%s\"", utf8.constData());
}

void PscomAdapter::scaleImageIntoWidth(
    const QString & path, int width
) const {
    assertFileFormat(path);

    if (_PSCOM(sw, path, width)) { return; }

    const auto utf8 = path.toUtf8();
    qFatal("Could not resize file: \"%s\"", utf8.constData());
}

void PscomAdapter::scaleImageIntoHeight(
    const QString & path, int height
) const {
    assertFileFormat(path);

    if (_PSCOM(sh, path, height)) { return; }

    const auto utf8 = path.toUtf8();
    qFatal("Could not resize file: \"%s\"", utf8.constData());
}

void PscomAdapter::assertExists(const QString & path) const {
    if (exists(path)) { return; }
    const auto utf8 = path.toUtf8();
    qFatal("Path does not exist: \"%s\"", utf8.constData());
}

void PscomAdapter::assertDirectory(const QString & path) const {
    if (existsDirectory(path)) { return; }
    assertExists(path); // for differentiated error message
    const auto utf8 = path.toUtf8();
    qFatal("Directory does not exist: \"%s\"", utf8.constData());
}

void PscomAdapter::assertFile(const QString & path) const {
    if (existsFile(path)) { return; }
    assertExists(path); // for differentiated error message
    const auto utf8 = path.toUtf8();
    qFatal("File does not exist: \"%s\"", utf8.constData());
}

void PscomAdapter::assertFileFormat(const QString & path) const {
    if (supportsFile(path)) { return; }

    const auto utf8 = path.toUtf8();
    qFatal("File format not supported: \"%s\"", utf8.constData());
}

void PscomAdapter::assertFormat(const QString & format) const {
    if (supportsFormat(format)) { return; }

    const auto utf8 = format.toUtf8();
    qFatal("Image format not supported: \"%s\"", utf8.constData());
}

void PscomAdapter::denyExists(const QString & path) const {
    if (!exists(path)) { return; }
    const auto utf8 = path.toUtf8();
    qFatal("File already exists: \"%s\"", utf8.constData());
}

PscomSimulator::PscomSimulator(
    IPscomCore & core, const std::function<QTextStream(void)> stream)
    : _core(&core), _stream(stream)
{
}

void PscomSimulator::log(const QString & message) const {
    _stream() << message << Qt::endl;
}

const QString PscomSimulator::version(void) const {
    return _core->version();
}

const QStringList PscomSimulator::supportedFormats(void) const {
    return _core->supportedFormats();
}

void PscomSimulator::assertDirectory(const QString & path) const {
    _core->assertDirectory(path);
}

void PscomSimulator::assertFormat(const QString & format) const {
    _core->assertFormat(format);
}

void PscomSimulator::copyFile(
    const QString & source, const QString & destination
) const {
    log(QString("copy \"%1\" \"%2\"").arg(source).arg(destination));
}

void PscomSimulator::createDirectory(const QString & pathInDirectory) const {
    log(QString("create directory for \"%1\"").arg(pathInDirectory));
}

bool PscomSimulator::exists(const QString & path) const {
    return _core->exists(path);
}

bool PscomSimulator::existsDirectory(const QString & path) const {
    return _core->existsDirectory(path);
}

bool PscomSimulator::existsFile(const QString & path) const {
    return _core->existsFile(path);
}

const QStringList PscomSimulator::findFiles(
    const QString & path,
    const QDateTime & dateMin,
    const QDateTime & dateMax,
    bool recursive
) const {
    return _core->findFiles(path, dateMin, dateMax, recursive);
}

const QStringList PscomSimulator::findFiles(
    const QString & path, const QRegExp & regex, bool recursive
) const {
    return _core->findFiles(path, regex, recursive);
}

const QDateTime PscomSimulator::getDate(const QString & path) const {
    return _core->getDate(path);
}

const QString PscomSimulator::getSuffix(const QString & path) const {
    return _core->getSuffix(path);
}

const QString PscomSimulator::makeDirectoryPath(
    const QString & path,
    const QDate & date,
    const QString & format
) const {
    return _core->makeDirectoryPath(path, date, format);
}

const QString PscomSimulator::makeFilePath(
    const QString & path,
    const QDateTime & dateTime,
    const QString & format
) const {
    return _core->makeFilePath(path, dateTime, format);
}

const QString PscomSimulator::makeSuffix(
    const QString & path,
    const QString & format
) const {
    return _core->makeSuffix(path, format);
}

void PscomSimulator::moveFile(
    const QString & source, const QString & destination
) const {
    log(QString("move \"%1\" \"%2\"").arg(source).arg(destination));
}

bool PscomSimulator::supportsFile(const QString & path) const {
    return _core->supportsFile(path);
}

void PscomSimulator::removeFile(const QString & path) const {
    log(QString("remove \"%1\"").arg(path));
}

const QString PscomSimulator::convertImage(
    const QString & path, const QString & suffix, int quality
) const {
    auto logString = QString("convert \"%1\"");
    if (suffix != nullptr) {
        logString += " to %2";
    }
    if (quality != -1) {
        logString += " with quality=%3";
    }
    log(logString.arg(path).arg(suffix).arg(quality));

    return makeSuffix(path, suffix);
}

void PscomSimulator::scaleImage(
    const QString & path, int width, int height
) const {
    log(QString("scale \"%1\" to %2x%3").arg(path).arg(width).arg(height));
}

void PscomSimulator::scaleImageIntoWidth(
    const QString & path, int width
) const {
    log(QString("scale \"%1\" to width=%2").arg(path).arg(width));
}

void PscomSimulator::scaleImageIntoHeight(
    const QString & path, int height
) const {
    log(QString("scale \"%1\" to height=%2").arg(path).arg(height));
}
