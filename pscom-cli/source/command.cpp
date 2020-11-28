#include "command.h"

#include <QDebug>
#include <QTextStream>

#include "engine.h"
#include "main.h"


PscomCommandLineParser::PscomCommandLineParser(PscomCli &app) :
    QCommandLineParser::QCommandLineParser(),
    _app(&app),
    _commands(QList<PscomCommand>())
{
    _showVersion = [this](){ this->QCommandLineParser::showVersion(); };

    addPositionalArgument("command", "The operation to perform.");
    addOption(QCommandLineOption(
        QStringList{"h", "help", "?"},
        "Displays help on command-line options."
    ));
    addOption(QCommandLineOption(
        QStringList{"v", "version"},
        "Displays version information."
    ));
}

void PscomCommandLineParser::addCommand(PscomCommand &command) {
    _commands.append(command);
}

PscomCommand PscomCommandLineParser::addVersionCommand(void) {
    auto command = PscomCommand(
        QStringList{"version"},
        QStringList{},
        "Displays help on commandline options.",
        [this](){showVersion();});
    command.isHiddenFromHelp = true;
    addCommand(command);
    return command;
}

PscomCommand PscomCommandLineParser::addHelpCommand(void) {
    auto command = PscomCommand(
        QStringList{"help"},
        QStringList{},
        "Displays version information.",
        [this](){showHelp(EXIT_SUCCESS);});
    command.isHiddenFromHelp = true;
    addCommand(command);
    return command;
}

void PscomCommandLineParser::process(const QStringList &arguments)
{
    if (!parse(arguments)) {
        qFatal("%s", errorText().toUtf8().constData());
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
            qFatal("Unknown command: %s", cmdName.toUtf8().constData());
        }
        _command = command;
    }
}

void PscomCommandLineParser::process()
{
    process(QCoreApplication::arguments());
}

bool PscomCommandLineParser::hasCommand() const {
    return _command;
}

void PscomCommandLineParser::runCommand(PscomEngine &engine) const {
    QStringList posArgs = positionalArguments(), arguments;

    {
        const auto numPosArgs = posArgs.length() - 1;
        const auto cmdParams = _command->parameters;

        if (numPosArgs > cmdParams.length()) {
            qFatal("Too many arguments");
        }

        int i = 0;
        QStringList missingParameters;
        auto
            defaultIt = _command->defaultValues.constBegin(),
            defaultEnd = _command->defaultValues.constEnd();
        for (auto param : cmdParams) {
            if (i++ < numPosArgs) {
                arguments.append(posArgs[i]);
            } else if (defaultIt != defaultEnd && *(defaultIt++) != nullptr) {
                arguments.append(*(defaultIt - 1));
            } else {
                missingParameters.append(param);
            }
        }
        if (!missingParameters.isEmpty()) {
            qFatal((missingParameters.size() == 1
                    ? "Missing argument: %s"
                    : "Missing arguments: %s"),
                missingParameters.join(", ").toUtf8().constData());
        }
    }

    _command->execute(engine, arguments);
}

QString PscomCommandLineParser::helpText() const {
    auto text = QCommandLineParser::helpText();

    if (_commands.isEmpty()) {
        return text;
    }

    const auto nl = QLatin1Char('\n');
    text += nl;
    text += QCommandLineParser::tr("Commands:") + nl;
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
        text += "  " + syntaxText.leftJustified(maxWidth)
              + command.description + nl;
    }

    return text;
}

void PscomCommandLineParser::showHelp(int exitCode) const {
    _app->cout() << helpText();
    //qt_call_post_routines(); // Not available
    ::exit(exitCode);
}

void PscomCommandLineParser::showVersion() const {
    _showVersion();
    //qt_call_post_routines(); // Not available
    ::exit(EXIT_SUCCESS);
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const QStringList &defaultValues
) :
    names(QStringList(names)),
    parameters(QStringList(parameters)),
    description(description),
    defaultValues(QStringList(defaultValues))
{
    assert(defaultValues.length() <= parameters.length());
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(void)> function,
    const QStringList &defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 0;
    _function_p0 = function;
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(PscomEngine &)> function,
    const QStringList &defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 1;
    _function_p1 = function;
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(PscomEngine &, QString)> function,
    const QStringList &defaultValues
) : PscomCommand(names, parameters, description, defaultValues)
{
    _function_numArgs = 2;
    _function_p2 = function;
}

void PscomCommand::execute(
    PscomEngine &engine,
    QStringList &arguments
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
