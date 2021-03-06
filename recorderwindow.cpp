#include "recorderwindow.h"

RecorderWindow::RecorderWindow()
{
    //pointer set to null (container with all data)
    currentData = nullptr;

    device = new SerialPort;
    statusBar = new QStatusBar;
    statusBar->setAutoFillBackground(1);

    createLayout();

    //all signals and slots
    //signal from device
    connect(device,SIGNAL(gotSample()),this,SLOT(loadReceivedSample()));
    connect(this,SIGNAL(newSample()),this,SLOT(currentMaxMinRMSChanged()));
    //buttons signals
    connect(connectButton,SIGNAL(clicked(bool)),device,SLOT(connectWithDevice()));
    connect(loadButton,SIGNAL(clicked(bool)),this,SLOT(loadData()));
    connect(saveButton,SIGNAL(clicked(bool)),this,SLOT(saveData()));
    connect(startButton,SIGNAL(clicked(bool)),this,SLOT(startChanged()));
    //combo boxes signals
    connect(samplingComboBox,SIGNAL(activated(int)),this,SLOT(samplingChanged()));
    connect(channelsComboBox,SIGNAL(activated(int)),this,SLOT(channelChanged()));
    connect(rangeComboBox,SIGNAL(activated(int)),this,SLOT(rangeChanged()));
    //infos in status bar
    connect(device,SIGNAL(connected()),this,SLOT(connectedChanged()));
    connect(device,SIGNAL(noConnected()),this,SLOT(noConnectedChanged()));
    connect(device,SIGNAL(disconnectedCorrectly()),this,SLOT(disconnectedCorrectlyChanged()));

}


void RecorderWindow::connectedChanged()
{
    connectButton->setText("Rozłącz");
    statusBar->showMessage("Znaleziono urządzenie i połączono");
}
void RecorderWindow::noConnectedChanged()
{
    connectButton->setText("Połącz");
    statusBar->showMessage("Nie znaleziono urządzenia - podłącz urządzenie i spróbuj ponownie");
}

void RecorderWindow::disconnectedCorrectlyChanged()
{
    connectButton->setText("Połącz");
    statusBar->showMessage("Rozłączono urządzenie - kliknij połącz aby połączyć ponownie");
}

void RecorderWindow::channelChanged()
{

    QStringList channels = {"C1!","C2!","C3!"};
    device->send(channels.at(channelsComboBox->currentIndex()));
}

void RecorderWindow::rangeChanged()
{
    currentRange = rangeComboBox->currentIndex();
}

void RecorderWindow::samplingChanged()
{
    QStringList sampling = {"F0!","F1!","F2!","F3!","F4!","F5!","F6!","F7!","F8!","F9!"};
    device->send(sampling.at(samplingComboBox->currentIndex()));
    device->samplingFrequency = (samplingComboBox->currentIndex()+1)*1000;
    QString tmp;
    tmp.append(QString::number(device->samplingFrequency,10));
    tmp.append(" Hz");
    samplingFrequencyLabel->setText(tmp);

}

void RecorderWindow::timeChanged()
{
    QStringList time = {"T0!","T1!","T2!","T3!","T4!","T5!","T6!","T7!","T8!","T9!"};
    device->send(time.at(timeComboBox->currentIndex()));
}

void RecorderWindow::startChanged()
{
    device->send("S2!");
}

void RecorderWindow::currentMaxMinRMSChanged()
{
    QString tmp;
    tmp.append(QString::number((int)currentData->minAmplitude,10));
    tmp.append(" A");
    currentIminLabel->setText(tmp);

    tmp.clear();
    tmp.append(QString::number((int)currentData->maxAmplitude,10));
    tmp.append(" A");
    currentImaxLabel->setText(tmp);

    tmp.clear();
    tmp.append(QString::number((int)(currentData->maxAmplitude/sqrt(2)),10));
    tmp.append(" A");
    currentRMSLabel->setText(tmp);
}
void RecorderWindow::loadData()
{
    if(currentData!=nullptr)
    {
        delete currentData;
        currentData = nullptr;
    }

    //allocate memory for data container
     currentData = new CurrentAllData;
     myFile = new Files;
     myFile->openFile(); //choose and open file
     myFile->copyData(currentData); //copy data to object currentData class CurrentAllData

     fftObj = new myFFT; //create myFFT object
     fftObj->doFFT(currentData); //do FFT and results put in object currentData //class CurrentAllData

     drawSinChart(currentData); //draw sinus/es charts
     drawFFTChart(currentData); //draw fft charts

  delete myFile;
  delete  fftObj;


}


