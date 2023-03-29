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

	// Image functions
	bool openImage(QString filename);
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
};
