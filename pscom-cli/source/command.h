#pragma once

#include <QCommandLineParser>

#include "engine.h" // TODO: Maybe decouple using template?
#include "main.h"


class PscomCommand;


class PscomCommandLineParser : public QCommandLineParser {
private:
    PscomApp *_app;
    QList<PscomCommand> _commands;
    PscomCommand *_command = nullptr;
    QStringList _commandArguments;
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
};


class PscomCommand {
private:
    PscomCommand(
        const QStringList & names,
        const QStringList & parameters,
        const QString & description,
        const QStringList & defaultValues = QStringList());

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