void RecorderWindow::drawSinChart(CurrentAllData *obj)
{
    if(series->count() != 0) series->clear();
    if(series2->count() != 0) series2->clear();

    float x = 0;
    float step = 1;

   if(obj->samplingFrequency!=0) step = 1/(float)obj->samplingFrequency;
   else qDebug()<<"dzielenie przez zero";

   if(obj->amountOfChannels==1)
   {
       for (int i = 0; i < obj->samples1CH.size(); i++)
         {
             QPointF p((qreal)(x+=step), ((qreal)(obj->samples1CH[i])));
             series->append(p);
         }
   }
   else if(obj->amountOfChannels==2)
   {
       for (int i = 0; i < obj->samples1CH.size(); i++)
         {
             QPointF p((qreal)(x+=step), ((qreal)(obj->samples1CH[i])));
             series->append(p);
             QPointF p1((qreal)(x+=step), ((qreal)(obj->samples2CH[i])));
             series2->append(p1);
         }
   }
   else{}

   series->setName("CH1");
   series2->setName("CH2");
   chart->createDefaultAxes();
   if(obj->amountOfChannels==2)chart->legend()->show();
   chart->axisY()->setRange(obj->minAmplitude,
                            obj->maxAmplitude);
   chart->axisX()->setRange(0,0.1);
   chart->axisX()->setTitleText("t [s]");
   chart->axisY()->setTitleText("i [A]");

}

void RecorderWindow::drawFFTChart(CurrentAllData *obj)
{
    if(seriesFFT->count()!=0)seriesFFT->clear();
    if(seriesFFT2->count()!=0)seriesFFT2->clear();


    if(obj->amountOfChannels==1)
    {
        for (int i = 0; i < obj->amplitudeCH1.size(); i++)
        {
            QPointF p((qreal)(obj->xAxis[i]), ((qreal)obj->amplitudeCH1[i]));
            seriesFFT->append(p);
        }
    }
    else if(obj->amountOfChannels==2)
    {
        for (int i = 0; i < obj->amplitudeCH1.size(); i++)
        {
            QPointF p((qreal)(obj->xAxis[i]), ((qreal)obj->amplitudeCH1[i]));
            seriesFFT->append(p);

            QPointF p2((qreal)(obj->xAxis[i]), ((qreal)obj->amplitudeCH2[i]));
            seriesFFT2->append(p2);
        }
    }
    else
    {}

     chartFFT->createDefaultAxes();
     chartFFT->axisY()->setRange(0,obj->maxFFT1Amplitude+0.25*obj->maxFFT1Amplitude);
     chartFFT->axisX()->setRange(0,200);
     chartFFT->axisX()->setTitleText("f [Hz]");
     chartFFT->axisY()->setTitleText("i [A]");
   if(obj->amountOfChannels==2)
      {
     chartFFT2->createDefaultAxes();
     chartFFT2->axisY()->setRange(0,obj->maxFFT2Amplitude+0.25*obj->maxFFT2Amplitude);
     chartFFT2->axisX()->setRange(0,200);
     chartFFT2->axisX()->setTitleText("f [Hz]");
     chartFFT2->axisY()->setTitleText("i [A]");
    }

 }


void RecorderWindow::saveData()
{
    if(currentData==nullptr)return;
     myFile = new Files;
     myFile->saveFile(currentData);
     delete myFile;
}

