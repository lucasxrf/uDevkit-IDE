#include "gitversioncontrol.h"

#include <QDirIterator>
#include <QDebug>
#include <QProcess>
#include <QTextStream>

GitVersionControl::GitVersionControl()
    : AbstractVersionControl ()
{
    _indexWatcher = Q_NULLPTR;
    _statusState = StatusNone;
    _diffState = DiffNone;

    _processGitState = new QProcess(this);
    //connect(_process, &QProcess::finished, this, &GitVersionControl::parseModifiedFiles); // does not work...
    connect(_processGitState, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int, QProcess::ExitStatus){processEnd();}); // but this crap is recomended

    _processGitDiff = new QProcess(this);
    connect(_processGitDiff, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int, QProcess::ExitStatus){processDiffEnd();}); // but this crap is recomended

    _settingsClass = rtset()->registerClass("gitversion");
    connect(_settingsClass, &SettingsClass::classModified, this, &GitVersionControl::updateSettings);
    updateSettings();
}

GitVersionControl::~GitVersionControl()
{
    _processGitState->deleteLater();
}

QString GitVersionControl::versionControlName() const
{
    return "git";
}

QString GitVersionControl::basePath() const
{
    return _basePath;
}

void GitVersionControl::validFile(const QSet<QString> &filesPath)
{
    if (!isValid())
        return;
    QStringList args;
    args << "add";

    QDir dir(_basePath);
    foreach (QString filePath, filesPath)
        args<<dir.relativeFilePath(filePath);

    QProcess *newProcess = new QProcess(this);
    newProcess->setWorkingDirectory(_basePath);
    connect(newProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int, QProcess::ExitStatus){delete sender();});
    newProcess->start(_programPath, args);
}

void GitVersionControl::inValidFile(const QSet<QString> &filesPath)
{
    if (!isValid())
        return;
    QStringList args;
    args << "reset" << "HEAD";

    QDir dir(_basePath);
    foreach (QString filePath, filesPath)
        args<<dir.relativeFilePath(filePath);

    QProcess *newProcess = new QProcess(this);
    newProcess->setWorkingDirectory(_basePath);
    connect(newProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int, QProcess::ExitStatus){delete sender();});
    newProcess->start(_programPath, args);
}

void GitVersionControl::checkoutFile(const QSet<QString> &filesPath)
{
    if (!isValid())
        return;
    QStringList args;
    args << "checkout";

    QDir dir(_basePath);
    foreach (QString filePath, filesPath)
        args<<dir.relativeFilePath(filePath);

    QProcess *newProcess = new QProcess(this);
    newProcess->setWorkingDirectory(_basePath);
    connect(newProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int, QProcess::ExitStatus){delete sender();});
    newProcess->start(_programPath, args);
}

bool GitVersionControl::isValid() const
{
    return !_gitPath.isEmpty();
}

void GitVersionControl::requestFileModifications(const QString &filePath)
{
    _diffRequestQueue.enqueue(filePath);
    if (_processGitDiff->state() == QProcess::NotRunning)
        reqFileModifHead(_diffRequestQueue.head());
}

void GitVersionControl::reqFetch()
{
    _statusState = StatusFetch;
    _processGitState->start(_programPath, QStringList()<<"fetch");
}

void GitVersionControl::reqModifFiles()
{
    _statusState = StatusModifiedFiles;
    _processGitState->start(_programPath, QStringList()<<"ls-files"<<"-m"<<".");
}

void GitVersionControl::reqTrackedFiles()
{
    _statusState = StatusTrackedFiles;
    _processGitState->start(_programPath, QStringList()<<"ls-files"<<".");
}

void GitVersionControl::reqValidatedFiles()
{
    _statusState = StatusValidatedFiles;
    _processGitState->start(_programPath, QStringList()<<"diff"<<"--cached"<<"--name-only");
}

void GitVersionControl::indexCheck()
{
    _indexWatcher->addPath(_gitPath+"index");
    reqModifFiles();
}

void GitVersionControl::processEnd()
{
    QSet<QString> newmodifiedFiles, validedFile;

    switch (_statusState) {
    case GitVersionControl::StatusNone:
        break;
    case GitVersionControl::StatusFetch:
        reqModifFiles();
        break;
    case GitVersionControl::StatusCheck:
        if (_processGitState->exitCode() != 0)
            reqFetch();
        else
            reqModifFiles();
        break;
    case GitVersionControl::StatusModifiedFiles:
        parseFilesList(_modifiedFiles, validedFile, newmodifiedFiles);
        reqTrackedFiles();
        break;
    case GitVersionControl::StatusTrackedFiles:
        parseFilesList(_trackedFiles, validedFile, newmodifiedFiles);
        reqValidatedFiles();
        break;
    case GitVersionControl::StatusValidatedFiles:
        parseFilesList(_validatedFiles, validedFile, newmodifiedFiles);
        break;
    }

    if (!newmodifiedFiles.isEmpty())
        emit newModifiedFiles(newmodifiedFiles);
    if (!validedFile.isEmpty())
        emit newValidatedFiles(validedFile);
}

void GitVersionControl::updateSettings()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString gitPath = _settingsClass->setting("path").toString();
#if defined(Q_OS_WIN)
    QChar listSep = ';';
#else
    QChar listSep = ':';
