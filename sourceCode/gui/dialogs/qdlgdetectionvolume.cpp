#include "qdlgdetectionvolume.h"
#include "ui_qdlgdetectionvolume.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "simStrings.h"

bool CQDlgDetectionVolume::showVolumeWindow=false;

CQDlgDetectionVolume::CQDlgDetectionVolume(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgDetectionVolume)
{
    _dlgType=DETECTION_VOLUME_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgDetectionVolume::~CQDlgDetectionVolume()
{
    delete ui;
}

void CQDlgDetectionVolume::cancelEvent()
{ // no cancel event allowed
    showVolumeWindow=false;
    CDlgEx::cancelEvent();
    App::setFullDialogRefreshFlag();
}

void CQDlgDetectionVolume::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    bool prox=App::currentWorld->sceneObjects->isLastSelectionAProxSensor();
    bool mill=App::currentWorld->sceneObjects->isLastSelectionAMill();
    bool ssel=false;
    if (prox)
        ssel=(App::currentWorld->sceneObjects->getProxSensorCountInSelection()>1);
    if (mill)
        ssel=(App::currentWorld->sceneObjects->getMillCountInSelection()>1);
    CConvexVolume* cv=nullptr;
    CProxSensor* proxIt=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
    CMill* millIt=App::currentWorld->sceneObjects->getLastSelectionMill();
    if (prox)
        cv=proxIt->convexVolume;
    if (mill)
        cv=millIt->convexVolume;

    ui->qqType->clear();
    ui->qqType->setEnabled((prox||mill)&&noEditModeNoSim);
    ui->qqApplyAll->setEnabled(ssel&&noEditModeNoSim);


    if (prox||mill)
    {
        // The combo first:
        if (prox)
        {
            ui->qqType->addItem(IDSN_RAY,QVariant(0));
            ui->qqType->addItem(IDSN_RANDOMIZED_RAY,QVariant(1));
        }
        ui->qqType->addItem(IDSN_PYRAMID,QVariant(2));
        ui->qqType->addItem(IDSN_CYLINDER,QVariant(3));
        ui->qqType->addItem(IDSN_DISC,QVariant(4));
        ui->qqType->addItem(IDSN_CONE,QVariant(5));
        // Select the current item:
        if (prox)
        {
            if (proxIt->getSensorType()==sim_proximitysensor_ray_subtype)
            {
                if (proxIt->getRandomizedDetection())
                    ui->qqType->setCurrentIndex(1);
                else
                    ui->qqType->setCurrentIndex(0);
            }
            if (proxIt->getSensorType()==sim_proximitysensor_pyramid_subtype)
                ui->qqType->setCurrentIndex(2);
            if (proxIt->getSensorType()==sim_proximitysensor_cylinder_subtype)
                ui->qqType->setCurrentIndex(3);
            if (proxIt->getSensorType()==sim_proximitysensor_disc_subtype)
                ui->qqType->setCurrentIndex(4);
            if (proxIt->getSensorType()==sim_proximitysensor_cone_subtype)
                ui->qqType->setCurrentIndex(5);
        }
        if (mill)
        {
            if (millIt->getMillType()==sim_mill_pyramid_subtype)
                ui->qqType->setCurrentIndex(0);
            if (millIt->getMillType()==sim_mill_cylinder_subtype)
                ui->qqType->setCurrentIndex(1);
            if (millIt->getMillType()==sim_mill_disc_subtype)
                ui->qqType->setCurrentIndex(2);
            if (millIt->getMillType()==sim_mill_cone_subtype)
                ui->qqType->setCurrentIndex(3);
        }

        if (prox)
        {
            setWindowTitle(IDSN_DETECTION_VOLUME_PROPERTIES);

            ui->qqOffset->setEnabled(noEditModeNoSim);
            if ( (proxIt->getSensorType()==sim_proximitysensor_ray_subtype)&&proxIt->getRandomizedDetection() )
                ui->qqOffset->setText(tt::getFString(true,cv->getRadius(),4).c_str()); // Special
            else
                ui->qqOffset->setText(tt::getFString(true,cv->getOffset(),4).c_str());
            ui->qqRange->setEnabled(noEditModeNoSim);
            ui->qqRange->setText(tt::getFString(false,cv->getRange(),4).c_str());
            if (proxIt->getSensorType()==sim_proximitysensor_pyramid_subtype)
            {
                ui->qqSizeX->setEnabled(noEditModeNoSim);
                ui->qqSizeX->setText(tt::getFString(false,cv->getXSize(),4).c_str());
                ui->qqSizeY->setEnabled(noEditModeNoSim);
                ui->qqSizeY->setText(tt::getFString(false,cv->getYSize(),4).c_str());
                ui->qqSizeFarX->setEnabled(noEditModeNoSim);
                ui->qqSizeFarX->setText(tt::getFString(false,cv->getXSizeFar(),4).c_str());
                ui->qqSizeFarY->setEnabled(noEditModeNoSim);
                ui->qqSizeFarY->setText(tt::getFString(false,cv->getYSizeFar(),4).c_str());

                ui->qqRadius->setEnabled(false);
                ui->qqRadius->setText("");
                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");

                ui->qqAngle->setEnabled(false);
                ui->qqAngle->setText("");

                ui->qqFaceCount->setEnabled(false);
                ui->qqFaceCount->setText("");
                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (proxIt->getSensorType()==sim_proximitysensor_cylinder_subtype)
            {
                ui->qqRadius->setEnabled(noEditModeNoSim);
                ui->qqRadius->setText(tt::getFString(false,cv->getRadius(),4).c_str());
                ui->qqRadiusFar->setEnabled(noEditModeNoSim);
                ui->qqRadiusFar->setText(tt::getFString(false,cv->getRadiusFar(),4).c_str());
                ui->qqFaceCount->setEnabled(noEditModeNoSim);
                ui->qqFaceCount->setText(tt::getIString(false,cv->getFaceNumber()).c_str());

                ui->qqAngle->setEnabled(false);
                ui->qqAngle->setText("");

                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeY->setEnabled(false);
                ui->qqSizeY->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (proxIt->getSensorType()==sim_proximitysensor_ray_subtype)
            {
                ui->qqAngle->setEnabled(proxIt->getRandomizedDetection()&&noEditModeNoSim);
                if (proxIt->getRandomizedDetection())
                    ui->qqAngle->setText(tt::getAngleFString(false,cv->getAngle(),2).c_str());
                else
                    ui->qqAngle->setText("");

                ui->qqRadius->setEnabled(false);
                ui->qqRadius->setText("");
                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");

                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeY->setEnabled(false);
                ui->qqSizeY->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCount->setEnabled(false);
                ui->qqFaceCount->setText("");
                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (proxIt->getSensorType()==sim_proximitysensor_disc_subtype)
            {
                ui->qqSizeY->setEnabled(noEditModeNoSim);
                ui->qqSizeY->setText(tt::getFString(false,cv->getYSize(),4).c_str());
                ui->qqRadius->setEnabled(noEditModeNoSim);
                ui->qqRadius->setText(tt::getFString(false,cv->getRadius(),4).c_str());
                ui->qqFaceCount->setEnabled(noEditModeNoSim);
                ui->qqFaceCount->setText(tt::getIString(false,cv->getFaceNumber()).c_str());
                ui->qqFaceCountFar->setEnabled(noEditModeNoSim);
                ui->qqFaceCountFar->setText(tt::getIString(false,cv->getFaceNumberFar()).c_str());
                ui->qqAngle->setEnabled(noEditModeNoSim);
                ui->qqAngle->setText(tt::getAngleFString(false,cv->getAngle(),2).c_str());
                ui->qqInsideGap->setEnabled(noEditModeNoSim);
                ui->qqInsideGap->setText(tt::getFString(false,cv->getInsideAngleThing(),3).c_str());

                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");

                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");
            }
            if (proxIt->getSensorType()==sim_proximitysensor_cone_subtype)
            {
                ui->qqRadius->setEnabled(noEditModeNoSim);
                ui->qqRadius->setText(tt::getFString(false,cv->getRadius(),4).c_str());
                ui->qqFaceCount->setEnabled(noEditModeNoSim);
                ui->qqFaceCount->setText(tt::getIString(false,cv->getFaceNumber()).c_str());
                ui->qqAngle->setEnabled(noEditModeNoSim);
                ui->qqAngle->setText(tt::getAngleFString(false,cv->getAngle(),2).c_str());
                ui->qqInsideGap->setEnabled(noEditModeNoSim);
                ui->qqInsideGap->setText(tt::getFString(false,cv->getInsideAngleThing(),3).c_str());
                ui->qqSubdivisions->setEnabled(noEditModeNoSim);
                ui->qqSubdivisions->setText(tt::getIString(false,cv->getSubdivisions()).c_str());
                ui->qqSubdivisionsFar->setEnabled(noEditModeNoSim);
                ui->qqSubdivisionsFar->setText(tt::getIString(false,cv->getSubdivisionsFar()).c_str());

                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");

                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeY->setEnabled(false);
                ui->qqSizeY->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");
            }
        }

        if (mill)
        {
            setWindowTitle(IDSN_CUTTING_VOLUME_PROPERTIES);
            ui->qqInsideGap->setEnabled(false);
            ui->qqInsideGap->setText("");
            ui->qqSubdivisions->setEnabled(false);
            ui->qqSubdivisions->setText("");

            ui->qqOffset->setEnabled(noEditModeNoSim);
            ui->qqOffset->setText(tt::getFString(true,cv->getOffset(),4).c_str());
            ui->qqRange->setEnabled(noEditModeNoSim);
            ui->qqRange->setText(tt::getFString(false,cv->getRange(),4).c_str());
            if (millIt->getMillType()==sim_mill_pyramid_subtype)
            {
                ui->qqSizeX->setEnabled(noEditModeNoSim);
                ui->qqSizeX->setText(tt::getFString(false,cv->getXSize(),4).c_str());
                ui->qqSizeY->setEnabled(noEditModeNoSim);
                ui->qqSizeY->setText(tt::getFString(false,cv->getYSize(),4).c_str());
                ui->qqSizeFarX->setEnabled(noEditModeNoSim);
                ui->qqSizeFarX->setText(tt::getFString(false,cv->getXSizeFar(),4).c_str());
                ui->qqSizeFarY->setEnabled(noEditModeNoSim);
                ui->qqSizeFarY->setText(tt::getFString(false,cv->getYSizeFar(),4).c_str());

                ui->qqRadius->setEnabled(false);
                ui->qqRadius->setText("");
                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");

                ui->qqAngle->setEnabled(false);
                ui->qqAngle->setText("");

                ui->qqFaceCount->setEnabled(false);
                ui->qqFaceCount->setText("");
                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (millIt->getMillType()==sim_mill_cylinder_subtype)
            {
                ui->qqRadius->setEnabled(noEditModeNoSim);
                ui->qqRadius->setText(tt::getFString(false,cv->getRadius(),4).c_str());
                ui->qqRadiusFar->setEnabled(noEditModeNoSim);
                ui->qqRadiusFar->setText(tt::getFString(false,cv->getRadiusFar(),4).c_str());
                ui->qqFaceCount->setEnabled(noEditModeNoSim);
                ui->qqFaceCount->setText(tt::getIString(false,cv->getFaceNumber()).c_str());


                ui->qqAngle->setEnabled(false);
                ui->qqAngle->setText("");

                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeY->setEnabled(false);
                ui->qqSizeY->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (millIt->getMillType()==sim_mill_disc_subtype)
            {
                ui->qqSizeY->setEnabled(noEditModeNoSim);
                ui->qqSizeY->setText(tt::getFString(false,cv->getYSize(),4).c_str());
                ui->qqFaceCountFar->setEnabled(noEditModeNoSim);
                ui->qqFaceCountFar->setText(tt::getIString(false,cv->getFaceNumberFar()).c_str());
                ui->qqAngle->setEnabled(noEditModeNoSim);
                ui->qqAngle->setText(tt::getAngleFString(false,cv->getAngle(),2).c_str());

                ui->qqRadius->setEnabled(false);
                ui->qqRadius->setText("");
                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");


                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCount->setEnabled(false);
                ui->qqFaceCount->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");
                ui->qqSubdivisionsFar->setEnabled(false);
                ui->qqSubdivisionsFar->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
            if (millIt->getMillType()==sim_mill_cone_subtype)
            {
                ui->qqFaceCount->setEnabled(noEditModeNoSim);
                ui->qqFaceCount->setText(tt::getIString(false,cv->getFaceNumber()).c_str());
                ui->qqAngle->setEnabled(noEditModeNoSim);
                ui->qqAngle->setText(tt::getAngleFString(false,cv->getAngle(),2).c_str());
                ui->qqSubdivisionsFar->setEnabled(noEditModeNoSim);
                ui->qqSubdivisionsFar->setText(tt::getIString(false,cv->getSubdivisionsFar()).c_str());

                ui->qqRadius->setEnabled(false);
                ui->qqRadius->setText("");
                ui->qqRadiusFar->setEnabled(false);
                ui->qqRadiusFar->setText("");


                ui->qqSizeX->setEnabled(false);
                ui->qqSizeX->setText("");
                ui->qqSizeY->setEnabled(false);
                ui->qqSizeY->setText("");
                ui->qqSizeFarX->setEnabled(false);
                ui->qqSizeFarX->setText("");
                ui->qqSizeFarY->setEnabled(false);
                ui->qqSizeFarY->setText("");

                ui->qqFaceCountFar->setEnabled(false);
                ui->qqFaceCountFar->setText("");

                ui->qqSubdivisions->setEnabled(false);
                ui->qqSubdivisions->setText("");

                ui->qqInsideGap->setEnabled(false);
                ui->qqInsideGap->setText("");
            }
        }
    }
    else
    {
        setWindowTitle(IDSN_VOLUME_PROPERTIES);
        ui->qqOffset->setEnabled(false);
        ui->qqOffset->setText("");
        ui->qqRange->setEnabled(false);
        ui->qqRange->setText("");

        ui->qqRadius->setEnabled(false);
        ui->qqRadius->setText("");
        ui->qqRadiusFar->setEnabled(false);
        ui->qqRadiusFar->setText("");

        ui->qqAngle->setEnabled(false);
        ui->qqAngle->setText("");

        ui->qqSizeX->setEnabled(false);
        ui->qqSizeX->setText("");
        ui->qqSizeY->setEnabled(false);
        ui->qqSizeY->setText("");
        ui->qqSizeFarX->setEnabled(false);
        ui->qqSizeFarX->setText("");
        ui->qqSizeFarY->setEnabled(false);
        ui->qqSizeFarY->setText("");

        ui->qqFaceCount->setEnabled(false);
        ui->qqFaceCount->setText("");
        ui->qqFaceCountFar->setEnabled(false);
        ui->qqFaceCountFar->setText("");

        ui->qqSubdivisions->setEnabled(false);
        ui->qqSubdivisions->setText("");
        ui->qqSubdivisionsFar->setEnabled(false);
        ui->qqSubdivisionsFar->setText("");

        ui->qqInsideGap->setEnabled(false);
        ui->qqInsideGap->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

CConvexVolume* CQDlgDetectionVolume::getCurrentConvexVolume()
{
    CConvexVolume* cv=nullptr;
    CProxSensor* proxIt=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
    CMill* millIt=App::currentWorld->sceneObjects->getLastSelectionMill();
    if (proxIt!=nullptr)
        cv=proxIt->convexVolume;
    if (millIt!=nullptr)
        cv=millIt->convexVolume;
    return(cv);
}

void CQDlgDetectionVolume::on_qqType_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            CConvexVolume* cv=getCurrentConvexVolume();
            if (cv!=nullptr)
            {
                int index=ui->qqType->currentIndex();
                CProxSensor* proxIt=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
                CMill* millIt=App::currentWorld->sceneObjects->getLastSelectionMill();
                int theType=-1;
                bool randomized=false;
                if (proxIt!=nullptr)
                {
                    if ((index==0)||(index==1))
                    {
                        theType=sim_proximitysensor_ray_subtype;
                        randomized=(index==1);
                    }
                    if (index==2)
                        theType=sim_proximitysensor_pyramid_subtype;
                    if (index==3)
                        theType=sim_proximitysensor_cylinder_subtype;
                    if (index==4)
                        theType=sim_proximitysensor_disc_subtype;
                    if (index==5)
                        theType=sim_proximitysensor_cone_subtype;
                }
                if (millIt)
                {
                    if (index==0)
                        theType=sim_mill_pyramid_subtype;
                    if (index==1)
                        theType=sim_mill_cylinder_subtype;
                    if (index==2)
                        theType=sim_mill_disc_subtype;
                    if (index==3)
                        theType=sim_mill_cone_subtype;
                }
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_TYPE_DETECTIONVOLUMEGUITRIGGEREDCMD;
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
                cmd.intParams.push_back(theType);
                cmd.boolParams.push_back(randomized);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDetectionVolume::on_qqOffset_editingFinished()
{
    if (!ui->qqOffset->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* proxSensor=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        CMill* mill=App::currentWorld->sceneObjects->getLastSelectionMill();
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqOffset->text().toFloat(&ok);
        if (ok&&cv&&(proxSensor!=nullptr))
        {
            // special here:
            if ( (proxSensor->getSensorType()==sim_proximitysensor_ray_subtype)&&proxSensor->getRandomizedDetection() )
                App::appendSimulationThreadCommand(SET_RADIUS_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            else
                App::appendSimulationThreadCommand(SET_OFFSET_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        if (ok&&cv&&(mill!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_OFFSET_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqRadius_editingFinished()
{
    if (!ui->qqRadius->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqRadius->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_RADIUS_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqRange_editingFinished()
{
    if (!ui->qqRange->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqRange->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_RANGE_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqRadiusFar_editingFinished()
{
    if (!ui->qqRadiusFar->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqRadiusFar->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_RADIUSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSizeX_editingFinished()
{
    if (!ui->qqSizeX->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqSizeX->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_XSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqAngle_editingFinished()
{
    if (!ui->qqAngle->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqAngle->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_ANGLE_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal*gv::userToRad);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSizeY_editingFinished()
{
    if (!ui->qqSizeY->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqSizeY->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_YSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqFaceCount_editingFinished()
{
    if (!ui->qqFaceCount->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        int newVal=ui->qqFaceCount->text().toInt(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_FACECOUNT_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSizeFarX_editingFinished()
{
    if (!ui->qqSizeFarX->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqSizeFarX->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_XSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqFaceCountFar_editingFinished()
{
    if (!ui->qqFaceCountFar->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        int newVal=ui->qqFaceCountFar->text().toInt(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_FACECOUNTFAR_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSizeFarY_editingFinished()
{
    if (!ui->qqSizeFarY->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqSizeFarY->text().toFloat(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_YSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSubdivisions_editingFinished()
{
    if (!ui->qqSubdivisions->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* proxSensor=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        int newVal=ui->qqSubdivisions->text().toInt(&ok);
        if (ok&&cv&&(proxSensor!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_SUBDIVISIONS_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqInsideGap_editingFinished()
{
    if (!ui->qqInsideGap->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* proxSensor=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        float newVal=ui->qqInsideGap->text().toFloat(&ok);
        if (ok&&cv&&(proxSensor!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_INSIDEGAP_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqSubdivisionsFar_editingFinished()
{
    if (!ui->qqSubdivisionsFar->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CConvexVolume* cv=getCurrentConvexVolume();
        bool ok;
        int newVal=ui->qqSubdivisionsFar->text().toInt(&ok);
        if (ok&&cv)
        {
            App::appendSimulationThreadCommand(SET_SUBDIVISIONSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDetectionVolume::on_qqApplyAll_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* proxSensor=App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        CMill* mill=App::currentWorld->sceneObjects->getLastSelectionMill();
        if ( (proxSensor!=nullptr)||(mill!=nullptr) )
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_DETECTIONVOLUMEGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
