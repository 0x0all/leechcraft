#ifndef SOURCEVIEWER_H
#define SOURCEVIEWER_H
#include <QMainWindow>
#include "ui_sourceviewer.h"

class SourceViewer : public QMainWindow
{
	Q_OBJECT
	
	Ui::SourceViewer Ui_;
public:
	SourceViewer (QWidget* = 0);
	void SetHtml (const QString&);
};

#endif

