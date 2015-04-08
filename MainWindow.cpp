#include "MainWindow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>
#include <QKeySequence>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QTextCodec>
#include <QDragEnterEvent>
#include <QUrl>
#include <QDebug>

#include "TimeZoneDialog.h"

const qint64 MainWindow::FILETIME_EPOCH_START =
        QDateTime(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC).toMSecsSinceEpoch();

MainWindow::MainWindow(const QString& path, QWidget* parent)
    : QWidget(parent),
      timeZoneOffset(getLocalTimeZoneOffset())
{
    createUi();
    openTvProgram(path);
}

MainWindow::MainWindow(const QStringList& pathList, QWidget* parent)
    : QWidget(parent),
      timeZoneOffset(getLocalTimeZoneOffset())
{
    createUi();
    openFiles(pathList);
}

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
      timeZoneOffset(getLocalTimeZoneOffset())
{
    createUi();
}

void MainWindow::createUi()
{
    updateWindowTitle("");

    QMenuBar* menu = new QMenuBar(this);

    QMenu* fileMenu = menu->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open"), this, SLOT(onOpenActionTriggered()), QKeySequence::Open);
    fileMenu->addAction(tr("E&xit"), this, SLOT(close()));

    QMenu* optionsMenu = menu->addMenu(tr("O&ptions"));
    optionsMenu->addAction(tr("&Time zone"), this, SLOT(onTimeZoneActionTriggered()));

    tvProgramTrw = new QTreeWidget();
    tvProgramTrw->setColumnCount(2);
    tvProgramTrw->setHeaderLabels(QStringList() << tr("Time") << tr("Program"));
    tvProgramTrw->header()->setStretchLastSection(false);
    tvProgramTrw->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    tvProgramTrw->header()->setResizeMode(1, QHeaderView::Stretch);
    tvProgramTrw->setSelectionMode(QAbstractItemView::NoSelection);
    tvProgramTrw->setAnimated(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tvProgramTrw);
    layout->setMenuBar(menu);
    setLayout(layout);

    setAcceptDrops(true);
}

void MainWindow::updateWindowTitle(const QString& filename)
{
    if (filename.isEmpty())
        setWindowTitle(QApplication::applicationName());
    else
        setWindowTitle(filename + QString::fromUtf8(" \u2014 ") + QApplication::applicationName());
}

MainWindow::~MainWindow()
{
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 600);
}

void MainWindow::onOpenActionTriggered()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open JTV TV Program"),
                                                      "", tr("JTV TV Program (*.ndx *.pdt)"));
    openFiles(files);
}

