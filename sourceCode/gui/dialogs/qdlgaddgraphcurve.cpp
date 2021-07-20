#include "qdlgaddgraphcurve.h"
#include "ui_qdlgaddgraphcurve.h"
#include "gV.h"
#include "tt.h"
#include "graphingRoutines_old.h"
#include "app.h"


CQDlgAddGraphCurve::CQDlgAddGraphCurve(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgAddGraphCurve)
{
    ui->setupUi(this);
}

CQDlgAddGraphCurve::~CQDlgAddGraphCurve()
{
    delete ui;
}

void CQDlgAddGraphCurve::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgAddGraphCurve::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgAddGraphCurve::refresh()
{
    CGraph* it=App::currentWorld->sceneObjects->getLastSelectionGraph();
    ui->qqZValue->setVisible(!xyGraph);
    ui->qqComboZ->setVisible(!xyGraph);

    ui->qqComboX->clear();
    ui->qqComboY->clear();
    ui->qqComboZ->clear();
    ui->qqComboX->addItem("0.0",QVariant(-1));
    ui->qqComboY->addItem("0.0",QVariant(-1));
    ui->qqComboZ->addItem("0.0",QVariant(-1));
    for (size_t i=0;i<it->dataStreams_old.size();i++)
    {
        int theID=it->dataStreams_old[i]->getIdentifier();
        ui->qqComboX->addItem(it->dataStreams_old[i]->getName().c_str(),QVariant(theID));
        ui->qqComboY->addItem(it->dataStreams_old[i]->getName().c_str(),QVariant(theID));
        if (!xyGraph)
            ui->qqComboZ->addItem(it->dataStreams_old[i]->getName().c_str(),QVariant(theID));
    }
}

void CQDlgAddGraphCurve::on_qqOkCancel_accepted()
{
    dataIDX=ui->qqComboX->itemData(ui->qqComboX->currentIndex()).toInt();
    dataIDY=ui->qqComboY->itemData(ui->qqComboY->currentIndex()).toInt();
    dataIDZ=ui->qqComboZ->itemData(ui->qqComboZ->currentIndex()).toInt();
    defaultModalDialogEndRoutine(true);
}

void CQDlgAddGraphCurve::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
