#include "ThreeDViewer.h"

ThreeDViewer ::ThreeDViewer(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::ThreeDViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	QColor default_color = Qt::blue;
	QString style_sheet = QString("background-color: #%1;").arg(default_color.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ThreeDViewer ::eventFilter(QObject *obj, QEvent *event)
{
	if (obj->objectName() == "ViewerWidget")
	{
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

// ViewerWidget Events
bool ThreeDViewer ::ViewerWidgetEventFilter(QObject *obj, QEvent *event)
{
	ViewerWidget *w = static_cast<ViewerWidget *>(obj);

	if (!w)
	{
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress)
	{
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove)
	{
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave)
	{
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter)
	{
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel)
	{
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ThreeDViewer ::ViewerWidgetMouseButtonPress(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (e->button() == Qt::LeftButton)
	{
		vW->startTranslation(e->pos());
	}
}
void ThreeDViewer ::ViewerWidgetMouseButtonRelease(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (e->button() == Qt::LeftButton)
	{
		vW->endTranslation();
	}
}
void ThreeDViewer ::ViewerWidgetMouseMove(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (vW->getIsTranslating())
		vW->translateObject(e->pos());
}
void ThreeDViewer ::ViewerWidgetLeave(ViewerWidget *w, QEvent *event)
{
}
void ThreeDViewer ::ViewerWidgetEnter(ViewerWidget *w, QEvent *event)
{
}
void ThreeDViewer ::ViewerWidgetWheel(ViewerWidget *w, QEvent *event)
{
	QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
	if (wheelEvent->angleDelta().y() > 0)
	{
		// vW->scaleObjects(1.1, 1.1);
	}
	else
	{
		// vW->scaleObjects(0.9, 0.9);
	}
}

// ThreeDViewer Events
void ThreeDViewer ::closeEvent(QCloseEvent *event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

// Image functions
bool ThreeDViewer ::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull())
	{
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ThreeDViewer ::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage *img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

// Slots
void ThreeDViewer ::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty())
	{
		return;
	}

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName))
	{
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ThreeDViewer ::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty())
	{
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName))
		{
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else
		{
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ThreeDViewer ::on_actionClear_triggered()
{
	vW->clear();
}
void ThreeDViewer ::on_actionExit_triggered()
{
	this->close();
}

void ThreeDViewer ::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->getGlobalColor(), this);
	if (newColor.isValid())
	{
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		vW->setGlobalColor(newColor);
	}
}
