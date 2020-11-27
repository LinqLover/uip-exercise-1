#include "command.h"

#include <QTextStream>

#include "engine.h"
#include "main.h"


PscomCommandLineParser::PscomCommandLineParser(PscomCli &app) :
    QCommandLineParser::QCommandLineParser(),
    _app(&app),
    _commands(QList<PscomCommand>())
{
    _showVersion = [this](){ this->QCommandLineParser::showVersion(); };

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
        _app->showError(errorText());
        //qt_call_post_routines(); // Not available
        ::exit(EXIT_FAILURE);
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
            _app->showError("Unknown command: " + cmdName);
            //qt_call_post_routines(); // Not available
            ::exit(EXIT_FAILURE);
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
    auto posArgs = positionalArguments();

    {
        const auto numPosArgs = posArgs.length() - 1;
        const auto cmdParams = _command->parameters;

        if (numPosArgs > cmdParams.length()) {
            _app->showError("Too many arguments");
        }

        int i = 0;
        QStringList missingParameters;
        for (auto param : cmdParams) {
            if (i++ >= numPosArgs) {
                missingParameters.append(param);
            }
        }
        if (!missingParameters.isEmpty()) {
            _app->showError((missingParameters.size() == 1
                ? "Missing argument: "
                : "Missing arguments: ")
                    + missingParameters.join(", "));
        }
    }
    
    auto it = posArgs.constBegin();
    it++; // skip command argument
    _command->execute(engine, it, posArgs.constEnd());
}

QString PscomCommandLineParser::helpText() const {
    auto text = QCommandLineParser::helpText();

    if (_commands.isEmpty()) {
        return text;
    }

    const auto nl = QLatin1Char('\n');
    text += nl;
    text += QCommandLineParser::tr("Commands:") + nl;
    const int maxWidth = 25;
    for (auto command : _commands) {
        if (command.isHiddenFromHelp)
            continue;
        QString syntaxText;
        syntaxText = command.names.join(", ");
        for (auto param : command.parameters) {
            syntaxText += " <" + param + "> ";
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
    const QString &desciption
) :
    names(QStringList(names)),
    parameters(QStringList(parameters)),
    description(desciption)
{
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(void)> function
) : PscomCommand(names, parameters, description)
{
    _function_numArgs = 0;
    _function_p0 = function;
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(PscomEngine &)> function
) : PscomCommand(names, parameters, description)
{
    _function_numArgs = 1;
    _function_p1 = function;
}

PscomCommand::PscomCommand(
    const QStringList &names,
    const QStringList &parameters,
    const QString &description,
    const std::function<void(PscomEngine &, QString)> function
) : PscomCommand(names, parameters, description)
{
    _function_numArgs = 2;
    _function_p2 = function;
}

void PscomCommand::execute(
    PscomEngine &engine,
    QList<QString>::const_iterator begin,
    QList<QString>::const_iterator end
) const {
    QStringList arguments;
    for(begin; begin != end; begin++) {
        arguments.push_back(*begin);
    }
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