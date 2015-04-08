#pragma once

#include <QWidget>
#include <QTreeWidget>

class MainWindow : public QWidget
{
    Q_OBJECT
        
    public:

        explicit MainWindow(const QString& path, QWidget* parent = 0);
        explicit MainWindow(const QStringList& pathList, QWidget* parent = 0);
        explicit MainWindow(QWidget* parent = 0);

        virtual ~MainWindow();

    protected:

        virtual QSize sizeHint() const;

        virtual void dragEnterEvent(QDragEnterEvent* e);
        virtual void dropEvent(QDropEvent* e);

    private:

        static const qint64 FILETIME_EPOCH_START;

        int timeZoneOffset;
        QString currentFilePath;

        QTreeWidget* tvProgramTrw;

        static int getLocalTimeZoneOffset();

        void createUi();
        void updateWindowTitle(const QString& filename);
        void openFiles(const QStringList& files);
        void openTvProgram(const QString& path);
        void addEntry(const QDateTime& time, const QString& name);

    private slots:

        void onOpenActionTriggered();
        void onTimeZoneActionTriggered();
};
