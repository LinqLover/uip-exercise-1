#include "command.h"

#include <QDebug>

#include "engine.h"
#include "main.h"


static QString wrapText(const QString &names, int optionNameMaxWidth, const QString &description);

PscomCommandLineParser::PscomCommandLineParser(PscomApp & app) :
    QCommandLineParser::QCommandLineParser(),
    _app(&app),
    _commands(QList<PscomCommand>())
{
    _showVersionCallback = [this](){
        this->QCommandLineParser::showVersion();
    };

    addPositionalArgument(
        "command",
        QObject::tr("The operation to perform."));
}

void PscomCommandLineParser::addCommand(PscomCommand & command) {
    _commands.append(command);
}

PscomCommand PscomCommandLineParser::addVersionCommand(void) {
    auto command = PscomCommand(
        QStringList{"version"},
        QStringList{},
        QObject::tr("Displays help on command-line options."),
        [this](){showVersion();});
    command.isHiddenFromHelp = true;
    addCommand(command);
    return command;
}

PscomCommand PscomCommandLineParser::addHelpCommand(void) {
    auto command = PscomCommand(
        QStringList{"help"},
        QStringList{},
        QObject::tr("Displays version information."),
        [this](){showHelp(EXIT_SUCCESS);});
    command.isHiddenFromHelp = true;
    addCommand(command);
    return command;
}

int PscomCommandLineParser::countSet(
    const QCommandLineOption & option
) const {
    const auto targetNames = option.names();
    int count = 0;
    for (const auto parsedName : optionNames()) {
        if (targetNames.contains(parsedName, Qt::CaseInsensitive)) {
            count++;
        }
    }
    return count;
}

void PscomCommandLineParser::process(const QStringList & arguments)
{
    if (!parse(arguments)) {
        const auto utf8 = errorText().toUtf8();
        qFatal("%s", utf8.constData());
    }

    if (isSet(QStringLiteral("version"))) {
        showVersion();
    }

    if (isSet(QStringLiteral("help"))) {
        showHelp(EXIT_SUCCESS);
    }

    const auto posArgs = positionalArguments();
    if (!posArgs.isEmpty()) {
        const auto cmdName = posArgs.first();
        PscomCommand *command = nullptr;
        for (
            auto it = _commands.begin(), end = _commands.end();
            it != end;
            ++it
        ) {
            if (it->names.contains(cmdName)) {
                command = &(*it);
                break;
            }
        }
        if (!command) {
            const auto utf8 = QObject::tr("Unknown command: %1")
                .arg(cmdName)
                .toUtf8();
            qFatal("%s", utf8.constData());
        }
        _command = command;
    }
}

void PscomCommandLineParser::process()
{
    process(QCoreApplication::arguments());
}

bool PscomCommandLineParser::commandRequiresEngine() const {
    return _command->requiresEngine();
}

bool PscomCommandLineParser::hasCommand() const {
    return _command;
}

void PscomCommandLineParser::runCommand(PscomEngine & engine) const {
    _command->execute(engine, _commandArguments);
}

void PscomCommandLineParser::parseCommand() {
    QStringList posArgs = positionalArguments();

    {
        const auto numPosArgs = posArgs.length() - 1;
        const auto cmdParams = _command->parameters;

        if (numPosArgs > cmdParams.length()) {
            const auto utf8 = QObject::tr("Too many arguments").toUtf8();
            qFatal("%s", utf8.constData());
        }

        int i = 0;
        QStringList missingParameters;
        auto
            defaultIt = _command->defaultValues.constBegin(),
            defaultEnd = _command->defaultValues.constEnd();
        for (auto param : cmdParams) {
            if (i++ < numPosArgs) {
                _commandArguments.append(posArgs[i]);
            } elif (defaultIt != defaultEnd && *(defaultIt++) != nullptr) {
                _commandArguments.append(*(defaultIt - 1));
            } else {
                missingParameters.append(param);
            }
        }
        if (!missingParameters.isEmpty()) {
            const auto utf8 = QObject::tr(
                (missingParameters.size() == 1
                    ? "Missing argument: %s"
                    : "Missing arguments: %s")
                ).arg(missingParameters.join(", "))
                .toUtf8();
            qFatal("%s", utf8.constData());
        }
    }
}

