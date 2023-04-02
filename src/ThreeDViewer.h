#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ThreeDViewer.h"
#include "ViewerWidget.h"

class ThreeDViewer : public QMainWindow
{
	Q_OBJECT

public:
	ThreeDViewer(QWidget *parent = Q_NULLPTR);

private:
	Ui::ThreeDViewerClass *ui;
	ViewerWidget *vW;

	QSettings settings;
	QMessageBox msgBox;

	// Event filters
	bool eventFilter(QObject *obj, QEvent *event);

	// ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject *obj, QEvent *event);
	void ViewerWidgetMouseButtonPress(ViewerWidget *w, QEvent *event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget *w, QEvent *event);
	void ViewerWidgetMouseMove(ViewerWidget *w, QEvent *event);
	void ViewerWidgetLeave(ViewerWidget *w, QEvent *event);
	void ViewerWidgetEnter(ViewerWidget *w, QEvent *event);
	void ViewerWidgetWheel(ViewerWidget *w, QEvent *event);

	// ThreeDViewer Events
	void closeEvent(QCloseEvent *event);

	// 3D Object functions
	int loadObject(QString filename);
	void drawObject() { vW->drawObject(); }
	void setCamera()
	{
		vW->setCamera(
			QVector3D(ui->camera_x->value(), ui->camera_y->value(), ui->camera_z->value()),
			ui->camera_zenit->value(), ui->camera_azimuth->value(),
			ui->projection_type->currentIndex() == 0 ? 0 : ui->center_of_projection->value());
	}

	// Image functions
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	// Tools slots
	void on_pushButtonSetColor_clicked();
	void on_clear_button_clicked()
	{
		vW->clear();
		vW->delete_objects();
	}

	// Camera slots
	void on_camera_x_valueChanged(double arg1) { setCamera(); }
	void on_camera_y_valueChanged(double arg1) { setCamera(); }
	void on_camera_z_valueChanged(double arg1) { setCamera(); }
	void on_camera_zenit_sliderMoved(int position) { setCamera(); }
	void on_camera_azimuth_sliderMoved(int position) { setCamera(); }
	void on_projection_type_currentIndexChanged(int index)
	{
		setCamera();
		if (index == 1)
		{
			ui->center_of_projection->setEnabled(true);
		}
		else
		{
			ui->center_of_projection->setEnabled(false);
		}
	}
	void on_center_of_projection_valueChanged(double arg1) { setCamera(); }
};
