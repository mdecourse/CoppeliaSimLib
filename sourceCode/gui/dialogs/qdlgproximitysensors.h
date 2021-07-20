
#ifndef QDLGPROXIMITYSENSORS_H
#define QDLGPROXIMITYSENSORS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgProximitySensors;
}

class CQDlgProximitySensors : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgProximitySensors(QWidget *parent = 0);
    ~CQDlgProximitySensors();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

private slots:
    void on_qqEnableAll_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqSensorTypeCombo_currentIndexChanged(int index);

    void on_qqShowDetecting_clicked();

    void on_qqPointSize_editingFinished();

    void on_qqShowNotDetecting_clicked();

    void on_qqApplyMain_clicked();

    void on_qqAdjustVolume_clicked();

    void on_qqAdjustDetectionParams_clicked();

    void on_qqPassiveVolumeColor_clicked();

    void on_qqActiveVolumeColor_clicked();

    void on_qqRayColor_clicked();

    void on_qqMinDistColor_clicked();

    void on_qqApplyColors_clicked();

private:
    Ui::CQDlgProximitySensors *ui;
};

#endif // QDLGPROXIMITYSENSORS_H
