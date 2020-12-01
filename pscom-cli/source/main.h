#pragma once

#include <QCoreApplication>
#include <QString>
#include <QTextStream>


class PscomApp : public QCoreApplication
{
public:
    PscomApp(int &argc, char *argv[]);
    ~PscomApp();

    QTextStream cout() const;
    QTextStream cerr() const;
    QTextStream cin() const;
};