QString PscomCommandLineParser::helpText() const {
    auto text = QCommandLineParser::helpText();

    if (_commands.isEmpty()) {
        return text;
    }

    const auto nl = QLatin1Char('\n');
    text += nl;
    text += QObject::tr("Commands:") + nl;
    const int maxWidth = 28;
    for (auto command : _commands) {
        if (command.isHiddenFromHelp)
            continue;
        QString syntaxText;
        syntaxText = command.names.join(", ");
        auto
            defaultIt = command.defaultValues.constBegin(),
            defaultEnd = command.defaultValues.constEnd();
        for (auto param : command.parameters) {
            const auto placeholder = "<" + param + ">";
            syntaxText += " " + (
                (defaultIt != defaultEnd && *(defaultIt++) != nullptr)
                    ? "[" + placeholder + "]"
                    : placeholder);
        }
        text += wrapText(syntaxText, maxWidth - 1, command.description);
    }

    return text;
}

void PscomCommandLineParser::showHelp(int exitCode) const {
    _app->cout() << helpText();
    //qt_call_post_routines(); // Not available
    ::exit(exitCode);
}

void PscomCommandLineParser::showVersion() const {
    _showVersionCallback();
    //qt_call_post_routines(); // Not available
    ::exit(EXIT_SUCCESS);
}

PscomCommand::PscomCommand(
    const QStringList & names,
    const QStringList & parameters,
    const QString & description,
    const QStringList & defaultValues
) :
    names(QStringList(names)),
    parameters(QStringList(parameters)),
    description(description),
    defaultValues(QStringList(defaultValues))
{
    assert(defaultValues.length() <= parameters.length());
}

PscomCommand::PscomCommand(
    const QStringList & names,
    const QStringList & parameters,
    const QString & description,
    const std::function<void(void)> function,
    const QStringList & defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 0;
    _function_p0 = function;
}

PscomCommand::PscomCommand(
    const QStringList & names,
    const QStringList & parameters,
    const QString & description,
    const std::function<void(PscomEngine &)> function,
    const QStringList & defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 1;
    _function_p1 = function;
}

PscomCommand::PscomCommand(
    const QStringList & names,
    const QStringList & parameters,
    const QString & description,
    const std::function<void(PscomEngine &, QString)> function,
    const QStringList & defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 2;
    _function_p2 = function;
}

void PscomCommand::execute(
    PscomEngine & engine,
    const QStringList & arguments
) const {
    assert(!_function_numArgs
        ? !arguments.length()
        : arguments.length() + 1 == _function_numArgs);
    switch (_function_numArgs) { // Early binding, sigh ...
        case 0:
            _function_p0();
            return;
        case 1:
            _function_p1(engine);
            return;
        case 2:
            _function_p2(engine, arguments[0]);
            return;
        default:
            assert(false); // Unsupported numArgs!
            std::terminate();
    }
}

bool PscomCommand::requiresEngine(void) const {
    return _function_numArgs;
}

static QString wrapText(
    const QString &names, int optionNameMaxWidth, const QString &description
) {
    const QLatin1Char nl('\n');
    const QLatin1String indentation("  ");

    // In case the list of option names is very long, wrap it as well
    int nameIndex = 0;
    auto nextNameSection = [&]() {
        QString section = names.mid(nameIndex, optionNameMaxWidth);
        nameIndex += section.size();
        return section;
    };

    QString text;
    int lineStart = 0;
    int lastBreakable = -1;
    const int max = 79 - (indentation.size() + optionNameMaxWidth + 1);
    int x = 0;
    const int len = description.length();

    for (int i = 0; i < len; ++i) {
        ++x;
        const QChar c = description.at(i);
        if (c.isSpace())
            lastBreakable = i;

        int breakAt = -1;
        int nextLineStart = -1;
        if (x > max && lastBreakable != -1) {
            // time to break and we know where
            breakAt = lastBreakable;
            nextLineStart = lastBreakable + 1;
        } else if ((x > max - 1 && lastBreakable == -1) || i == len - 1) {
            // time to break but found nowhere [-> break here], or end of last
            // line
            breakAt = i + 1;
            nextLineStart = breakAt;
        } else if (c == nl) {
            // forced break
            breakAt = i;
            nextLineStart = i + 1;
        }

        if (breakAt != -1) {
            const int numChars = breakAt - lineStart;
            text += indentation
                + nextNameSection().leftJustified(optionNameMaxWidth)
                + QLatin1Char(' ');
            text += description.midRef(lineStart, numChars) + nl;
            x = 0;
            lastBreakable = -1;
            lineStart = nextLineStart;
            if (lineStart < len && description.at(lineStart).isSpace())
                ++lineStart; // don't start a line with a space
            i = lineStart;
        }
    }

    while (nameIndex < names.size()) {
        text += indentation + nextNameSection() + nl;
    }

    return text;
}
