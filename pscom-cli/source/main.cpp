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
#include <QLibraryInfo>
#include <QList>
#include <QObject>
#include <QPair>
#include <QRegExp>
#include <QTextStream>
#include <QTranslator>

#include "command.h"
#include "engine.h"
#include "verbosity.h"


#define EXIT_MISUSE 2


void setupTranslators(PscomApp & app);

int main(int argc, char *argv[])
{
    qInstallMessageHandler(VerbosityHandler);

    // Setting the application name is not required, since, if not set, it defaults to the executable name.
    // QCoreApplication::setApplicationName("pscom-cli");
    QCoreApplication::setApplicationVersion("3.0.0");
    auto app = PscomApp(argc, argv);
    setupTranslators(app);

    IPscomCore *core = new PscomAdapter();

    PscomCommandLineParser parser(app);
    parser.setApplicationDescription(QObject::tr("Photo system command-line tool"));
    parser._showVersionCallback = std::bind(
        &PscomEngine::showVersion, PscomEngine(app, *core));

    // These will be handled by the parser itself
    parser.addOptions({
        {
            QStringList{"h", "help", "?"},
            QObject::tr("Displays help on command-line options.")
        },
        {
            QStringList{"V", "version"},
            QObject::tr("Displays version information.")
        }
    });
    QCommandLineOption optionSupportedFormats(
        QStringList{"supported-formats"},
        QObject::tr("Display all supported image formats.")
    );
    QCommandLineOption optionDirectory(
        QStringList{"d", "C", "directory"},
        QString(QObject::tr(
            "The directory to look up image files. Pass a single dash (%1) "
            "to enter a list of image files interactively."
        )).arg("-"),
        "path"
    );
    QCommandLineOption optionRecursive(
        QStringList{"R", "recursive"},
        QObject::tr("Include subdirectories.")
    );
    QCommandLineOption optionRegex(
        QStringList{"r", "regex"},
        QString(QObject::tr(
            "A regular expression to filter image files. Does not need to "
            "match the entire file name; use text anchors (%1) for full "
            "matches."
        )).arg("^ $"),
        "pattern"
    );
    QCommandLineOption optionDateMin(
        QStringList{"min", "min-date"},
        QObject::tr("Reject images older than the given date and time."),
        "date"
    );
    QCommandLineOption optionDateMax(
        QStringList{"max", "max-date"},
        QObject::tr(
            "Reject images newer than the given date and time. NOTE: If you "
            "only specify the date, it will be treated as midnight time."),
        "date"
    );
    QCommandLineOption optionVerbose(
        QStringList{"v", "verbose"},
        QObject::tr(
            "Verbose mode. Specify up to %1 times to increase the verbosity "
            "level of output messages. Opposite of quiet mode."
        ).arg(2)
    );
    QCommandLineOption optionQuiet(
        QStringList{"q", "quiet"},
        QObject::tr(
            "Quiet mode. Specify up to %1 times to decrease the verbosity "
            "level of output messages. Opposite of verbose mode."
        ).arg(2)
    );
    QCommandLineOption optionOnConflict(
        QStringList{"on-conflict"},
        QString(QObject::tr(
            "Conflict resolution strategy to be applied when a destructive "
            "operation is run. Can be one of the following:"
            "\n- overwrite: Overwrite the original file irrecoverably."
            "\n- skip: Just forget this incident and continue with the next "
                "file."
            "\n- backup: Create a backup of the original file (by appending "
                "a squiggle (%1) to its file name) and then overwrite it."
        )).arg("~"),
        "strategy"
    );
    QCommandLineOption optionForce(
        QStringList{"f", "force"},
        QString(QObject::tr(
            "Enforce possibly destructive operations regardless of the "
            "consequences. Equivalent to %1=%2.")
        ).arg(optionOnConflict.names().last()).arg("overwrite")
    );
    QCommandLineOption optionDryRun(
        QStringList{"dry-run"},
        QObject::tr(
            "Only simulate all modifications to the filesystem instead of "
            "actually applying them. Can be helpful to understand the "
            "consequences of your complicated invocation without hazarding "
            "your entire photo library.")
    );
    QCommandLineOption optionWidth(
        QStringList{"width"},
        QObject::tr("The width the images should be fit into."),
        "number"
    );
    QCommandLineOption optionHeight(
        QStringList{"height"},
        QObject::tr("The height the images should be fit into."),
        "number"
    );
    QCommandLineOption optionFormat(
        QStringList{"format"},
        QObject::tr(
            "The file format (e.g. %1 or %2) the images should be converted "
            "into."
        ).arg("jpg", "png"),
        "extension"
    );
    QCommandLineOption optionQuality(
        QStringList{"quality"},
        QObject::tr(
            "The quality for image conversion. Value between %1 (best "
            "compression) and %2 (best quality)."
        ).arg(0).arg(100),
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
        QObject::tr(
            "Execute a symbol from the pscom library manually. No safety "
            "checks! Intended for debugging purposes only."),
        [](PscomEngine &engine){ engine.pscom(QStringList{}); });
    PscomCommand commandList(
        QStringList{"list", "ls"},
        QStringList{},
        QObject::tr("Display image files."),
        &PscomEngine::listFiles);
    PscomCommand commandCopy(
        QStringList{"copy", "cp"},
        QStringList{"destination"},
        QObject::tr("Copy image files into the specified destination folder."),
        &PscomEngine::copyFiles);
    PscomCommand commandMove(
        QStringList{"move", "mv"},
        QStringList{"destination"},
        QObject::tr("Move image files into the specified destination folder."),
        &PscomEngine::moveFiles);
    const auto escapeNote = QString(QObject::tr(
        "To escape date specifiers in the schema, enclose literal parts "
        "into single quotes (%1, or %2 from the bash shell)."
    )).arg("'").arg("\"'\"");
    PscomCommand commandRename(
        QStringList{"rename", "rn"},
        QStringList{"schema"},
        QObject::tr(
            "Rename image files according to the given schema, or according "
            "to the UPA standard, if omitted. "
        ) + escapeNote,
        &PscomEngine::renameFiles,
        QStringList{"yyyyMMdd_HHmmsszzz"});
    PscomCommand commandGroup(
        QStringList{"group", "g"},
        QStringList{"schema"},
        QObject::tr(
            "Group image files into subdirectories according to the given "
            "schema, or according to the UPA standard, if omitted. "
        ) + escapeNote,
        &PscomEngine::groupFiles,
        QStringList{"yyyy/yyyy-MM"});
    PscomCommand commandResize(
        QStringList{"resize"},
        QStringList{},
        QObject::tr("Resize image files into the given dimensions."),
        [&parser, &optionWidth, &optionHeight](PscomEngine &engine){
            bool ok = true;
            const auto width = parser.isSet(optionWidth)
                ? parser.value(optionWidth).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = QObject::tr("%1: Invalid number was specified")
                    .arg(optionWidth.names().last())
                    .toUtf8();
                qFatal("%s", utf8.constData());
            }
            const auto height = parser.isSet(optionHeight)
                ? parser.value(optionHeight).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = QObject::tr("%1: Invalid number was specified")
                    .arg(optionHeight.names().last())
                    .toUtf8();
                qFatal("%s", utf8.constData());
            }
            engine.resizeFiles(width, height);
        });
    PscomCommand commandConvert(
        QStringList{"convert"},
        QStringList{},
        QObject::tr(
            "Convert image files into a different file format and/or "
            "quality."),
        [&parser, &optionFormat, &optionQuality](PscomEngine &engine){
            const auto format = parser.isSet(optionFormat)
                ? parser.value(optionFormat)
                : nullptr;
            bool ok = true;
            const auto quality = parser.isSet(optionQuality)
                ? parser.value(optionQuality).toInt(&ok)
                : -1;
            if (!ok) {
                const auto utf8 = QObject::tr("%1: Invalid number was specified")
                    .arg(optionQuality.names().last())
                    .toUtf8();
                qFatal("%s", utf8.constData());
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
        const auto utf8 = QObject::tr("Cannot combine options %1 and %2")
            .arg(optionOnConflict.names().last())
            .arg(optionForce.names().last())
            .toUtf8();
        qFatal("%s", utf8.constData());
    }

    // Inspired by the ssh tool interface.
    if (parser.isSet(optionVerbose) || parser.isSet(optionQuiet)) {
        auto verbosityLevel = static_cast<int>(VerbosityLevel::Info);
        verbosityLevel += parser.countSet(optionVerbose);
        verbosityLevel -= parser.countSet(optionQuiet);
        setVerbosityLevel(static_cast<VerbosityLevel>(verbosityLevel));
    }
    setInteractive(app.isCerrInteractive());

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
            const auto utf8 = QObject::tr("Unknown {1} strategy: {2}")
                .arg(optionOnConflict.names().last())
                .arg(conflictStrategy)
                .toUtf8();
            qFatal("%s", utf8.constData());
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

void setupTranslators(PscomApp & app) {
    QTranslator qtTranslator;
    if (!qtTranslator.load(
        QLocale(), "qt", "_",
        QLibraryInfo::location(QLibraryInfo::TranslationsPath), {})
    ) {
        qWarning("Could not load system translation files");
    }
    app.installTranslator(&qtTranslator);
    QTranslator appTranslator;
    if (!appTranslator.load(
        QLocale(), "pscom-cli", "_",
        QFileInfo(app.applicationDirPath()).absoluteDir()
            .absoluteFilePath("resources/translations"))
    ) {
        qWarning("Could not load application translation files");
    }
    app.installTranslator(&appTranslator);
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
    if (isCerrInteractive()) {
        streamOut << "\033[1;3m"; // bold + italic
    }
    streamOut << message << Qt::endl;
    while (true) {
        streamOut << instructionMessage << Qt::endl;
        const auto answer = streamIn.readLine();

        if (answer.length() != 1) { continue; }
        const auto key = answer[0];
        int index = 0;
        for (const auto answer : answers) {
            index++;
            if (answer.first == key) {
                if (isCerrInteractive()) {
                    streamOut << "\033[0m"; // reset font
                }
                return index;
            }
        }
    }
}
