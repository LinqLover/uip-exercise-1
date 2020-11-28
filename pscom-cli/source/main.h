#pragma once

#include <QCoreApplication>
#include <QString>
#include <QTextStream>


class PscomCli : public QCoreApplication
{
public:
    PscomCli(int &argc, char *argv[]);
    ~PscomCli();

    QTextStream cout() const;
    QTextStream cerr() const;
};
