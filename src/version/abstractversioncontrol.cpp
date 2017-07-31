#include "abstractversioncontrol.h"

#include <QDir>
#include <QDebug>

AbstractVersionControl::AbstractVersionControl(QObject *parent) : QObject(parent)
{
}

QString AbstractVersionControl::versionControlName() const
{
    return "none";
}

void AbstractVersionControl::setPath(const QString &path)
{

    _validatedFiles.clear();
    _trackedFiles.clear();
    _modifiedFiles.clear();
    _path = QDir::cleanPath(path);
    analysePath();
}

bool AbstractVersionControl::isDirTracked(const QString &filePath) const
{
    if (!isValid())
        return true;
    QSetIterator<QString> i(_trackedFiles);
    while (i.hasNext())
        if (i.next().startsWith(filePath))
            return true;
    return false;
}

bool AbstractVersionControl::isFileModified(const QString &filePath) const
{
    return _modifiedFiles.contains(filePath);
}

bool AbstractVersionControl::isDirModified(const QString &filePath) const
{
    QSetIterator<QString> i(_modifiedFiles);
    while (i.hasNext())
        if (i.next().startsWith(filePath))
            return true;
    return false;
}

bool AbstractVersionControl::isFileTracked(const QString &filePath) const
{
    if (!isValid())
        return true;
    return _trackedFiles.contains(filePath);
}

bool AbstractVersionControl::isFileValidated(const QString &filePath) const
{
    return _validatedFiles.contains(filePath);
}

const QSet<QString> &AbstractVersionControl::trackedFiles() const
{
    return _trackedFiles;
}

const QSet<QString> &AbstractVersionControl::modifiedFiles() const
{
    return _modifiedFiles;
}

const QSet<QString> &AbstractVersionControl::validatedFiles() const
{
    return _validatedFiles;
}

void AbstractVersionControl::validFile(const QSet<QString> &filesPath)
{
    Q_UNUSED(filesPath)
}

void AbstractVersionControl::inValidFile(const QSet<QString> &filesPath)
{
    Q_UNUSED(filesPath)
}

void AbstractVersionControl::checkoutFile(const QSet<QString> &filesPath)
{
    Q_UNUSED(filesPath)
}

bool AbstractVersionControl::isValid() const
{
    return true;
}

void AbstractVersionControl::analysePath()
{
}