void RecorderWindow::loadReceivedSample()
{
    if(currentData!=nullptr)
    {
        delete currentData;
        currentData = nullptr;
    }

    //allocate memory for data container
     currentData = new CurrentAllData;

//-----------basic infos about samples--------------------------------

     //save amount of channels
     if(channelsComboBox->currentIndex() == 0 || channelsComboBox->currentIndex() == 1) currentData->amountOfChannels = 1;
     else if(channelsComboBox->currentIndex() == 2) currentData->amountOfChannels = 2;

     //save sampling frequency
     currentData->samplingFrequency = (samplingComboBox->currentIndex()+1)*1000;

     //save sampling period
     currentData->samplingPeriod = timeComboBox->currentIndex()+1;

     //save sampling range
     if(currentRange==0)currentData->samplingRange = 1;
     if(currentRange==1)currentData->samplingRange = 10;
     if(currentRange==2)currentData->samplingRange = 100;

//----------end of basic infos about samples---------------------------

     device->copyData(currentData);

     qDebug()<<currentData->samples1CH;
     qDebug()<<currentData->amountOfChannels;


     fftObj = new myFFT; //create myFFT object
     fftObj->doFFT(currentData); //do FFT and results put in object currentData //class CurrentAllData

     drawSinChart(currentData); //draw sinus/es charts
     drawFFTChart(currentData); //draw fft charts

     delete fftObj;
     emit newSample();

}

