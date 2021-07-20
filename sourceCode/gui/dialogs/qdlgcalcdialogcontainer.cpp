#include "qdlgcalcdialogcontainer.h"
#include "ui_qdlgcalcdialogcontainer.h"
#include "app.h"
#include "qdlgcollisions.h"
#include "qdlgdistances.h"
#include "qdlgdynamics.h"
#include "qdlgik.h"

CQDlgCalcDialogContainer::CQDlgCalcDialogContainer(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgCalcDialogContainer)
{
    _dlgType=CALCULATION_DLG;
    ui->setupUi(this);

    topBorderWidth=0;
    if (App::userSettings->showOldDlgs)
    {
        topBorderWidth=35;
        QRect geom=ui->qqGroupBox->geometry();
        geom.setHeight(57);
        ui->qqGroupBox->setGeometry(geom);
    }
    else
    {
        ui->qqGroupBox->setVisible(false);
        setWindowTitle("Dynamics");
    }

    if (App::userSettings->showOldDlgs)
    {
        pageDlgs[0]=new CQDlgCollisions();
        originalHeights[0]=pageDlgs[0]->size().height();

        pageDlgs[1]=new CQDlgDistances();
        originalHeights[1]=pageDlgs[1]->size().height();

        pageDlgs[2]=new CQDlgIk();
        originalHeights[2]=pageDlgs[2]->size().height();

        pageDlgs[3]=new CQDlgDynamics();
        originalHeights[3]=pageDlgs[3]->size().height();
    }
    else
    {
        pageDlgs[0]=new CQDlgDynamics();
        originalHeights[0]=pageDlgs[0]->size().height();
        ui->qqCollision->setVisible(false);
        ui->qqDistance->setVisible(false);
        ui->qqIk->setVisible(false);
        ui->qqDynamics->setVisible(false);
    }

    currentPage=0;
    desiredPage=0;
    bl=new QVBoxLayout();
    bl->setContentsMargins(0,topBorderWidth,0,0);
    setLayout(bl);
    if (App::userSettings->showOldDlgs)
    {
        bl->addWidget(pageDlgs[0]);
        bl->addWidget(pageDlgs[1]);
        pageDlgs[1]->setVisible(false);
        bl->addWidget(pageDlgs[2]);
        pageDlgs[2]->setVisible(false);
        bl->addWidget(pageDlgs[3]);
        pageDlgs[3]->setVisible(false);
    }
    else
        bl->addWidget(pageDlgs[0]);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]+topBorderWidth);
    setFixedSize(s);
}

CQDlgCalcDialogContainer::~CQDlgCalcDialogContainer()
{
    delete ui;
}

void CQDlgCalcDialogContainer::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    pageDlgs[currentPage]->dialogCallbackFunc(cmdIn,cmdOut);
}

void CQDlgCalcDialogContainer::refresh()
{
    if (App::userSettings->showOldDlgs)
    {
        ui->qqCollision->setChecked(desiredPage==0);
        ui->qqDistance->setChecked(desiredPage==1);
        ui->qqIk->setChecked(desiredPage==2);
        ui->qqDynamics->setChecked(desiredPage==3);
    }
    else
        ui->qqDynamics->setChecked(desiredPage==0);

    if (desiredPage!=currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage=desiredPage;
        pageDlgs[currentPage]->setVisible(true);

        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]+topBorderWidth);

        setFixedSize(s);
    }
    pageDlgs[currentPage]->refresh();
}

void CQDlgCalcDialogContainer::on_qqCollision_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=0;
            refresh();
        }
    }
}

void CQDlgCalcDialogContainer::on_qqDistance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=1;
            refresh();
        }
    }
}

void CQDlgCalcDialogContainer::on_qqIk_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=2;
            refresh();
        }
    }
}

void CQDlgCalcDialogContainer::on_qqDynamics_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
            desiredPage=3;
        else
            desiredPage=0;
        refresh();
    }
}

