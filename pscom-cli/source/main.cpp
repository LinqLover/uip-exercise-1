
#include <pscom.h>

#include <optional>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QRegExp>

#include <QTextStream>

#include "verbosity.h"


/* PLEASE NOTE
 * - be carful using the pscom library, it may irreversible delete actual data if used without care.
 * - try to rely on pscom library exclusively in order to fulfill the user stories / tasks.
 * - use qt only to drive the library and control command language aspects and CLI related features, e.g.,
 *   progress, logging, verbosity, undo, help, etc.
 * - prefer qDebug, qInfo, qWarning, qCritical, and qFatal for verbosity and console output.
 */

// CREDITS: https://stackoverflow.com/a/25506213/13994294 (edited)
QStringList intersection(const QList<QStringList> &vecs) {
    auto last_intersection = vecs[0];
    QStringList curr_intersection;
    for (auto i = 1; i < vecs.size(); ++i) {
        std::set_intersection(last_intersection.begin(), last_intersection.end(),
            vecs[i].begin(), vecs[i].end(),
            std::back_inserter(curr_intersection));
        std::swap(last_intersection, curr_intersection);
        curr_intersection.clear();
    }
    return last_intersection;
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(VerbosityHandler);

    // Setting the application name is not required, since, if not set, it defaults to the executable name.
    // QCoreApplication::setApplicationName("pscom-cli");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication app(argc, argv);

    QTextStream cout(stdout);
    QTextStream cerr(stderr);

    QCommandLineParser parser;
    parser.setApplicationDescription("Photo system command-line tool");
    auto optionHelp = parser.addHelpOption();
    auto optionVersion = parser.addVersionOption();
    parser.addPositionalArgument("command", "run command");
    parser.parse(app.arguments());

    if (parser.isSet(optionHelp)) {
        parser.showHelp();
    }
    if (parser.isSet(optionVersion)) {
        parser.showVersion();
    }

    auto args = parser.positionalArguments();
    auto iArg = -1;
    if (args.length() <= ++iArg) {
        parser.showHelp(1);
    }
    const auto command = args[iArg];

    if (command == "help") {
        parser.showHelp(0);
    } else if (command == "version") {
        parser.showVersion();
    } else if (command == "version-all") {
        cout << pscom::vi() << Qt::endl;
        return 0;
    } else if (command == "supported-formats") {
        auto supportedFormats = pscom::sf();
        for (auto format : supportedFormats) {
            cout << format << Qt::endl;
        }
        return 0;
    }
    
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
    if (command == "list") {
        parser.addPositionalArgument(
            "directory",
            "The directory where the files should be searched."
        );
        assert(parser.addOption(optionRecursive));
        assert(parser.addOption(optionRegex));
        assert(parser.addOption(optionMinDate));
        assert(parser.addOption(optionMaxDate));
        if (!(parser.parse(app.arguments()) && parser.unknownOptionNames().isEmpty())) {
            cerr << QCoreApplication::applicationName() + QLatin1String(": ") + parser.errorText() + QLatin1Char('\n');
            return 1;
        }
        args = parser.positionalArguments();
        const auto directory = args.length() > iArg + 1
            ? args[++iArg]
            : ".";
        const auto recursive = parser.isSet(optionRecursive);
        const auto dateMin = parser.isSet(optionMinDate)
            ? std::make_optional(QDateTime::fromString(parser.value(optionMinDate), Qt::ISODateWithMs))
            : std::nullopt;
        const auto dateMax = parser.isSet(optionMaxDate)
            ? std::make_optional(QDateTime::fromString(parser.value(optionMaxDate), Qt::ISODateWithMs))
            : std::nullopt;
        const auto regex = parser.isSet(optionRegex)
            ? std::make_optional(QRegExp(parser.value(optionRegex)))
            : std::nullopt;

        QStringList files;
        if (directory == "-") {
            if (bool(dateMin) || bool(dateMax) || bool(regex)) {
                cerr << QCoreApplication::applicationName() + QLatin1String(": ") + "Filter options and stdin cannot be combined" + QLatin1Char('\n');
                return(1);
            }
            QTextStream cin(stdin);
            while (!cin.atEnd()) {
                files.append(cin.readLine());
            }
        } else {
            QList<QStringList> fileLists;
            if (bool(dateMin) || bool(dateMax)) {
                fileLists.append(pscom::dt(directory, dateMin.value_or(QDateTime()), dateMax.value_or(QDateTime::fromSecsSinceEpoch(185542587187199999)), recursive));
            }
            if (regex) {
                fileLists.append(pscom::re(directory, regex.value(), recursive));
            }
            files = fileLists.isEmpty()
                ? pscom::re(directory, QRegExp(), recursive)
                : intersection(fileLists);
        }

        for (auto file : files) {
            cout << file << Qt::endl;
        }
        return 0;
    }

    cerr << QCoreApplication::applicationName() + QLatin1String(": ") + "Unknown command: " + command + QLatin1Char('\n');

    return EXIT_SUCCESS;
}