void MainWindow::onTimeZoneActionTriggered()
{
    TimeZoneDialog dialog (this);
    dialog.setOffsetSeconds(timeZoneOffset);
    if (dialog.exec() == QDialog::Accepted)
    {
        timeZoneOffset = dialog.getOffsetSeconds();
        if (!currentFilePath.isEmpty())
            openTvProgram(currentFilePath);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if (!e->mimeData()->hasUrls())
        e->ignore();
    else
    {
        foreach (const QUrl url, e->mimeData()->urls())
        {
            if (url.scheme() != "file")
            {
                e->ignore();
                return;
            }
        }

        e->setDropAction(Qt::LinkAction);
        e->accept();
    }
}

void MainWindow::dropEvent(QDropEvent* e)
{
    QStringList files;
    foreach (const QUrl& url, e->mimeData()->urls())
    {
        if (url.scheme() == "file")
            files.append(url.path());
    }

    openFiles(files);

    e->setDropAction(Qt::LinkAction);
    e->accept();
}

int MainWindow::getLocalTimeZoneOffset()
{
    QDateTime local = QDateTime::currentDateTime();
    local.setTimeSpec(Qt::UTC);
    return (local.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch()) / 1000;
}

void MainWindow::openFiles(const QStringList& files)
{
    QStringList uniqFiles = files;
    qSort(uniqFiles);
    QMutableListIterator<QString> f (uniqFiles);
    while (f.hasNext())
    {
        if ( f.next().endsWith(".ndx", Qt::CaseInsensitive)
             && f.hasNext() && f.peekNext().endsWith(".pdt", Qt::CaseInsensitive)
             && f.peekNext().left(f.peekNext().length() - 4)
                        == f.peekPrevious().left(f.peekNext().length() - 4) )
            f.remove();
    }

    if (!uniqFiles.empty())
    {
        openTvProgram(uniqFiles[0]);

        uniqFiles.removeFirst();
        foreach (const QString& file, uniqFiles)
        {
            MainWindow* w = new MainWindow(file);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
        }
    }
}

void MainWindow::openTvProgram(const QString& path)
{
    QString jtvBaseName = path;
    if (jtvBaseName.endsWith(".ndx", Qt::CaseInsensitive)
        || jtvBaseName.endsWith(".pdt", Qt::CaseInsensitive))
        jtvBaseName.chop(4);

    updateWindowTitle("");
    currentFilePath.clear();
    tvProgramTrw->clear();

    try
    {
        QFile ndx (jtvBaseName + ".ndx");
        if (!ndx.open(QFile::ReadOnly))
        {
            throw tr("Failed to open %1: %2")
                  .arg(ndx.fileName())
                  .arg(ndx.errorString());
        }

        QFile pdt (jtvBaseName + ".pdt");
        if (!pdt.open(QFile::ReadOnly))
        {
            throw tr("Failed to open %1: %2")
                  .arg(pdt.fileName())
                  .arg(pdt.errorString());
        }

        quint16 entryCount;
        if (ndx.read((char*)&entryCount, sizeof(entryCount)) != sizeof(entryCount))
        {
            throw tr("Failed to read entry count from %1: %2")
                  .arg(ndx.fileName())
                  .arg(ndx.errorString());
        }

        for (uint entry = 1; entry <= entryCount; entry++)
        {
            if (ndx.read(2) != QByteArray("\x00\x00", 2))
            {
                throw tr("Failed to read entry %1 form %2: bad format (does not starts with 0x00"
                         " 0x00)")
                      .arg(entry)
                      .arg(ndx.fileName());
            }

            qint64 time;
            if (ndx.read((char*)&time, sizeof(time)) != sizeof(time))
            {
                throw tr("Failed to read entry %1 form %2: failed to read time: %3")
                      .arg(entry)
                      .arg(ndx.fileName())
                      .arg(ndx.errorString());
            }

            quint16 pdtOffset;
            if (ndx.read((char*)&pdtOffset, sizeof(pdtOffset)) != sizeof(pdtOffset))
            {
                throw tr("Failed to read entry %1 from %2: failed to read offset: %3")
                      .arg(entry)
                      .arg(ndx.fileName())
                      .arg(ndx.errorString());
            }

            if (!pdt.seek(pdtOffset))
            {
                throw tr("Failed to read entry %1 from %2: seek failed: %3")
                      .arg(entry)
                      .arg(pdt.fileName())
                      .arg(pdt.errorString());
            }

            quint16 len;
            if (pdt.read((char*)&len, sizeof(len)) != sizeof(len))
            {
                throw tr("Failed to read entry %1 from %2: failed to read size: %3")
                      .arg(entry)
                      .arg(pdt.fileName())
                      .arg(pdt.errorString());
            }

            QByteArray name = pdt.read(len);
            if (name.size() != len)
            {
                throw tr("Failed to read entry %1 from %2: failed to read name: %3")
                      .arg(entry)
                      .arg(pdt.fileName())
                      .arg(pdt.errorString());
            }

            QDateTime progTime = QDateTime::fromMSecsSinceEpoch(time / 10000 + FILETIME_EPOCH_START
                                                                - timeZoneOffset * 1000);
            QString progName = QTextCodec::codecForName("cp1251")->toUnicode(name);

            addEntry(progTime, progName);
        }

        updateWindowTitle(QFileInfo(jtvBaseName).fileName());
        currentFilePath = jtvBaseName;
    }
    catch (const QString& errorMessage) // yeah, this is a crapcode
    {
        QMessageBox::critical(this, tr("Error"), errorMessage);
    }
}

void MainWindow::addEntry(const QDateTime& time, const QString& name)
{
    QTreeWidgetItem* parentItem = 0;
    if (tvProgramTrw->topLevelItemCount() > 0)
        parentItem = tvProgramTrw->topLevelItem(tvProgramTrw->topLevelItemCount() - 1);

    if (!parentItem || parentItem->data(0, Qt::UserRole).toDate() != time.date())
    {
        if (parentItem)
            parentItem->data(0, Qt::UserRole).clear();

        parentItem = new QTreeWidgetItem(tvProgramTrw);
        parentItem->setText(0, time.date().toString(Qt::SystemLocaleLongDate));
        parentItem->setData(0, Qt::UserRole, time.date());
        parentItem->setFirstColumnSpanned(true);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    item->setText(0, time.time().toString(Qt::SystemLocaleShortDate));
    item->setText(1, name);
}
