#include "pscom.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QLibraryInfo>
#include <QRegExp>
#include <QVersionNumber>
#include <QString>


namespace pscom {


static const auto VERSION = QVersionNumber(0, 1, 0);

QString vi()
{
    return QString("%1 version %2 | pscom-%3 qt-%4")
        .arg(QCoreApplication::applicationName())
        .arg(QCoreApplication::applicationVersion())
        .arg(pscom::VERSION.toString())
        .arg(QLibraryInfo::version().toString());
}


QStringList sf()
{
    const auto list = QImageWriter::supportedImageFormats();
    auto suffixes = QStringList();
    foreach (const auto & suffix, list) {
        suffixes.push_back(QString::fromLocal8Bit(suffix));
    }
    return suffixes;
}


bool de(const QString & path)
{
    const auto fi = QFileInfo(path);
    return fi.exists() && fi.isDir();
}


bool fe(const QString & path)
{
    const auto fi = QFileInfo(path);
    return fi.exists() && fi.isFile();
}


bool ne(const QString & path)
{
    const auto fi = QFileInfo(path);
    return !fi.exists();
}


QString fs(const QString & path)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }
    return fi.suffix();
}


QString cs(const QString & path, const QString & suffix)
{
    if(!sf().contains(suffix)) {
        std::terminate();
    }

    const auto fi = QFileInfo(path);
    return fi.path() + QDir::separator() + fi.completeBaseName() + '.' + suffix;
}


QString fn(const QString & path, const QDateTime & dateTime, const QString & format)
{
    const auto fi = QFileInfo(path);
    return fi.path() + QDir::separator() + dateTime.toString(format) + '.' + fi.completeSuffix();
}


QString fp(const QString & path, const QDate & date, const QString & format)
{
    const auto fi = QFileInfo(path);
    return fi.path() + QDir::separator() + date.toString(format) + QDir::separator() + fi.fileName();
}


bool mv(const QString & source, const QString & destination)
{
    const auto sourceInfo = QFileInfo(source);
    if(!sourceInfo.exists()) {
        std::terminate();
    }
    if(!(sourceInfo.isFile() || sourceInfo.isDir())){
        std::terminate();
    }

    const auto destinationInfo = QFileInfo(destination);
    if(destinationInfo.exists()) {
        std::terminate();
    }

    return sourceInfo.isDir() ?
        QDir(source).rename(source, destination) : QFile(source).rename(source, destination);
}


bool cp(const QString & source, const QString & destination)
{
    const auto sourceInfo = QFileInfo(source);
    if(!sourceInfo.exists() || !sourceInfo.isFile()) {
        std::terminate();
    }

    const auto destinationInfo = QFileInfo(destination);
    if(destinationInfo.exists()) {
        std::terminate();
    }

    return QFile::copy(source, destination);
}


bool rm(const QString & path)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists()) {
        std::terminate();
    }
    if(!(fi.isFile() || fi.isDir())){
        std::terminate();
    }

    if(fi.isFile()) {
        return QFile::remove(path);
    }
    return QDir().rmpath(path);
}


bool mk(const QString & path)
{
    const auto destinationInfo = QFileInfo(path);
    if(destinationInfo.exists()) {
        std::terminate();
    }

    return QDir().mkpath(destinationInfo.path());
}


QDateTime et(const QString & path)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }
    const auto t0 = fi.birthTime();
    const auto t1 = fi.lastModified();
    const auto t2 = fi.metadataChangeTime();

    if(t0 < t1 && t0 < t2) {
        return t0;
    }
    if(t1 < t2) {
        return t1;
    }
    return t2;
}


bool sw(const QString & path, const int width)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }

    const auto image = QImage(path);
    if(image.isNull()) {
        std::terminate();
    }

    const auto scaled = image.scaledToWidth(width, Qt::TransformationMode::SmoothTransformation);
    return scaled.save(path, nullptr, 100);
}


bool sh(const QString & path, const int height)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }

    const auto image = QImage(path);
    if(image.isNull()) {
        std::terminate();
    }

    const auto scaled = image.scaledToHeight(height, Qt::TransformationMode::SmoothTransformation);
    return scaled.save(path, nullptr, 100);
}


bool ss(const QString & path, const int width, const int height)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }

    const auto image = QImage(path);
    if(image.isNull()) {
        std::terminate();
    }

    const auto scaled = image.scaled(width, height,
        Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
    return scaled.save(path, nullptr, 100);
}


bool cf(const QString & path, const QString & suffix, const int quality)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isFile()) {
        std::terminate();
    }

    const auto image = QImage(path);
    if(image.isNull()) {
        std::terminate();
    }

    const auto destination = cs(path, suffix);
    const auto destinationInfo = QFileInfo(destination);
    if(destinationInfo.exists()) {
        std::terminate();
    }

    return image.save(destination, nullptr, quality);
}


QStringList re(const QString & path, const QRegExp & regex, const bool recursive)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isDir()) {
        std::terminate();
    }

    auto files = QStringList();

    QDirIterator it(path, QDir::Files, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        const auto file = it.next();
        if(!regex.exactMatch(file)) {
            continue;
        }
        files.push_back(file);
    }
    return files;
}


QStringList dt(const QString & path, const QDateTime & t0, const QDateTime & t1, bool recursive)
{
    const auto fi = QFileInfo(path);
    if(!fi.exists() || !fi.isDir()) {
        std::terminate();
    }

    auto files = QStringList();

    QDirIterator it(path, QDir::Files, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        const auto file = it.next();
        const auto tf = et(file);
        if(tf < t0 || t1 < tf) {
            continue;
        }
        files.push_back(file);
    }
    return files;
}


}
