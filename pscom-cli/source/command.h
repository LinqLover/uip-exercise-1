#pragma once

#include <QCommandLineParser>

#include "engine.h" // TODO: Maybe decouple using template?
#include "main.h"


class PscomCommand;


class PscomCommandLineParser : public QCommandLineParser {
private:
    PscomCli *_app;
    QList<PscomCommand> _commands;
    PscomCommand *_command = nullptr;
public:
    PscomCommandLineParser(PscomCli &app);
    std::function<void(void)> _showVersion;

    void addCommand(PscomCommand &command);
    PscomCommand addVersionCommand(void);
    PscomCommand addHelpCommand(void);
    void process(const QStringList &arguments);
    void process();
    bool hasCommand() const;
    void runCommand(PscomEngine &engine) const;

    QString helpText() const;
    void showHelp(int exitCode) const;
    void showVersion() const;
};


class PscomCommand {
private:
    PscomCommand(
        const QStringList &names,
        const QStringList &parameters,
        const QString &description,
        const QStringList &defaultValues = QStringList());

    int _function_numArgs;
    std::function<void(void)> _function_p0;
    std::function<void(PscomEngine &)> _function_p1;
    std::function<void(PscomEngine &, QString)> _function_p2;
public:
    PscomCommand(
        const QStringList &names,
        const QStringList &parameters,
        const QString &description,
        const std::function<void(void)> function,
        const QStringList &defaultValues = QStringList());
    PscomCommand(
        const QStringList &names,
        const QStringList &parameters,
        const QString &description,
        const std::function<void(PscomEngine &)> function,
        const QStringList &defaultValues = QStringList());
    PscomCommand(
        const QStringList &names,
        const QStringList &parameters,
        const QString &description,
        const std::function<void(PscomEngine &, QString)> function,
        const QStringList &defaultValues = QStringList());

    QStringList names;
    QStringList parameters;
    QString description;
    QStringList defaultValues;
    bool isHiddenFromHelp = false;

    void execute(PscomEngine &engine, QStringList &arguments) const;
};
