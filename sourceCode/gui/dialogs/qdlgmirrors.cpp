#include "qdlgmirrors.h"
#include "ui_qdlgmirrors.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgcolor.h"
#include "app.h"
#include "simStrings.h"

CQDlgMirrors::CQDlgMirrors(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgMirrors)
{
    _dlgType=MIRROR_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgMirrors::~CQDlgMirrors()
{
    delete ui;
}

void CQDlgMirrors::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgMirrors::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::currentWorld->simulation->isSimulationStopped();

    CMirror* it=App::currentWorld->sceneObjects->getLastSelectionMirror();

    ui->qqDisableAllMirrors->setEnabled(noEditMode);
    ui->qqDisableAllClippingPlanes->setEnabled(noEditMode);

    ui->qqDisableAllMirrors->setChecked(App::currentWorld->mainSettings->mirrorsDisabled);
    ui->qqDisableAllClippingPlanes->setChecked(App::currentWorld->mainSettings->clippingPlanesDisabled);


    ui->qqEnabled->setEnabled((it!=nullptr)&&noEditModeAndNoSim);
    ui->qqWidth->setEnabled((it!=nullptr)&&noEditModeAndNoSim);
    ui->qqHeight->setEnabled((it!=nullptr)&&noEditModeAndNoSim);
    ui->qqReflectance->setEnabled((it!=nullptr)&&it->getIsMirror()&&noEditModeAndNoSim);
    ui->qqColor->setEnabled((it!=nullptr)&&noEditModeAndNoSim);

    ui->qqIsMirror->setEnabled((it!=nullptr)&&noEditModeAndNoSim);
    ui->qqIsClippingPlane->setEnabled((it!=nullptr)&&noEditModeAndNoSim);

    ui->qqEntityCombo->setEnabled((it!=nullptr)&&noEditModeAndNoSim);
    ui->qqEntityCombo->clear();

    if (it!=nullptr)
    {
        ui->qqEnabled->setChecked(it->getActive()&&noEditModeAndNoSim);

        ui->qqWidth->setText(tt::getFString(false,it->getMirrorWidth(),3).c_str());
        ui->qqHeight->setText(tt::getFString(false,it->getMirrorHeight(),3).c_str());
        ui->qqReflectance->setText(tt::getFString(false,it->getReflectance(),2).c_str());
        ui->qqIsMirror->setChecked(it->getIsMirror());
        ui->qqIsClippingPlane->setChecked(!it->getIsMirror());

        if (!it->getIsMirror())
        {

            ui->qqEntityCombo->addItem(IDS_ALL_VISIBLE_OBJECTS_IN_SCENE,QVariant(-1));

            std::vector<std::string> names;
            std::vector<int> ids;

            // Now collections:
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
            {
                CCollection* it=App::currentWorld->collections->getObjectFromIndex(i);
                std::string name(tt::decorateString("[",IDSN_COLLECTION,"] "));
                name+=it->getCollectionName();
                names.push_back(name);
                ids.push_back(it->getCollectionHandle());
            }
            tt::orderStrings(names,ids);
            for (int i=0;i<int(names.size());i++)
                ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

            names.clear();
            ids.clear();

            // Now objects:
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
                std::string name(tt::decorateString("[",IDS_OBJECT,"] "));
                name+=it->getObjectAlias_printPath();
                names.push_back(name);
                ids.push_back(it->getObjectHandle());
            }
            tt::orderStrings(names,ids);
            for (int i=0;i<int(names.size());i++)
                ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

            // Select current item:
            for (int i=0;i<ui->qqEntityCombo->count();i++)
            {
                if (ui->qqEntityCombo->itemData(i).toInt()==it->getClippingObjectOrCollection())
                {
                    ui->qqEntityCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    else
    {
        ui->qqEnabled->setChecked(false);
        ui->qqIsMirror->setChecked(true);
        ui->qqIsClippingPlane->setChecked(false);
        ui->qqWidth->setText("");
        ui->qqHeight->setText("");
        ui->qqReflectance->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgMirrors::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLED_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqWidth_editingFinished()
{
    if (!ui->qqWidth->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqWidth->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_WIDTH_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqHeight_editingFinished()
{
    if (!ui->qqHeight->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqHeight->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_HEIGHT_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMirror* it=App::currentWorld->sceneObjects->getLastSelectionMirror();
        if (it!=nullptr)
        {
            if (it->getIsMirror())
                CQDlgColor::displayDlg(COLOR_ID_MIRROR,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,0,App::mainWindow);
            else
                CQDlgMaterial::displayMaterialDlg(COLOR_ID_CLIPPINGPLANE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
        }
    }
}


void CQDlgMirrors::on_qqReflectance_editingFinished()
{
    if (!ui->qqReflectance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqReflectance->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_REFLECTANCE_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqIsMirror_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_MIRRORFUNC_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqIsClippingPlane_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_CLIPPINGFUNC_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqEntityCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int objID=ui->qqEntityCombo->itemData(ui->qqEntityCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_CLIPPINGENTITY_MIRRORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMirrors::on_qqDisableAllMirrors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DISABLEALLMIRRORS_MIRRORGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMirrors::on_qqDisableAllClippingPlanes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DISABLEALLCLIPPING_MIRRORGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
