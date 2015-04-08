#include "TimeZoneDialog.h"

#include <QFormLayout>
#include <QBoxLayout>
#include <QDialogButtonBox>

TimeZoneDialog::TimeZoneDialog(QWidget* parent)
    : QDialog(parent)
{
    createUi();
}

void TimeZoneDialog::createUi()
{
    setWindowTitle(tr("Time zone options"));

    offsetSbx = new QSpinBox();
    offsetSbx->setMinimum(-24 * 60 * 60);
    offsetSbx->setMaximum(+24 * 60 * 60);
    offsetSbx->setValue(0);
    offsetSbx->setSuffix(tr(" sec"));

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));

    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Source file time zone:"), offsetSbx);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(form);
    layout->addWidget(box);

    setLayout(layout);
}

void TimeZoneDialog::setOffsetSeconds(int seconds)
{
    offsetSbx->setValue(seconds);
}

int TimeZoneDialog::getOffsetSeconds() const
{
    return offsetSbx->value();
}
