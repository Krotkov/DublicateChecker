#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <searcher.h>
#include <QThread>
#include <QTreeWidgetItem>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

public slots:
    void showDublicates(QVector<QString> const&);
    void setProgress(int);
    //void setProgressRange(int);

private slots:
    void finish();
    void stopSearching();
    void select_directory();
    void openFile(QTreeWidgetItem* item, int);
    //void scan_directory(QString const& dir);
    void show_about_dialog();
    void deleteFiles();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    Searcher * searcher;
    QThread * thread;
};

#endif // MAINWINDOW_H
