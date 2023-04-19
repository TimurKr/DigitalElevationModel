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
			ui->projection_type->currentIndex() == 0 ? 0 : ui->center_of_projection->value());
	}

	// Image functions
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	//// Tools slots ////

	// Display
	void on_global_color_clicked();
	void on_alg_type_currentIndexChanged(int index)
	{
		vW->setRasterizationAlgorithm(index == 0 ? ViewerWidget::DDA : ViewerWidget::BRESENHAMM);
	}
	void on_coloring_type_currentIndexChanged(int index)
	{
		vW->setColoringType(
			index == 0 ? ViewerWidget::WIREFRAME : index == 1 ? ViewerWidget::VERTEX
															  : ViewerWidget::SIDE);
	}

	// Camera slots
	void on_camera_x_valueChanged(double arg1)
	{
		setCamera();
	}
	void on_camera_y_valueChanged(double arg1) { setCamera(); }
	void on_camera_z_valueChanged(double arg1) { setCamera(); }
	void on_camera_top_clicked() { vW->setCameraRotation(M_PI / 2, 0); }
	void on_camera_front_clicked() { vW->setCameraRotation(0, 0); }
	void on_camera_right_clicked() { vW->setCameraRotation(0, 3 * M_PI / 2); }

	// Projection slots
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

	// Point Light Source slots
	void on_light_color_clicked();
	void on_light_x_valueChanged(double x) { vW->setLightPositionX(x); }
	void on_light_y_valueChanged(double y) { vW->setLightPositionY(y); }
	void on_light_z_valueChanged(double z) { vW->setLightPositionZ(z); }
	void on_light_intensity_sliderMoved(int arg1) { vW->setLightIntensity(arg1); }

	// Phong Light Model slots
	void on_ambient_color_clicked();
	void on_ambient_red_valueChanged(int value)
	{
		vW->getLightModel().ambient.setX(value / 255.);
		vW->redraw();
	}
	void on_ambient_green_valueChanged(int value)
	{
		vW->getLightModel().ambient.setY(value / 255.);
		vW->redraw();
	}
	void on_ambient_blue_valueChanged(int value)
	{
		vW->getLightModel().ambient.setZ(value / 255.);
		vW->redraw();
	}
	void on_diffuse_red_valueChanged(int value)
	{
		vW->getLightModel().diffuse.setX(value / 255.);
		vW->redraw();
	}
	void on_diffuse_green_valueChanged(int value)
	{
		vW->getLightModel().diffuse.setY(value / 255.);
		vW->redraw();
	}
	void on_diffuse_blue_valueChanged(int value)
	{
		vW->getLightModel().diffuse.setZ(value / 255.);
		vW->redraw();
	}
	void on_mirror_red_valueChanged(int value)
	{
		vW->getLightModel().specular.setX(value / 255.);
		vW->redraw();
	}
	void on_mirror_green_valueChanged(int value)
	{
		vW->getLightModel().specular.setY(value / 255.);
		vW->redraw();
	}
	void on_mirror_blue_valueChanged(int value)
	{
		vW->getLightModel().specular.setZ(value / 255.);
		vW->redraw();
	}
	void on_mirror_sharpness_valueChanged(double value)
	{
		vW->getLightModel().specular_sharpness = value;
		vW->redraw();
	}
};
