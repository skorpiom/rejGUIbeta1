#include <QApplication>

#include <QStackedLayout>
#include<QComboBox>
#include<QObject>
#include<QLayout>
#include<QWidget>
#include<QPushButton>

#include "recorderwindow.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

//    layout lay;
    RecorderWindow *tabW = new RecorderWindow;

   //    QWidget *w = new QWidget();

        // Set the grid layout as a main layout

        tabW->mainWindowTabWidget->setMinimumSize(1150,780);

        // Window title
       tabW->mainWindowTabWidget->setWindowTitle("Rejestrator przebiegów prądowych");

        // Display
        tabW->mainWindowTabWidget->show();


    return a.exec();
}
