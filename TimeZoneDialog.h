#pragma once

#include <QDialog>
#include <QSpinBox>

class TimeZoneDialog : public QDialog
{
    Q_OBJECT

    public:

        explicit TimeZoneDialog(QWidget* parent = 0);

        void setOffsetSeconds(int seconds);
        int getOffsetSeconds() const;

    private:

        QSpinBox* offsetSbx;

        void createUi();
};
