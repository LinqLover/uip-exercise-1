#pragma once

#include <QCoreApplication>
#include <QString>
#include <QTextStream>


// Our very special QCoreApplication specialization. Motivation includes:
// - Shorter to type
// - A bit flair of OOP without wasting the architecture with too many roles
// - Store I/O related logic near to the QApplication stuff.
// Singleton (or at least meant to be one).
class PscomApp : public QCoreApplication
{
public:
    PscomApp(int &argc, char *argv[]);
    ~PscomApp();

    QTextStream cout() const;
    QTextStream cerr() const;
    QTextStream cin() const;
    bool isCerrInteractive() const;
    bool isCinInteractive() const;
    bool isCoutInteractive() const;
    int interactiveRequest(
        const QString & message,
        const QList<QPair<QChar, QString>> & answers) const;
};
