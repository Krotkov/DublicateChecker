#include "searcher.h"
#include <QDir>
#include <QMap>
#include <QVector>
#include <QString>
#include <QThread>
#include <QDirIterator>
#include <QCryptographicHash>

const qint64 MAXN = 10;

bool Searcher::checkStop() {
    return QThread::currentThread()->isInterruptionRequested();
}

void Searcher::getDublicates() {
    QMap <qint64, QVector<QString>> firstGroups;
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        if (checkStop()) {
            emit finishSearching();
            return;
        }
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        sumSize += fileInfo.size();
        firstGroups[fileInfo.size()].push_back(filePath);
    }
    if (sumSize == 0) {
        emit progress(100);
        emit finishSearching();
        return;
    }
    qint64 curSize = 0;
    for (auto firstIt = firstGroups.begin(); firstIt != firstGroups.end(); firstIt++) {
        if (checkStop()) {
            emit finishSearching();
            return;
        }
        QMap <QString, QVector<QString>> secondGroups;
        for (auto filePath: firstIt.value()) {
            if (checkStop()) {
                emit finishSearching();
                return;
            }
            secondGroups[getPrefix(filePath, std::min(MAXN, firstIt.key()))].push_back(filePath);
            curSize += firstIt.key();
            updateProgress(curSize);
        }
        if (checkStop()) {
            emit finishSearching();
            return;
        }
        for (auto secondIt = secondGroups.begin(); secondIt != secondGroups.end(); secondIt++) {
            if (checkStop()) {
                emit finishSearching();
                return;
            }
            QMap <QByteArray, QVector<QString>> finalGroups;
            for (auto filePath: secondIt.value()) {
                if (checkStop()) {
                    emit finishSearching();
                    return;
                }
                finalGroups[getHash(filePath)].push_back(filePath);
                curSize += firstIt.key();
                updateProgress(curSize);
            }
            if (checkStop()) {
                emit finishSearching();
                return;
            }
            QVector<QVector<QString>> dublicates;
            for (auto finalIt = finalGroups.begin(); finalIt != finalGroups.end(); finalIt++) {
                if (checkStop()) {
                    emit finishSearching();
                    return;
                }
                if (finalIt.value().size() != 1) emit sendDublicates(finalIt.value());
            }
        }
    }
    emit finishSearching();
}


QString Searcher::getPrefix(QString const& filePath, qint64 number) {
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QString prefix = file.read(number);
        return prefix;
    }
}

QByteArray Searcher::getHash (QString const& filePath) {
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash fileHash(QCryptographicHash::Sha256);
        if (fileHash.addData(&file)) {
            return fileHash.result();
        }
    }
    return QByteArray();
}

void Searcher::updateProgress(qint64 curSize) {
    qint64 newPercent = curSize * 50 / (sumSize);
    if (newPercent > percent) {
        percent = newPercent;
        emit progress(percent);
    }
}


