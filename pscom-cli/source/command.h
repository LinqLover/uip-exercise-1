#pragma once

#include <QCommandLineParser>

#include "engine.h"
#include "main.h"


class PscomCommand;


// Subclass of QCommandLineParser that supports parsing and executing
// PscomCommands passed as particular positional arguments.
// Disclaimer: Because the Qt implementation hides a lot of its logic in a
// private helper class, this class contains a lot of code copied from
// there ...
class PscomCommandLineParser : public QCommandLineParser {
public:
    PscomCommandLineParser(PscomApp & app);
    std::function<void(void)> _showVersionCallback;

    void addCommand(PscomCommand & command);
    PscomCommand addVersionCommand(void);
    PscomCommand addHelpCommand(void);
    int countSet(const QCommandLineOption & option) const;
    bool commandRequiresEngine(void) const;
    bool hasCommand(void) const;
    void process(const QStringList & arguments);
    void process(void);
    void parseCommand(void);
    void runCommand(PscomEngine & engine) const;

    QString helpText(void) const;
    void showHelp(int exitCode) const;
    void showVersion(void) const;
private:
    PscomApp *_app;
    QList<PscomCommand> _commands;
    PscomCommand *_command = nullptr;
    QStringList _commandArguments;
};


// Represents a command line argument that is passed as a positional parameter
// to a CLI application. Kind of similar to QCommandLineOption, but commands
// are not optional nor can have arguments themselves.
class PscomCommand {
private:
    PscomCommand(
        const QStringList & names,
        const QStringList & parameters,
        const QString & description,
        const QStringList & defaultValues = QStringList());

    // Sigh ... Because C++ does not support late binding, reflection or any
    // other cool stuff, we need to maintain the different signatured function
    // pointers separately. It might be possible to use a union type to save a
    // few bits, but this does not seems very important.
    // (In Smalltalk, I would simply have sent #cull:cull: to the function 😛)
    int _function_numArgs;
    std::function<void(void)> _function_p0;
    std::function<void(PscomEngine &)> _function_p1;
    std::function<void(PscomEngine &, QString)> _function_p2;
public:
    PscomCommand(
        const QStringList & names,
        const QStringList & parameters,
        const QString & description,
        const std::function<void(void)> function,
        const QStringList & defaultValues = QStringList());
    PscomCommand(
        const QStringList & names,
        const QStringList & parameters,
        const QString & description,
        const std::function<void(PscomEngine &)> function,
        const QStringList & defaultValues = QStringList());
    PscomCommand(
        const QStringList & names,
        const QStringList & parameters,
        const QString & description,
        const std::function<void(PscomEngine &, QString)> function,
        const QStringList & defaultValues = QStringList());

    QStringList names;
    QStringList parameters;
    QString description;
    QStringList defaultValues;
    bool isHiddenFromHelp = false;

    bool requiresEngine() const;
    void execute(
        PscomEngine & engine,
        const QStringList & arguments) const;
};
