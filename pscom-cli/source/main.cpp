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
    auto app = PscomApp(argc, argv);

    PscomCommandLineParser parser(app);
    parser.setApplicationDescription("Photo system command-line tool");
    parser._showVersionCallback = std::bind(
        &PscomEngine::showVersion, PscomEngine(app));

    QCommandLineOption optionSupportedFormats(
        QStringList{"supported-formats"},
        "Display all supported image formats."
    );
    QCommandLineOption optionDirectory(
        QStringList{"d", "C", "directory"},
        "The directory to look up image files.",
        "path"
    );
    QCommandLineOption optionRecursive(
        QStringList{"R", "recursive"},
        "Include subdirectories."
    );
    QCommandLineOption optionRegex(
        QStringList{"r", "regex"},
        QString(
            "A regular expression to filter image files. Must not match the "
            "entire file name; use text anchors (%1) for full matches."
        ).arg("^ $"),
        "pattern"
    );
    QCommandLineOption optionMinDate(
        QStringList{"mi", "min-date"},
        "Reject images older than the given date and time.",
        "date"
    );
    QCommandLineOption optionMaxDate(
        QStringList{"ma", "max-date"},
        "Reject images newer than the given date and time. NOTE: If you only "
        "specify the date, it will be treated as midnight time.",
        "date"
    );
    QCommandLineOption optionWidth(
        QStringList{"max-width"},
        "the width the images should be fit into",
        "number"
    );
    QCommandLineOption optionHeight(
        QStringList{"max-height"},
        "The height the images should be fit into.",
        "number"
    );
    QCommandLineOption optionFormat(
        QStringList{"format"},
        "The file format (e.g. jpg or png) the images should be converted "
        "into.",
        "extension"
    );
    QCommandLineOption optionQuality(
        QStringList{"quality"},
        "The quality for image conversion. Value between 0 (best compression) "
        "and 100 (best quality).",
        "value"
    );
    assert(parser.addOption(optionSupportedFormats));
    assert(parser.addOption(optionDirectory));
    assert(parser.addOption(optionRecursive));
    assert(parser.addOption(optionRegex));
    assert(parser.addOption(optionMinDate));
    assert(parser.addOption(optionMaxDate));
    assert(parser.addOption(optionWidth));
    assert(parser.addOption(optionHeight));
    assert(parser.addOption(optionFormat));
    assert(parser.addOption(optionQuality));

    PscomCommand commandPscom(
        QStringList{"pscom"},
        QStringList{"symbol", "arguments"},
        "Execute a symbol from the pscom library manually. No safety checks! "
        "Intended for development use only.",
        [](PscomEngine &engine){ engine.pscom(QStringList{}); });
    PscomCommand commandList(
        QStringList{"list", "ls"},
        QStringList{},
        "Display image files.",
        &PscomEngine::listFiles);
    PscomCommand commandCopy(
        QStringList{"copy", "cp"},
        QStringList{"destination"},
        "Copy image files into the specified destination folder.",
        &PscomEngine::copyFiles);
    PscomCommand commandMove(
        QStringList{"move", "mv"},
        QStringList{"destination"},
        "Move image files into the specified destination folder.",
        &PscomEngine::moveFiles);
    const auto escapeNote = QString(
        "To escape format selectors in the schema, enclose constant parts "
        "into single quotes (%1, or %2 from the bash shell)."
    ).arg("'").arg("\"'\"");
    PscomCommand commandRename(
        QStringList{"rename", "rn"},
        QStringList{"schema"},
        "Rename image files according to the given schema, or according to "
        "the UPA standard, if omitted. " + escapeNote,
        &PscomEngine::renameFiles,
        QStringList{"yyyyMMdd_HHmmsszzz"});
    PscomCommand commandGroup(
        QStringList{"group", "g"},
        QStringList{"schema"},
        "Group image files into subdirectories according to the given "
        "schema, or according to the UPA standard, if omitted. " + escapeNote,
        &PscomEngine::groupFiles,
        QStringList{"yyyy/yyyy-MM"});
    PscomCommand commandResize(
        QStringList{"resize"},
        QStringList{},
        "Resize image files into the given dimensions.",
        [&parser, &optionWidth, &optionHeight](PscomEngine &engine){
            engine.resizeFiles(
                parser.isSet(optionWidth)
                    ? parser.value(optionWidth).toInt()
                    : -1,
                parser.isSet(optionHeight)
                    ? parser.value(optionHeight).toInt()
                    : -1);
        });
    PscomCommand commandConvert(
        QStringList{"convert"},
        QStringList{},
        "Convert image files into a different file format and/or quality.",
        [&parser, &optionFormat, &optionQuality](PscomEngine &engine){
            engine.convertFiles(
                parser.isSet(optionFormat)
                    ? parser.value(optionFormat)
                    : nullptr,
                parser.isSet(optionQuality)
                    ? parser.value(optionQuality).toInt()
                    : -1);
        });
    parser.addHelpCommand();
    parser.addVersionCommand();
    parser.addCommand(commandPscom);
    parser.addCommand(commandList);
    parser.addCommand(commandCopy);
    parser.addCommand(commandMove);
    parser.addCommand(commandRename);
    parser.addCommand(commandGroup);
    parser.addCommand(commandResize);
    parser.addCommand(commandConvert);

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
        ? std::make_optional(QRegExp(".*" + parser.value(optionRegex) + ".*"))
        : std::nullopt;

    engine.findFiles(directory, recursive, dateMin, dateMax, regex);

    parser.runCommand(engine);

    return EXIT_SUCCESS;
}


PscomApp::PscomApp(int &argc, char *argv[]) :
    QCoreApplication(argc, argv)
{
}

PscomApp::~PscomApp()
{
}

QTextStream PscomApp::cout() const {
    return QTextStream(stdout);
}

QTextStream PscomApp::cerr() const {
    return QTextStream(stderr);
}
