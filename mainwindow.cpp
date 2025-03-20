#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImageReader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置窗口标题和初始大小
    setWindowTitle("Qt5 Image Viewer");
    resize(1920, 1080);

    //获取UI中的imageLabel
    imageLabel = findChild<QLabel*>("imageLabel");

    //如果UI中没有imageLabel,则创建一个
    if (!imageLabel) {
        imageLabel = new QLabel(this);
        imageLabel->setAlignment(Qt::AlignCenter);
        setCentralWidget(imageLabel);
    }

    //设置图像标签属性
    imageLabel->setMinimumSize(400, 300);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; }");
    imageLabel->setText("没有图片");

    //如果ui没有打开图片的动作，则创建一个
    if(!findChild<QAction*>("actionOpen")) {
        QMenu *fileMenu = menuBar()->addMenu("文件");
        QAction *openAction = new QAction("打开图片", this);
        openAction->setObjectName("actionOpen");
        fileMenu->addAction(openAction);
        connect(openAction, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
    } else {
        //连接UI中已有的actionOpen到对应的槽函数
        // connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图片"), QDir::homePath(), tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));
    if (fileName.isEmpty())
        return;

    //加载图片
    QImageReader reader(fileName);
    reader.setAutoTransform(true); //自动应用EXIF方向
    QImage image = reader.read();

    if (image.isNull()) {
        QMessageBox::critical(this, tr("错误"), tr("无法加载图片 %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return;
    }

    //将图片转化为QPixmap并显示
    QPixmap pixmap = QPixmap::fromImage(image);

    //缩放图片到1920*1080,并保持纵横比
    pixmap = pixmap.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //在标签中显示图片
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);

    //调整窗口大小以适应图片(可选)
    // resize(pixmap.width(), pixmap.height());

    //更新窗口标题以包含文件名
    setWindowTitle(tr("Qt5 Image Viewer - %1").arg(QFileInfo(fileName).fileName()));

}










