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
	setCamera();

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
	}
}
void ThreeDViewer ::ViewerWidgetMouseButtonRelease(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);

	if (e->button() == Qt::LeftButton)
	{
	}
}
void ThreeDViewer ::ViewerWidgetMouseMove(ViewerWidget *w, QEvent *event)
{
	QMouseEvent *e = static_cast<QMouseEvent *>(event);
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

// 3D Objects
int ThreeDViewer ::loadObject(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error", "Could not open file");
		return false;
	}

	QTextStream in(&file);
	QString line = in.readLine();
	// Wait for POINTS
	while (!line.startsWith("POINTS"))
	{
		line = in.readLine();
	}

	// Handle POINTS
	line = in.readLine();
	QVector<QVector3D> points;
	while (!line.startsWith("POLYGONS"))
	{
		QStringList list = line.split(" ");
		QList<double> coordinates;

		// Try converting all strings to doubles
		bool ok = true;
		for (int i = 0; i < list.size(); i++)
		{
			coordinates.push_back(list[i].toDouble(&ok));
			if (!ok)
				return false;
		}
		points.push_back(QVector3D(coordinates[0], coordinates[1], coordinates[2]));
		line = in.readLine();
	}
	// Wait for POLYGONS
	while (!line.startsWith("POLYGONS"))
	{
		line = in.readLine();
	}

	// Handle POLYGONS
	line = in.readLine();
	QVector<QVector<unsigned int>> polygons;
	while (!line.isEmpty())
	{
		QStringList list = line.split(" ");
		QList<unsigned int> coordinates;

		// Try converting all strings to unsigned ints
		bool ok = true;
		for (int i = 0; i < list.size(); i++)
		{
			coordinates.push_back(list[i].toUInt(&ok));
			if (!ok)
				return false;
		}
		polygons.push_back(QVector<unsigned int>());
		for (int i = 1; i < coordinates.size(); i++)
		{
			polygons.back().push_back(coordinates[i]);
		}
		line = in.readLine();
	}

	vW->loadObject(points, polygons);
	return true;
}

// Image functions
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

	// QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileFilter = "VTK súbor (*.vtk)";

	QString fileName = QFileDialog::getOpenFileName(this, "Load object", folder, fileFilter);
	if (fileName.isEmpty())
	{
		return;
	}

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	int id = loadObject(fileName);
	if (id == -1)
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
