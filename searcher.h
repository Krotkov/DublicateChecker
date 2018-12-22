#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>

class Searcher : public QObject {
    Q_OBJECT
public:
    explicit Searcher(QString const& dir) {
        this->dir = dir;
        sumSize = 0;
        percent = 0;
    }
public:
    signals:
    void sendDublicates(QVector<QString> const&);
    void progress(int percent);
    void finishSearching();
public:
    virtual ~Searcher() = default;
public slots:
    void getDublicates();
private:
    QByteArray getHash(QString const&);
    QString getPrefix(QString const& filepath, qint64 number);
    QString dir;
    qint64 sumSize;
    qint64 percent;
    void updateProgress(qint64 curSize);
    bool checkStop();
};

#endif // SEARCHER_H
