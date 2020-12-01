#pragma once

#include <QDateTime>
#include <QRegExp>
#include <QString>
#include <QTextStream>


class IPscomCore {
public:
    virtual const QString version(void) const = 0;
    virtual const QStringList supportedFormats(void) const = 0;

    virtual void assertDirectory(const QString & path) const = 0;
    virtual void assertFormat(const QString & format) const = 0;
    virtual void copyFile(
        const QString & source,
        const QString & destinationDirectory) const = 0;
    virtual void createDirectory(const QString & pathInDirectory) const = 0;
    virtual bool exists(const QString & path) const = 0;
    virtual bool existsDirectory(const QString & path) const = 0;
    virtual bool existsFile(const QString & path) const = 0;
    virtual const QStringList findFiles(
        const QString & path,
        const QDateTime & dateMin,
        const QDateTime & dateMax,
        bool recursive) const = 0;
    virtual const QStringList findFiles(
        const QString & path,
        const QRegExp & regex,
        bool recursive) const = 0;
    virtual const QDateTime getDate(const QString & path) const = 0;
    virtual const QString getSuffix(const QString & path) const = 0;
    virtual const QString makeDirectoryPath(
        const QString & path,
        const QDate & date,
        const QString & format) const = 0;
    virtual const QString makeFilePath(
        const QString & path,
        const QDateTime & dateTime,
        const QString & format) const = 0;
    virtual const QString makeSuffix(
        const QString & path,
        const QString & format) const = 0;
    virtual void moveFile(
        const QString & source,
        const QString & destinationDirectory) const = 0;
    virtual bool supportsFile(const QString & path) const = 0;
    virtual void removeFile(const QString & path) const = 0;

    virtual const QString convertImage(
        const QString & path,
        const QString & suffix,
        int quality = -1) const = 0;
    virtual void scaleImage(
        const QString & path, int width, int height) const = 0;
    virtual void scaleImageIntoWidth(
        const QString & path, int width) const = 0;
    virtual void scaleImageIntoHeight(
        const QString & path, int height) const = 0;
};

class PscomAdapter : public IPscomCore {
public:
    void pscom(
        const QList<QString> & arguments,
        int argOffset,
        QTextStream & outputStream
    ) const;
    const QString version(void) const;
    const QStringList supportedFormats(void) const;

    void assertDirectory(const QString & path) const;
    void assertFormat(const QString & format) const;
    void copyFile(
        const QString & source, const QString & destinationDirectory) const;
    void createDirectory(const QString & pathInDirectory) const;
    bool exists(const QString & path) const;
    bool existsDirectory(const QString & path) const;
    bool existsFile(const QString & path) const;
    const QStringList findFiles(
        const QString & path,
        const QDateTime & dateMin,
        const QDateTime & dateMax,
        bool recursive) const;
    const QStringList findFiles(
        const QString & path,
        const QRegExp & regex,
        bool recursive) const;
    const QDateTime getDate(const QString & path) const;
    const QString getSuffix(const QString & path) const;
    const QString makeDirectoryPath(
        const QString & path,
        const QDate & date,
        const QString & format) const;
    const QString makeFilePath(
        const QString & path,
        const QDateTime & dateTime,
        const QString & format) const;
    const QString makeSuffix(
        const QString & path,
        const QString & format) const;
    void moveFile(
        const QString & source, const QString & destinationDirectory) const;
    bool supportsFile(const QString & path) const;
    void removeFile(const QString & path) const;

    const QString convertImage(
        const QString & path,
        const QString & suffix,
        int quality = -1) const;
    void scaleImage(const QString & path, int width, int height) const;
    void scaleImageIntoWidth(const QString & path, int width) const;
    void scaleImageIntoHeight(const QString & path, int height) const;
private:
    void assertExists(const QString & path) const;
    void assertFile(const QString & path) const;
    void assertFileFormat(const QString & path) const;
    void denyExists(const QString & path) const;
    bool supportsFormat(const QString & format) const;
};
