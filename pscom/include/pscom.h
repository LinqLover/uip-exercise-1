#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

#include "pscom-api.h"


/* PLEASE NOTE
 * - naming of functions is intentionally bad to avoid bias in command naming/denotation as much as possible.
 * - do not alter this library since it WILL BE REPLACED FOR SCORING!
 * - if something is missing or does not work adhere to the forum or contact daniel.limberger@hpi.de
 * - this is a fabricated facade over basic QT functionality and is intended as a working mock of a potentially
 * much more complex library that is to be controlled by the CLI (fake common real life scenarios).
 * - all functions are designed to be self contained and atomic, i.e., images have to be first checked for
 * existence, then for support, then can be scaled (but should probably copied first since scaling overrides), and
 * could finally be converted and/or adjusted in quality (this could be easily improved in order to reduce read write
 * operations and improve the overall processing speed but is kept stupid and simple, again intentionally).
 */

namespace pscom {

/**
 * @brief Fromated application version string.
 * Provides a fromated application version string `%1 version %2 pscom-%3 qt-%4` comprising
 * - application name (%1),
 * - application version (%2),
 * - pscom library version (%3, internal), and
 * - qt version (%4).
 * Note that version should be set via QCoreApplication by the caller.
 * @return Formated version string, e.g., `pscom-cli version 0.0.0 -- pscom-1.0.0 qt-5.11.2`
 */
PSCOMSHARED_EXPORT QString vi();

/**
 * @brief Supported image formats that can be processed.
 * Provides a list of filed endings of supported image file formats. This list depends on the qt build as well as on
 * what image plugins could be loaded at run-time.
 * @return List of supported formats.
 */
PSCOMSHARED_EXPORT QStringList sf();

/**
 * @brief Checks whether a path exists and points to a directory.
 * @param source - Path that is to be checked.
 * @return True iff the path exists and points to a directory. False otherwise.
 */
PSCOMSHARED_EXPORT bool de(const QString & path);

/**
 * @brief Checks whether a path exists and points to a file.
 * @param source - Path that is to be checked.
 * @return True iff the path exists and points to a file. False otherwise.
 */
PSCOMSHARED_EXPORT bool fe(const QString & path);

/**
 * @brief Checks if a path does not exists.
 * @param source - Path that is to be checked for non existence.
 * @return True iff the path does not exists. False otherwise.
 */
PSCOMSHARED_EXPORT bool ne(const QString & path);

/**
 * @brief Returns the suffix (extension) of the file.
 * Note: terminates if path does not exists or path does not point to a file.
 * @param source - Path pointing to a file with extension.
 * @return The file's suffix, e.g., "gz" for "/tmp/archive.tar.gz".
 */
PSCOMSHARED_EXPORT QString fs(const QString & path);

/**
 * @brief Changes the suffix of a file path to the provided suffix.
 * Note: terminates if suffix is not supported.
 * @param path - Path pointing to an image file.
 * @param suffix - Suffix to use for new/destination file path.
 * @return Complete path with its suffix replaced by provided suffix.
 */
PSCOMSHARED_EXPORT QString cs(const QString & path, const QString & suffix);

/**
 * @brief Creates a new base name for the given path based on the date-time and format.
 * @param path - Path used to resolve path and complete suffix from.
 * @param dateTime - Date-time used for deriving the base file name.
 * @param format - Format used to derive the date-time based base file name.
 * @return Given path with the based name replaced by date-time identifier based on format.
 */
PSCOMSHARED_EXPORT QString fn(const QString & path, const QDateTime & dateTime, const QString & format);

/**
 * @brief Inserts an additional path to the given path based on the date and format.
 * @param path - Path used to resolve file name (base name and suffix) from.
 * @param date - Date used for deriving the additional path.
 * @param format - Format used to derive the additional, date based path name.
 * @return Given file path with extended by the date based path.
 */
PSCOMSHARED_EXPORT QString fp(const QString & path, const QDate & date, const QString & format);

/**
 * @brief Moves, or renames a file or directory.
 * Note: terminates if source does not exist or destination does already exist.
 * @param source - Path pointing to a file or directory that is to be moved/renamed.
 * @param destination - Path to move/rename the file or directory to.
 * @return True if the operation was successful. False otherwise.
 */
PSCOMSHARED_EXPORT bool mv(const QString & source, const QString & destination);

/**
 * @brief Copies a file from source path to destination path.
 * Note: terminates if source does not exist and is not a file or destination already exists.
 * @param source - Path pointing to an existing file.
 * @param destination - Path denoting the target destination.
 * @return True if copy operation was successful. False otherwise.
 */
PSCOMSHARED_EXPORT bool cp(const QString & source, const QString & destination);

/**
 * @brief Removes a file or directory. If directory, all parent directories within path will be removed.
 * Note: terminates if path does not exist or points neither to a file nor to a directory.
 * @param path - Path pointing to a file or directory that is to be removed.
 * @return True if removed successfully. False otherwise.
 */
PSCOMSHARED_EXPORT bool rm(const QString & path);

/**
 * @brief Creates a directory. If a file path is provided, the full path without file name is created.
 * Note: terminates if destination does already exist.
 * @param path - Path that is to be created as new diretory.
 * @return True if the operation was successful. False otherwise.
 */
PSCOMSHARED_EXPORT bool mk(const QString & path);

/**
 * @brief Returns the earliest date and time available for this file indicating creation date.
 * This returns the first date of birth time (file creation), last modified (sometimes earlier than created due to
 * weird copy operations), or metadata change time (perhaps last resort of true data, captured by camera...).
 * Note: terminates if path does not exists or path does not point to a file.
 * @param path - Path that for which the birth time is to be queried.
 * @return The date and time when the file was created.
 */
PSCOMSHARED_EXPORT QDateTime et(const QString & path);

/**
 * @brief Loads, scales, and saves/overwrites the image at path to the provided width.
 * Smooth transformation is used (slow) and maximum quality while saving is applied (large files).
 * Note: terminates if file could not be loaded (check if supported first) or file does not exist.
 * @param path - Path pointing to an image file.
 * @param width - Target width the image should be scaled to (aspect ratio is kept and smooth transformation used).
 * @return True if the image was scaled and saved successfully. False otherwise (if not scaled or written).
 */
PSCOMSHARED_EXPORT bool sw(const QString & path, int width);

/**
 * @brief Loads, scales, and saves/overwrites the image at path to the provided height.
 * Smooth transformation is used (slow) and maximum quality while saving is applied (large files).
 * Note: terminates if file could not be loaded (check if supported first) or file does not exist.
 * @param path - Path pointing to an image file.
 * @param height - Target height the image should be scaled to (aspect ratio is kept and smooth transformation used).
 * @return True if the image was scaled and saved successfully. False otherwise (if not scaled or written).
 */
PSCOMSHARED_EXPORT bool sh(const QString & path, int height);

/**
 * @brief Loads, scales, and saves/overwrites the image at path to the provided size.
 * For the transformation the aspect ratio is kept and the image will resize to the best fit within the target size.
 * Furthermore, smooth transformation is used (slow) and maximum quality while saving is applied (large files).
 * Note: terminates if file could not be loaded (check if supported first) or file does not exist.
 * @param path - Path pointing to an image file.
 * @param width - Target width the image should be scaled to.
 * @param height - Target height the image should be scaled to.
 * @return True if the image was scaled and saved successfully. False otherwise (if not scaled or written).
 */
PSCOMSHARED_EXPORT bool ss(const QString & path, int width, int height);

/**
 * @brief Changes the image format of the given image (at path) to the format derived from suffix.
 * This loads the image and stores a new image with the new suffix at the same location. The olf image remains.
 * Note: termiantes if image could not be loaded, or new suffixed path already exists, or suffix is not supported.
 * @param path - Path pointing to an image file.
 * @param suffix - New suffix to be used to derive the targetd format and image's file path.
 * @param quality - Quality of the image from 0 to 100, minimum to maximum respectively.
 * @return True if converting and saving succeeded. False otherwise.
 */
PSCOMSHARED_EXPORT bool cf(const QString & path, const QString & suffix, int quality);

/**
 * @brief Creates a list of filepaths within the directory pointed to by path, matching the regular expression.
 * Note: terminates if path does not exist or does not point to a directory.
 * @param path - Path pointing to a directory, that is to be iterated and searched for regex matching files.
 * @param regex - Regex to be used: files are only considered on exact match.
 * @param recursive - Whether or not to recursively iterate subdirectories.
 * @return List of filenames matching the given regex for the given path/directory.
 */
PSCOMSHARED_EXPORT QStringList re(const QString & path, const QRegExp & regex, bool recursive);


/**
 * @brief Creates a list of filepaths of files with their earliest time beeing within the given time range.
 * Note: terminates if path does not exist or does not point to a directory.
 * @param path - Path pointing to a directory, that is to be iterated and searched for regex matching files.
 * @param t0 - Start data-time.
 * @param t1 - End date-time.
 * @param recursive - Whether or not to recursively iterate subdirectories.
 * @return List of filenames with their earliest time beeing within the given time range for the given path/directory.
 */
PSCOMSHARED_EXPORT QStringList dt(const QString & path, const QDateTime & t0, const QDateTime & t1, bool recursive);

}


