#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searcher.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      searcher( nullptr),
      thread(nullptr)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setUniformRowHeights(1);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    ui->statusBar->addPermanentWidget(ui->deleteButton);
    ui->statusBar->addPermanentWidget(ui->stopButton);
    ui->statusBar->addPermanentWidget(ui->progressBar);

    ui->progressBar->setHidden(true);
    ui->progressBar->setRange(0, 100);
    ui->deleteButton->setHidden(true);
    ui->stopButton->setHidden(true);

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->actionStop, &QAction::triggered, this, &main_window::finish);
    connect(ui->stopButton, &QPushButton::clicked, this, &main_window::stopSearching);
    connect(ui->deleteButton, &QPushButton::clicked, this, &main_window::deleteFiles);
}

main_window::~main_window()
{
    if (thread != nullptr) {
        thread->exit();
        thread->wait();
    }
}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    ui->treeWidget->clear();

    searcher = new Searcher(dir);
    thread = new  QThread();
    searcher->moveToThread(thread);

    setWindowTitle(QString("Find dublicates..."));

    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    ui->deleteButton->setHidden(true);
    ui->progressBar->setHidden(false);
    ui->progressBar->setValue(0);
    ui->stopButton->setHidden(false);
    ui->actionScan_Directory->setDisabled(true);
    connect(thread, SIGNAL(started()), searcher, SLOT(getDublicates()));
    connect(searcher, SIGNAL(finished), thread, SLOT(quit()));
    connect(searcher, SIGNAL(progress(int)), this, SLOT(setProgress(int)));
    connect(searcher, &Searcher::sendDublicates, this, &main_window::showDublicates);
    connect(searcher, SIGNAL(finishSearching()), this, SLOT(finish()));
    connect(ui->treeWidget,
            SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this,
            SLOT(openFile(QTreeWidgetItem*, int)));
    thread->start();
}

void main_window::showDublicates(QVector<QString> const& dublicates) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    QFileInfo fileInfo(dublicates[0]);
    item->setText(0, QString("There is " + QString::number(dublicates.size()) + " dublicated files"));
    item->setText(2, QString::number(fileInfo.size()));
    for (QString const& filePath: dublicates) {
        QFileInfo file(filePath);
        QTreeWidgetItem* childItem = new QTreeWidgetItem();
        childItem->setText(0, file.baseName());
        childItem->setText(1, file.filePath());
        item->addChild(childItem);
    }
    ui->treeWidget->addTopLevelItem(item);
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}

void main_window::setProgress(int percent) {
    ui->progressBar->setValue(percent);
}

void main_window::finish() {
    setWindowTitle(QString("Done"));
    ui->stopButton->setHidden(true);
    ui->deleteButton->setHidden(false);
    ui->actionScan_Directory->setDisabled(false);
    thread->quit();
    thread->wait();
}

void main_window::stopSearching() {
    thread->requestInterruption();
}

void main_window::openFile(QTreeWidgetItem* item, int) {
    QFile file(item->text(1));
    if (file.exists()) {
        QDesktopServices::openUrl(item->text(1));
    }
}

void main_window::deleteFiles() {
    auto selectedItems = ui->treeWidget->selectedItems();
    QSet<QTreeWidgetItem* > toDelete;
    for (auto const& selectedItem: selectedItems) {
        if (!selectedItem->childCount()) {
            toDelete.insert(selectedItem);
        } else {
            auto children = selectedItem->takeChildren();
            for (auto const& child: children) {
                toDelete.insert(child);
                selectedItem->addChild(child);
            }
        }
    }

    auto answer = QMessageBox::question(this, "Deleting",
                                        "Do you want to delete " + QString::number(toDelete.size()) + " selected files?");
    if (answer == QMessageBox::No) {
        return;
    }

    QSet<QTreeWidgetItem*> changedItems;

    for (auto const& item: toDelete) {
        QFile file(item->text(1));
        if (!file.exists() || file.remove()) {
            changedItems.insert(item->parent());
            item->parent()->removeChild(item);
        }
    }

    for (auto const& item: changedItems) {
        if (item->childCount() > 1) {
            item->setText(2, "There is " + QString::number(item->childCount()) + " dublicated files");
        } else {
            delete item;
        }
    }
}