void RecorderWindow::createLayout()
{
    //widgets with individual pages, MainWindowTabWidget consists of that
    recordPageWidget = new QWidget;
    fftPageWidget = new QWidget;

    //widget that cover all window space, divides it on record and fft area
    mainWindowTabWidget = new QTabWidget;
    mainWindowTabWidget->addTab(recordPageWidget,"Rejestracja");
    mainWindowTabWidget->addTab(fftPageWidget,"Analiza FFT");



    //Main Grid Layout, parent->recordPageWidget, contains left menu and right chart
    mainRecordLayout = new QGridLayout;
    mainRecordLayout->setColumnMinimumWidth(0,140);

//-------------------------------------------------------------------------------------
//------------------------------ BEGIN OF LEFT MENU -----------------------------------
//-------------------------------------------------------------------------------------

    //QVBoxLayout that holds start, pomiary, wyniki QVBoxLayouts
    mainMenuBoxLayout = new QVBoxLayout;


//---------------- start of start position -----------------------------------

    //first GroupBox in left menu, contains QVBoxLayot and widgets - buttons
    startGroupBox = new QGroupBox("Start");
    startGroupBox->setContentsMargins(20,20,20,20);
    //contains all widgets, parent = startGroupBox, because i cant add directly widgets to QGroupBox
    startMenuBoxLayout = new QVBoxLayout;
    startMenuBoxLayout->setContentsMargins(0,5,0,0);

            //objects

            connectButton = new QPushButton;
            connectButton->setText("Połącz");

            loadButton = new QPushButton;
            loadButton->setText("Wczytaj");

            saveButton = new QPushButton;
            saveButton->setText("Zapisz");


    //add objects to startMenuBoxLayout //QVBoxLayout
    startMenuBoxLayout->addWidget(connectButton);
    startMenuBoxLayout->addWidget(loadButton);
    startMenuBoxLayout->addWidget(saveButton);

    //add made up layout to startGroupBox widget
     startGroupBox->setLayout(startMenuBoxLayout);

     //add startgroupBox to left menu start layout
     mainMenuBoxLayout->addWidget(startGroupBox);

//------------------ end of start position ----------------------------
//---------------------------------------------------------------------
//------------------ start of pomiary position ------------------------

     pomiarGroupBox = new QGroupBox("Pomiar");
     pomiarGroupBox->setContentsMargins(20,20,20,20);

     pomiarMenuBoxLayout = new QVBoxLayout;
     pomiarMenuBoxLayout->setContentsMargins(0,0,0,0);
     pomiarMenuBoxLayout->addSpacing(10);

     //----- start of channels position ------------------

           channelsLabel = new QLabel;
           channelsComboBox = new QComboBox;
           channelsStringList = new QStringList;

           channelsLabel->setText("Kanał");
           channelsStringList->append("CH1");
           channelsStringList->append("CH2");
           channelsStringList->append("CH1 i CH2");

           channelsComboBox->addItems(*channelsStringList);


    //----- end of channels position ----------------------

    //----- start of częstotliwość próbkowania position----

           samplingLabel = new QLabel;
           samplingComboBox = new QComboBox;
           samplingStringList = new QStringList;

           samplingLabel->setText("Częstotliwość próbkowania");
           samplingStringList->append("1 kHz");
           samplingStringList->append("2 kHz");
           samplingStringList->append("3 kHz");
           samplingStringList->append("4 kHz");
           samplingStringList->append("5 kHz");
           samplingStringList->append("6 kHz");
           samplingStringList->append("7 kHz");
           samplingStringList->append("8 kHz");
           samplingStringList->append("9 kHz");
           samplingStringList->append("10 kHz");

           samplingComboBox->addItems(*samplingStringList);


    //----- end of częstotliwość próbkowania position -----

    //----- start of czas próbkowania position ------------

           timeLabel = new QLabel;
           timeComboBox = new QComboBox;
           timeStringList = new QStringList;

           timeLabel->setText("Czas próbkowania");
           timeStringList->append("1 s");
           timeStringList->append("2 s");
           timeStringList->append("3 s");
           timeStringList->append("4 s");
           timeStringList->append("5 s");
           timeStringList->append("6 s");
           timeStringList->append("7 s");
           timeStringList->append("8 s");
           timeStringList->append("9 s");
           timeStringList->append("10 s");

           timeComboBox->addItems(*timeStringList);
           connect(timeComboBox,SIGNAL(activated(int)),this,SLOT(timeChanged()));

     //---- end of czas próbkowania position --------------

     //---- start of zakres position ----------------------

           rangeLabel = new QLabel;
           rangeComboBox = new QComboBox;
           rangeStringList = new QStringList;

           rangeLabel->setText("Zakres ");
           rangeStringList->append("1 mV/A");
           rangeStringList->append("10 mV/A");
           rangeStringList->append("100 mV/A");


           rangeComboBox->addItems(*rangeStringList);

     //---- end of zakres position -----------------------

    //---- start of tryb pomiaru -------------------------

          startButton = new QPushButton;
          startLabel = new QLabel;
          startLabel->setText("Rozpocznij pomiar ");
          startButton->setText("START");


     //------ end of tryb pomiaru --------------------------


    pomiarMenuBoxLayout->addWidget(channelsLabel);
    pomiarMenuBoxLayout->addWidget(channelsComboBox);
    pomiarMenuBoxLayout->addWidget(samplingLabel);
    pomiarMenuBoxLayout->addWidget(samplingComboBox);
    pomiarMenuBoxLayout->addWidget(timeLabel);
    pomiarMenuBoxLayout->addWidget(timeComboBox);
    pomiarMenuBoxLayout->addWidget(rangeLabel);
    pomiarMenuBoxLayout->addWidget(rangeComboBox);
    pomiarMenuBoxLayout->addWidget(startLabel);
    pomiarMenuBoxLayout->addWidget(startButton);

    pomiarGroupBox->setLayout(pomiarMenuBoxLayout);

    mainMenuBoxLayout->addWidget(pomiarGroupBox);

//----- end of pomiary position in main menu --------------
//---------------------------------------------------------
//----- start of wyniki position in main menu -------------

    resultsGroupBox = new QGroupBox("Wyniki");
    resultsGroupBox ->setContentsMargins(20,20,20,20);

    resultsMenuBoxLayout = new QVBoxLayout;
    resultsMenuBoxLayout->setContentsMargins(0,0,0,0);
    resultsMenuBoxLayout->addSpacing(10);

    //--start of SamplingFrequency--------------------------

        samplingFrequencyDescribeLabel = new QLabel;
        samplingFrequencyLabel = new QLabel;

        samplingFrequencyDescribeLabel ->setText("Częstotliwość próbkowania");

        samplingFrequencyLabel->setText(" ");
        samplingFrequencyLabel->setAutoFillBackground(1);

    //--start of Irms --------------------------

        currentRMSDescribeLabel = new QLabel;
        currentRMSLabel = new QLabel;

        currentRMSDescribeLabel ->setText("Irms");

        currentRMSLabel->setText(" ");
        currentRMSLabel->setAutoFillBackground(1);

     //-- end of Irms --------------------------

     //-- start of Imax ------------------------

        currentImaxDescribeLabel = new QLabel;
        currentImaxLabel = new QLabel;

        currentImaxDescribeLabel->setText("Imax");
        currentImaxLabel->setText(" ");
        currentImaxLabel->setAutoFillBackground(1);

     //-- end of Imax --------------------------

     //-- start of Imin ------------------------

        currentIminDescribeLabel = new QLabel;
        currentIminLabel = new QLabel;

        currentIminDescribeLabel->setText("Imin");
        currentIminLabel->setText(" ");
        currentIminLabel->setAutoFillBackground(1);

     //-- end of Imin --------------------------


    resultsMenuBoxLayout->addWidget(samplingFrequencyDescribeLabel);
    resultsMenuBoxLayout->addWidget(samplingFrequencyLabel);
    resultsMenuBoxLayout->addWidget(currentRMSDescribeLabel);
    resultsMenuBoxLayout->addWidget(currentRMSLabel);
    resultsMenuBoxLayout->addWidget(currentImaxDescribeLabel);
    resultsMenuBoxLayout->addWidget(currentImaxLabel);
    resultsMenuBoxLayout->addWidget(currentIminDescribeLabel);
    resultsMenuBoxLayout->addWidget(currentIminLabel);

    resultsGroupBox->setLayout(resultsMenuBoxLayout);

    mainMenuBoxLayout->addWidget(resultsGroupBox);

//---------------------------------------------------------------------------
//--------------------------- END OF LEFT MENU ------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//--------------------------- START OF CHART AREA ---------------------------
//---------------------------------------------------------------------------

    chart = new Chart();
    series = new QLineSeries();
    series2 = new QLineSeries;
    chart->addSeries(series);
    chart->addSeries(series2);

    chart->setTitle("PRZEBIEG PRĄDOWY i=f(t) ZAREJESTROWANEGO SYGNAŁU");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->axisY()->setRange(-500,500);
    chart->axisX()->setRange(0,0.1);
    chart->axisX()->setTitleText("t [s]");
    chart->axisY()->setTitleText("i [A]");
    chartView = new ChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

//---------------------------------------------------------------------------
//-------------------------- END OF CHART AREA ------------------------------
//---------------------------------------------------------------------------



    //---------------------------------------------------------------------------
    //--------------------------- START OF CHART AREA FFT -----------------------
    //---------------------------------------------------------------------------

    chartFFT = new Chart();
    seriesFFT = new QLineSeries();

    chartFFT2 = new Chart();
    seriesFFT2 = new QLineSeries();

    chartViewFFT = new ChartView(chartFFT);
    chartViewFFT2 = new ChartView(chartFFT2);


    mainFFTLayout = new QGridLayout;
    mainFFTLayout->setColumnMinimumWidth(0,140);

    chartFFT->addSeries(seriesFFT);
    chartFFT2->addSeries(seriesFFT2);


    chartFFT->setTitle("Rozkład FFT CH1");
    chartFFT->setAnimationOptions(QChart::SeriesAnimations);
    chartFFT->legend()->hide();
    chartFFT->createDefaultAxes();
    chartFFT->axisY()->setRange(0,100);
    chartFFT->axisX()->setRange(0,200);
    chartFFT->axisX()->setTitleText("f [Hz]");
    chartFFT->axisY()->setTitleText("i [A]");
    chartViewFFT->setRenderHint(QPainter::Antialiasing);

  //  chartFFT2->addSeries(seriesFFT2);
    chartFFT2->setTitle(" Rozkład FFT CH2");
    chartFFT2->setAnimationOptions(QChart::SeriesAnimations);
    chartFFT2->legend()->hide();
    chartFFT2->createDefaultAxes();
    chartFFT2->axisY()->setRange(0,100);
    chartFFT2->axisX()->setRange(0,200);
    chartFFT2->axisX()->setTitleText("f [Hz]");
    chartFFT2->axisY()->setTitleText("i [A]");
    chartViewFFT2->setRenderHint(QPainter::Antialiasing);


    mainFFTLayout->addWidget(chartViewFFT,0,0,1,1);
    mainFFTLayout->addWidget(chartViewFFT2,1,0,1,1);
    fftPageWidget->setLayout(mainFFTLayout);
    //---------------------------------------------------------------------------
    //-------------------------- END OF CHART AREA FFT --------------------------
    //---------------------------------------------------------------------------


// added left menu and chart to grid layout and nextly to tab widget
    mainRecordLayout->addLayout(mainMenuBoxLayout,0,0,1,1,Qt::AlignTop);
    mainRecordLayout->addWidget(chartView,0,1,1,1);
    mainRecordLayout->addWidget(statusBar,2,0,1,2);

    statusBar->showMessage("Ready");
    recordPageWidget->setLayout(mainRecordLayout);


}
