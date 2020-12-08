#include "main.h"

#include <functional>
#include <optional>
#include <stdio.h>
#ifdef Q_OS_WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QDebug>
#include <QList>
#include <QPair>
#include <QRegExp>
#include <QTextStream>

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

#define EXIT_MISUSE 2


int main(int argc, char *argv[])
{
    qInstallMessageHandler(VerbosityHandler);

    // Setting the application name is not required, since, if not set, it defaults to the executable name.
    // QCoreApplication::setApplicationName("pscom-cli");
    QCoreApplication::setApplicationVersion("3.0.0");
    auto app = PscomApp(argc, argv);
    IPscomCore *core = new PscomAdapter();

    PscomCommandLineParser parser(app);
    parser.setApplicationDescription("Photo system command-line tool");
    parser._showVersionCallback = std::bind(
        &PscomEngine::showVersion, PscomEngine(app, *core));

    parser.addOptions({
        {
            QStringList{"h", "help", "?"},
            "Displays help on command-line options."
        },
        {
            QStringList{"V", "version"},
            "Displays version information."
        }
    });
    QCommandLineOption optionVerbose(
        QStringList{"v", "verbose"},
        "Verbose mode. Specify up to 2 times to increase the verbosity level"
        "of output messages. Opposite of quiet mode."
    );
    QCommandLineOption optionQuiet(
        QStringList{"q", "quiet"},
        "Quiet mode. Specify up to 2 times to decrease the verbosity level "
        "of output messages. Opposite of verbose mode."
    );
    QCommandLineOption optionOnConflict(
        QStringList{"on-conflict"},
        "Conflict resolution strategy to be applied when a destructive "
        "operation is run. Can be one of the following:"
        "\n- overwrite: Overwrite the original file irrecoverably."
        "\n- skip: Just forget this incident and continue with the next "
            "file."
        "\n- backup: Create a backup of the original file (by appending a "
            "squiggle to its file name) and then overwrite it.",
        "strategy"
    );
    QCommandLineOption optionForce(
        QStringList{"f", "force"},
        QString("Enforce possibly destructive operations regardless of the "
                "consequences. Equivalent to %1=%2."
        ).arg(optionOnConflict.names().last()).arg("overwrite")
    );
    QCommandLineOption optionSupportedFormats(
        QStringList{"supported-formats"},
        "Display all supported image formats."
    );
    QCommandLineOption optionDirectory(
        QStringList{"d", "C", "directory"},
        QString(
            "The directory to look up image files. Pass a single dash (%1) "
            "to enter a list of image files interactively."
        ).arg("-"),
        "path"
    );
    QCommandLineOption optionRecursive(
        QStringList{"R", "recursive"},
        "Include subdirectories."
    );
    QCommandLineOption optionRegex(
        QStringList{"r", "regex"},
        QString(
            "A regular expression to filter image files. Does not need to "
            "match the entire file name; use text anchors (%1) for full "
            "matches."
        ).arg("^ $"),
        "pattern"
    );
    QCommandLineOption optionDateMin(
        QStringList{"min", "min-date"},
        "Reject images older than the given date and time.",
        "date"
    );
    QCommandLineOption optionDateMax(
        QStringList{"max", "max-date"},
        "Reject images newer than the given date and time. NOTE: If you only "
        "specify the date, it will be treated as midnight time.",
        "date"
    );
    QCommandLineOption optionDryRun(
        QStringList{"dry-run"},
        "Only simulate all modifications to the filesystem instead of "
        "actually applying them. Can be helpful to understand the "
        "consequences of your complicated invocation without hazarding your "
        "entire photo library."
    );
    QCommandLineOption optionWidth(
        QStringList{"width"},
        "The width the images should be fit into.",
        "number"
    );
    QCommandLineOption optionHeight(
        QStringList{"height"},
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
    parser.addOptions({optionVerbose, optionQuiet});
    parser.addOptions({optionOnConflict, optionForce});
    assert(parser.addOption(optionSupportedFormats));
    assert(parser.addOption(optionDirectory));
    assert(parser.addOption(optionRecursive));
    assert(parser.addOption(optionRegex));
    assert(parser.addOption(optionDateMin));
    assert(parser.addOption(optionDateMax));
    assert(parser.addOption(optionDryRun));
    assert(parser.addOption(optionWidth));
    assert(parser.addOption(optionHeight));
    assert(parser.addOption(optionFormat));
    assert(parser.addOption(optionQuality));

    PscomCommand commandPscom(
        QStringList{"pscom"},
        QStringList{"symbol", "arguments"},
        "Execute a symbol from the pscom library manually. No safety checks! "
        "Intended for debugging only.",
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
        "To escape date specifiers in the schema, enclose literal parts "
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
            bool ok = true;
            const auto width = parser.isSet(optionWidth)
                ? parser.value(optionWidth).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = optionWidth.names().last().toUtf8();
                qFatal("%s: Invalid number was specified", utf8.constData());
            }
            const auto height = parser.isSet(optionHeight)
                ? parser.value(optionHeight).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = optionHeight.names().last().toUtf8();
                qFatal("%s: Invalid number was specified", utf8.constData());
            }
            engine.resizeFiles(width, height);
        });
    PscomCommand commandConvert(
        QStringList{"convert"},
        QStringList{},
        "Convert image files into a different file format and/or quality.",
        [&parser, &optionFormat, &optionQuality](PscomEngine &engine){
            const auto format = parser.isSet(optionFormat)
                ? parser.value(optionFormat)
                : nullptr;
            bool ok = true;
            const auto quality = parser.isSet(optionQuality)
                ? parser.value(optionQuality).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = optionQuality.names().last().toUtf8();
                qFatal("%s: Invalid number was specified", utf8.constData());
            }
            engine.convertFiles(format, quality);
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

    if (parser.isSet(optionOnConflict) && parser.isSet(optionForce)) {
        const auto
            utf8_1 = optionOnConflict.names().last().toUtf8(),
            utf8_2 = optionForce.names().last().toUtf8();
        qFatal(
            "Cannot combine options %s and %s",
            utf8_1.constData(), utf8_2.constData());
    }

    if (parser.isSet(optionVerbose) || parser.isSet(optionQuiet)) {
        auto verbosityLevel = static_cast<int>(VerbosityLevel::Info);
        verbosityLevel += parser.countSet(optionVerbose);
        verbosityLevel -= parser.countSet(optionQuiet);
        setVerbosityLevel(static_cast<VerbosityLevel>(verbosityLevel));
    }

    if (parser.isSet(optionDryRun)) {
        std::function<QTextStream(void)> x = std::bind(&PscomApp::cout, &app);
        core = new PscomSimulator(*core, x);
    }
    PscomEngine engine(app, *core);
    if (parser.isSet(optionForce)) {
        engine.fileExistsReaction = FileExistsReaction::Overwrite;
    } elif (parser.isSet(optionOnConflict)) {
        const auto conflictStrategy = parser.value(optionOnConflict);
        if (conflictStrategy == "overwrite") {
            engine.fileExistsReaction = FileExistsReaction::Overwrite;
        } elif (conflictStrategy == "skip") {
            engine.fileExistsReaction = FileExistsReaction::Skip;
        } elif (conflictStrategy == "backup") {
            engine.fileExistsReaction = FileExistsReaction::Backup;
        } else {
            const auto
                utf8_1 = optionOnConflict.names().last().toUtf8(),
                utf8_2 = conflictStrategy.toUtf8();
            qFatal(
                "Unknown %s strategy: %s",
                utf8_1.constData(), utf8_2.constData());
        }
    }

    if (parser.isSet(optionSupportedFormats)) {
        engine.showSupportedFormats();
        return EXIT_SUCCESS;
    }

    if (!parser.hasCommand()) {
        parser.showHelp(EXIT_MISUSE);
    }

    if (parser.positionalArguments().first() == "pscom") {
        engine.pscom(parser.positionalArguments(), 1);
        return EXIT_SUCCESS;
    }

    parser.parseCommand();

    if (parser.commandRequiresEngine()) {
        const auto directory = parser.isSet(optionDirectory)
            ? parser.value(optionDirectory)
            : ".";
        const auto recursive = parser.isSet(optionRecursive);
        const auto dateMin = parser.isSet(optionDateMin)
            ? std::make_optional(QDateTime::fromString(
                parser.value(optionDateMin), Qt::ISODateWithMs))
            : std::nullopt;
        const auto dateMax = parser.isSet(optionDateMax)
            ? std::make_optional(QDateTime::fromString(
                parser.value(optionDateMax), Qt::ISODateWithMs))
            : std::nullopt;
        const auto regex = parser.isSet(optionRegex)
            ? QRegExp(parser.value(optionRegex))
            : QRegExp();

        engine.findFiles(directory, recursive, dateMin, dateMax, regex);
    }

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

QTextStream PscomApp::cin() const {
    return QTextStream(stdin);
}

bool PscomApp::isCinInteractive() const {
    #ifdef Q_OS_WIN32
    return _isatty(_fileno(stdin));
    #else
    return isatty(fileno(stdin));
    #endif
}

bool PscomApp::isCerrInteractive() const {
    #ifdef Q_OS_WIN32
    return _isatty(_fileno(stderr));
    #else
    return isatty(fileno(stderr));
    #endif
}

bool PscomApp::isCoutInteractive() const {
    #ifdef Q_OS_WIN32
    return _isatty(_fileno(stdout));
    #else
    return isatty(fileno(stdout));
    #endif
}

int PscomApp::interactiveRequest(
    const QString & message,
    const QList<QPair<QChar, QString>> & answers
) const {
    const auto instructionMessage = [answers](){
        QString instructionMessage;
        for (const auto answer : answers) {
            instructionMessage += QString(
                "[%1] %2  "
            ).arg(answer.first).arg(answer.second);
        }
        return instructionMessage;
    }();
    auto
        streamOut = cerr(),
        streamIn = cin();
    streamOut << message << Qt::endl;
    while (true) {
        streamOut << instructionMessage << Qt::endl;
        const auto answer = streamIn.readLine();

        if (answer.length() > 1) { continue; }
        const auto key = answer[0];
        int index = 0;
        for (const auto answer : answers) {
            index++;
            if (answer.first == key) {
                return index;
            }
        }
    }
}
