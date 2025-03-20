#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImageReader>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>

// DragDropImageLabel 实现
DragDropImageLabel::DragDropImageLabel(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true); // 允许接收拖放
}

void DragDropImageLabel::setOriginalPixmap(const QPixmap &pixmap)
{
    originalPixmap = pixmap;
    setPixmap(originalPixmap); // 初始显示原始图片
    update(); // 更新视图
}

void DragDropImageLabel::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查拖动的数据是否包含我们需要的格式
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void DragDropImageLabel::dropEvent(QDropEvent *event)
{
    // 获取拖放的形状类型
    QString shapeType = event->mimeData()->text();

    // 创建一个适当大小的矩形，表示形状的位置
    QRect shapeRect(event->pos().x() - 25, event->pos().y() - 25, 50, 50);

    // 添加形状到列表
    shapes.append(qMakePair(shapeType, shapeRect));

    // 更新视图
    update();

    event->acceptProposedAction();
}

void DragDropImageLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event); // 调用基类绘制图片

    if (originalPixmap.isNull())
        return;

    QPainter painter(this);

    // 绘制所有添加的形状
    for (const auto &shape : shapes) {
        if (shape.first == "圆形") {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawEllipse(shape.second);
        } else if (shape.first == "矩形") {
            painter.setPen(QPen(Qt::blue, 2));
            painter.drawRect(shape.second);
        }
    }
}

// ShapeListItem 实现
ShapeListItem::ShapeListItem(const QString &text, QListWidget *parent)
    : QListWidgetItem(text, parent)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置窗口标题和初始大小
    setWindowTitle("Qt5 Image Viewer with Shapes");
    resize(1920, 1080);

    // //获取UI中的imageLabel
    // imageLabel = findChild<QLabel*>("imageLabel");

    // //如果UI中没有imageLabel,则创建一个
    // if (!imageLabel) {
    //     imageLabel = new QLabel(this);
    //     imageLabel->setAlignment(Qt::AlignCenter);
    //     setCentralWidget(imageLabel);
    // }

    //设置图像标签属性
    imageLabel = new DragDropImageLabel(this);
    imageLabel->setMinimumSize(400, 300);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; }");
    imageLabel->setText("没有图片");
    setCentralWidget(imageLabel);

    //创建形状工具栏
    createShapeToolbar();

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

void MainWindow::createShapeToolbar()
{
    // 创建停靠窗口
    shapesDock = new QDockWidget(tr("形状工具"), this);
    shapesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    shapesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // 创建列表部件用于显示形状
    shapesListWidget = new QListWidget(shapesDock);
    shapesListWidget->setDragEnabled(true);
    shapesListWidget->setViewMode(QListWidget::IconMode);
    shapesListWidget->setIconSize(QSize(40, 40));
    shapesListWidget->setSpacing(10);
    shapesListWidget->setAcceptDrops(false);
    shapesListWidget->setDropIndicatorShown(false);

    // 添加形状项
    auto *circleItem = new ShapeListItem(tr("圆形"), shapesListWidget);
    circleItem->setIcon(QIcon(createShapeIcon("圆形")));

    auto *rectItem = new ShapeListItem(tr("矩形"), shapesListWidget);
    rectItem->setIcon(QIcon(createShapeIcon("矩形")));

    // 设置列表部件为停靠窗口的内容
    shapesDock->setWidget(shapesListWidget);

    // 将停靠窗口添加到主窗口的右侧
    addDockWidget(Qt::RightDockWidgetArea, shapesDock);
}

// 创建形状图标的辅助函数
QPixmap MainWindow::createShapeIcon(const QString &shape)
{
    QPixmap pixmap(40, 40);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (shape == "圆形") {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawEllipse(5, 5, 30, 30);
    } else if (shape == "矩形") {
        painter.setPen(QPen(Qt::blue, 2));
        painter.drawRect(5, 5, 30, 30);
    }

    return pixmap;
}