#endif
    if (!gitPath.isEmpty())
    {
        _programPath = gitPath + QDir::separator() + "git";
        gitPath = QDir::toNativeSeparators(gitPath);
        env.insert("PATH", gitPath + listSep + env.value("PATH"));
    }
    else
        _programPath = "git";
    _processGitState->setProcessEnvironment(env);
    _processGitDiff->setProcessEnvironment(env);
}

void GitVersionControl::reqFileModifHead(const QString &filePath)
{
    QDir dir(_basePath);
    _diffState = DiffHead;
    _fileGitDiff = filePath;
    _fileChanges.clear();
    _processGitDiff->start(_programPath, QStringList()<<"diff"<<"HEAD"<<"--no-color"<<"--unified=0"<<dir.relativeFilePath(filePath));
}

void GitVersionControl::reqFileModifNormal(const QString &filePath)
{
    QDir dir(_basePath);
    _diffState = DiffNormal;
    _fileGitDiff = filePath;
    _processGitDiff->start(_programPath, QStringList()<<"diff"<<"--no-color"<<"--unified=0"<<dir.relativeFilePath(filePath));
}

void GitVersionControl::processDiffEnd()
{
    QTextStream stream(_processGitDiff);
    QRegExp regContext("@@ -([0-9]+)(,([0-9]+))* \\+([0-9]+)(,([0-9]+))* @@");
    bool valid = false;

    switch (_diffState)
    {
    case GitVersionControl::DiffNone:
        break;
    case GitVersionControl::DiffHead:
    {
        VersionChange *change = new VersionChange();
        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            if (line.startsWith("@@")) // new modif
            {
                if (valid && change->isValid())
                {
                    _fileChanges.changes().append(change);
                    change = new VersionChange();
                }

                regContext.indexIn(line);
                int lineOld = regContext.cap(1).toInt();
                int lineNew = regContext.cap(4).toInt();
                change->setLineOld(lineOld);
                change->setLineNew(lineNew);
                change->setStaged(true);
                valid = true;
            }
            if (!valid)
                continue;
            if (line.startsWith('+'))
                change->addAddedLine(line.mid(1));
            else if (line.startsWith('-'))
                change->addRemovedLine(line.mid(1));
        }
        if (valid && change->isValid())
            _fileChanges.changes().append(change);
        else
            delete change;
        reqFileModifNormal(_fileGitDiff);
        break;
    }
    case GitVersionControl::DiffNormal:
        while (!stream.atEnd())
        {
            QString line = stream.readLine();

            if (line.startsWith("@@")) // new modif
            {
                regContext.indexIn(line);
                int lineNew = regContext.cap(4).toInt();
                QList<VersionChange *> changesForRange = _fileChanges.changesForRange(lineNew, lineNew);
                if (!changesForRange.isEmpty())
                    changesForRange.first()->setStaged(false);
            }
        }

        _changeForFile[_fileGitDiff] = _fileChanges;
        _diffRequestQueue.dequeue();
        emit fileModificationsAvailable(_fileGitDiff);
        _diffState = DiffNone;
        if (!_diffRequestQueue.isEmpty())
            reqFileModifHead(_diffRequestQueue.head());
        break;
    }
}

void GitVersionControl::parseFilesList(QSet<QString> &oldSed, QSet<QString> &outgoingFiles, QSet<QString> &incomingFiles)
{
    QSet<QString> modifiedFiles;
    if (_processGitState->exitStatus() == QProcess::NormalExit)
    {
        QTextStream stream(_processGitState);
        while (!stream.atEnd())
            modifiedFiles.insert(_basePath+"/"+stream.readLine(1000));
    }
    //if (!oldSed.isEmpty())
    {
        outgoingFiles = oldSed;
        outgoingFiles.subtract(modifiedFiles);

        incomingFiles = modifiedFiles;
        incomingFiles.subtract(oldSed);
    }

    oldSed = modifiedFiles;
}

void GitVersionControl::findGitDir()
{
    QDir dir(_path);
    do
    {
        if (dir.exists(".git"))
        {
            if (dir.exists(".git/index"))
            {
                _gitPath = dir.canonicalPath() + "/.git/";
                _basePath = dir.path();
                return;
            }
            else
            {
                QString parentPath;
                QFile gitFile(dir.canonicalPath() + "/.git");
                if (!gitFile.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;
                QTextStream stream(&gitFile);
                QString word;
                while (!stream.atEnd())
                {
                    stream >> word;
                    if (word != "gitdir:")
                        continue;
                    stream >> parentPath;
                    if (dir.exists(parentPath))
                    {
                        _gitPath = dir.absoluteFilePath(parentPath) +"/";
                        _basePath = dir.path();
                        return;
                    }
                }
            }
        }
    } while(dir.cdUp());
}

void GitVersionControl::analysePath()
{
    findGitDir();
    if (_gitPath.isEmpty())
        return;

    delete _indexWatcher;
    _indexWatcher = new QFileSystemWatcher();
    _indexWatcher->addPath(_gitPath + "index");
    _processGitState->setWorkingDirectory(_basePath);
    _processGitDiff->setWorkingDirectory(_basePath);
    connect(_indexWatcher, &QFileSystemWatcher::fileChanged, this, &GitVersionControl::indexCheck);

    _statusState = StatusCheck;
    _processGitState->start(_programPath, QStringList()<<"status");
}
