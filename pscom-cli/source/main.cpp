#include "main.h"

#include <functional>
#include <optional>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QDebug>
#include <QRegExp>
#include <QTextStream>

#include <QDir>
#include <QFileInfo>

#include "command.h"
#include "engine.h"
#include "verbosity.h"


/* PLEASE NOTE
 * - be careful using the pscom library, it may irreversible delete actual data if used without care.
 * - try to rely on pscom library exclusively in order to fulfill the user stories / tasks.
 * - use qt only to drive the library and control command language aspects and CLI related features, e.g.,
 *   progress, logging, verbosity, undo, help, etc.
 * - prefer qDebug, qInfo, qWarning, qCritical, and qFatal for verbosity and console output.
 */


int main(int argc, char *argv[])
{
    qInstallMessageHandler(VerbosityHandler);

    // Setting the application name is not required, since, if not set, it defaults to the executable name.
    // QCoreApplication::setApplicationName("pscom-cli");
    QCoreApplication::setApplicationVersion("1.0.0");
    auto app = PscomCli(argc, argv); // TODO: Rename into PscomApp?

    PscomCommandLineParser parser(app);
    parser.setApplicationDescription("Photo system command-line tool");
    parser._showVersion = std::bind(&PscomEngine::showVersion, PscomEngine(app));

    QCommandLineOption optionSupportedFormats(
        QStringList{"supported-formats"},
        "Displays all supported image formats."
    );
    QCommandLineOption optionDirectory(
        QStringList{"d", "C", "directory"},
        "The directory where the files should be searched.",
        "directory"
    );
    QCommandLineOption optionRecursive(
        QStringList{"r", "recursive"},
        "Search subdirectories"
    );
    QCommandLineOption optionRegex(
        QStringList{"re", "regex"},
        "Filter files by regex",
        "regex"
    );
    QCommandLineOption optionMinDate(
        QStringList{"mi", "min-date"},
        "Reject images older than",
        "date"
    );
    QCommandLineOption optionMaxDate(
        QStringList{"ma", "max-date"},
        "Reject images newer than",
        "date"
    );
    assert(parser.addOption(optionSupportedFormats));
    assert(parser.addOption(optionDirectory));
    assert(parser.addOption(optionRecursive));
    assert(parser.addOption(optionRegex));
    assert(parser.addOption(optionMinDate));
    assert(parser.addOption(optionMaxDate));

    PscomCommand commandPscom(
        QStringList{"pscom"},
        QStringList{},
        "pscom with a better description",
        [](PscomEngine &engine){ engine.pscom(QStringList{}); });
    PscomCommand commandList(
        QStringList{"list", "ls"},
        QStringList{},
        "list files with a better description",
        &PscomEngine::listFiles);
    PscomCommand commandCopy(
        QStringList{"copy", "cp"},
        QStringList{"destination"},
        "copy files with a better description",
        &PscomEngine::copyFiles);
    PscomCommand commandMove(
        QStringList{"move", "mv"},
        QStringList{"destination"},
        "move files with a better description",
        &PscomEngine::moveFiles);
    PscomCommand commandRename(
        QStringList{"rename", "rn"},
        QStringList{"schema"},
        "rename files with a better description",
        &PscomEngine::renameFiles,
        QStringList{"yyyyMMdd_HHmmsszzz"});
    PscomCommand commandGroup(
        QStringList{"group", "g"},
        QStringList{"schema"},
        "rename files with a better description",
        &PscomEngine::groupFiles,
        QStringList{"yyyy/yyyy-MM"});
    parser.addHelpCommand();
    parser.addVersionCommand();
    parser.addCommand(commandPscom);
    parser.addCommand(commandList);
    parser.addCommand(commandCopy);
    parser.addCommand(commandMove);
    parser.addCommand(commandRename);
    parser.addCommand(commandGroup);

    parser.process();

    PscomEngine engine(app);

    if (parser.isSet(optionSupportedFormats)) {
        return engine.showSupportedFormats();
    }

    if (!parser.hasCommand()) {
        parser.showHelp(EXIT_FAILURE);
    }

    if (parser.positionalArguments().first() == "pscom") {
        engine.pscom(parser.positionalArguments(), 1);
        return EXIT_SUCCESS;
    }

    const auto directory = parser.isSet(optionDirectory)
        ? parser.value(optionDirectory)
        : ".";
    const auto recursive = parser.isSet(optionRecursive);
    const auto dateMin = parser.isSet(optionMinDate)
        ? std::make_optional(QDateTime::fromString(
            parser.value(optionMinDate), Qt::ISODateWithMs))
        : std::nullopt;
    const auto dateMax = parser.isSet(optionMaxDate)
        ? std::make_optional(QDateTime::fromString(
            parser.value(optionMaxDate), Qt::ISODateWithMs))
        : std::nullopt;
    const auto regex = parser.isSet(optionRegex)
        ? std::make_optional(QRegExp(parser.value(optionRegex)))
        : std::nullopt;

    engine.findFiles(directory, recursive, dateMin, dateMax, regex);

    parser.runCommand(engine);

    return EXIT_SUCCESS;
}


PscomCli::PscomCli(int &argc, char *argv[]) :
    QCoreApplication(argc, argv)
{
}

PscomCli::~PscomCli()
{
}

QTextStream PscomCli::cout() const {
    return QTextStream(stdout);
}

QTextStream PscomCli::cerr() const {
    return QTextStream(stderr);
}

Q_NORETURN void PscomCli::showError(QString errorText) const {
    cerr() << PscomCli::applicationName() + QLatin1String(": ")
            + errorText + QLatin1Char('\n');
    cerr().flush();
    std::exit(EXIT_FAILURE);
}
