#include "qdlgenvironment.h"
#include "ui_qdlgenvironment.h"
#include "gV.h"
#include "qdlgcolor.h"
#include "qdlgmaterial.h"
#include "tt.h"
#include "app.h"
#include "simStrings.h"
#include "vMessageBox.h"

CQDlgEnvironment::CQDlgEnvironment(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgEnvironment)
{
    _dlgType=ENVIRONMENT_DLG;
    ui->setupUi(this);
}

CQDlgEnvironment::~CQDlgEnvironment()
{
    delete ui;
}

void CQDlgEnvironment::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    ui->qqNextSaveIsDefinitive->setEnabled((!App::currentWorld->environment->getSceneLocked())&&noEditModeNoSim);
    ui->qqExtensionString->setEnabled(noEditModeNoSim);
    ui->qqBackgroundColorUp->setEnabled(noEditModeNoSim);
    ui->qqBackgroundColorDown->setEnabled(noEditModeNoSim);
    ui->qqAmbientLightColor->setEnabled(noEditModeNoSim);
    ui->qqFogAdjust->setEnabled(noEditModeNoSim);

    ui->qqMaxTriangleSize->setEnabled(noEditModeNoSim);
    ui->qqMinRelTriangleSize->setEnabled(noEditModeNoSim);
    ui->qqSaveCalcStruct->setEnabled(noEditModeNoSim);
    ui->qqShapeTexturesDisabled->setEnabled(noEditModeNoSim);
//    ui->qqUserInterfaceTexturesDisabled->setEnabled(noEditModeNoSim);
    ui->qqAcknowledgments->setEnabled(noEditModeNoSim);

    ui->qqMaxTriangleSize->setText(tt::getEString(false,App::currentWorld->environment->getCalculationMaxTriangleSize(),2).c_str());
    ui->qqMinRelTriangleSize->setText(tt::getFString(false,App::currentWorld->environment->getCalculationMinRelTriangleSize(),3).c_str());
    ui->qqSaveCalcStruct->setChecked(App::currentWorld->environment->getSaveExistingCalculationStructures());
    ui->qqShapeTexturesDisabled->setChecked(!App::currentWorld->environment->getShapeTexturesEnabled());
//    ui->qqUserInterfaceTexturesDisabled->setChecked(!App::currentWorld->environment->get2DElementTexturesEnabled());

    ui->qqNextSaveIsDefinitive->setChecked(App::currentWorld->environment->getRequestFinalSave());

    ui->qqExtensionString->setText(App::currentWorld->environment->getExtensionString().c_str());
    ui->qqAcknowledgments->setPlainText(App::currentWorld->environment->getAcknowledgement().c_str());

    selectLineEdit(lineEditToSelect);
}

void CQDlgEnvironment::on_qqBackgroundColorUp_clicked()
{
    CQDlgColor::displayDlg(COLOR_ID_BACKGROUND_UP,-1,-1,0,App::mainWindow);
}

void CQDlgEnvironment::on_qqBackgroundColorDown_clicked()
{
    CQDlgColor::displayDlg(COLOR_ID_BACKGROUND_DOWN,-1,-1,0,App::mainWindow);
}

void CQDlgEnvironment::on_qqAmbientLightColor_clicked()
{
    CQDlgColor::displayDlg(COLOR_ID_AMBIENT_LIGHT,-1,-1,0,App::mainWindow);
}

void CQDlgEnvironment::on_qqFogAdjust_clicked()
{
    App::mainWindow->dlgCont->processCommand(OPEN_FOG_DLG_CMD);
}

void CQDlgEnvironment::on_qqMaxTriangleSize_editingFinished()
{
    if (!ui->qqMaxTriangleSize->isModified())
        return;
    bool ok;
    float newVal=ui->qqMaxTriangleSize->text().toFloat(&ok);
    if (ok)
    {
        App::appendSimulationThreadCommand(SET_MAXTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD,-1,-1,newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgEnvironment::on_qqSaveCalcStruct_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::currentWorld->environment->getSaveExistingCalculationStructures())
            App::uiThread->messageBox_information(App::mainWindow,IDSN_CALCULATION_STRUCTURE,IDS_SAVING_CALCULATION_STRUCTURE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        App::appendSimulationThreadCommand(TOGGLE_SAVECALCSTRUCT_ENVIRONMENTGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEnvironment::on_qqShapeTexturesDisabled_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_SHAPETEXTURES_ENVIRONMENTGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgEnvironment::on_qqNextSaveIsDefinitive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::currentWorld->environment->getRequestFinalSave())
            App::uiThread->messageBox_information(App::mainWindow,IDSN_SCENE_LOCKING,IDS_SCENE_LOCKING_INFO,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        App::appendSimulationThreadCommand(TOGGLE_LOCKSCENE_ENVIRONMENTGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEnvironment::on_qqAcknowledgments_textChanged()
{
    std::string txt=ui->qqAcknowledgments->toPlainText().toStdString();
    tt::removeSpacesAndEmptyLinesAtBeginningAndEnd(txt);
    // No refresh here!! (otherwise we can't edit the item properly)
    App::appendSimulationThreadCommand(SET_ACKNOWLEDGMENT_ENVIRONMENTGUITRIGGEREDCMD,-1,-1,0.0,0.0,txt.c_str());
}

void CQDlgEnvironment::on_qqMinRelTriangleSize_editingFinished()
{
    if (!ui->qqMinRelTriangleSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMinRelTriangleSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MINTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD,-1,-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEnvironment::on_qqExtensionString_editingFinished()
{
    if (!ui->qqExtensionString->isModified())
        return;
    App::appendSimulationThreadCommand(SET_EXTSTRING_ENVIRONMENTGUITRIGGEREDCMD,-1,-1,0.0,0.0,ui->qqExtensionString->text().toStdString().c_str());
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}
