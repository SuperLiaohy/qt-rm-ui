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
#include <QDrag>
// DragDropImageLabel 实现
DragDropImageLabel::DragDropImageLabel(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true); // 允许接收拖放
}


void DragDropImageLabel::setOriginalPixmap(const QPixmap &pixmap)
{
    // 保存原始图片
    originalPixmap = pixmap;

    // 更新显示（实际缩放在paintEvent中进行）
    update();
}

void DragDropImageLabel::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查拖动的数据是否包含我们需要的格式
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}


void DragDropImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(240, 240, 240)); // 背景色

    if (originalPixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    // 计算适合窗口的缩放尺寸，保持宽高比
    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);

    // 计算图片居中位置
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    // 绘制缩放后的图片
    painter.drawPixmap(x, y, scaledSize.width(), scaledSize.height(), originalPixmap);

    // 计算缩放比例
    double scaleX = (double)scaledSize.width() / originalPixmap.width();
    double scaleY = (double)scaledSize.height() / originalPixmap.height();

    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制所有形状
    for (const auto &shape : shapes) {
        // 计算形状在缩放后图片上的位置
        int imgX = x + shape.xPercent * originalPixmap.width() * scaleX;
        int imgY = y + shape.yPercent * originalPixmap.height() * scaleY;

        // 计算形状大小（也要缩放）
        int minDimension = qMin(originalPixmap.width(), originalPixmap.height());
        int shapeSize = shape.sizePercent * minDimension * scaleX;

        QRect shapeRect(imgX - shapeSize/2, imgY - shapeSize/2, shapeSize, shapeSize);

        if (shape.type == "圆形") {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawEllipse(shapeRect);
        } else if (shape.type == "矩形") {
            painter.setPen(QPen(Qt::blue, 2));
            painter.drawRect(shapeRect);
        }
    }
}

void DragDropImageLabel::dropEvent(QDropEvent *event)
{
    if (originalPixmap.isNull())
        return;

    // 获取拖放的形状类型
    QString shapeType = event->mimeData()->text();

    // 计算适合窗口的缩放尺寸
    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);

    // 计算图片在窗口中的位置
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    // 检查鼠标是否在图片区域内
    QRect imgRect(x, y, scaledSize.width(), scaledSize.height());
    if (!imgRect.contains(event->pos())) {
        return; // 如果不在图片上，忽略此次拖放
    }

    // 计算在原始图片坐标系中的相对位置
    double scaleX = (double)originalPixmap.width() / scaledSize.width();
    double scaleY = (double)originalPixmap.height() / scaledSize.height();

    double relX = (event->pos().x() - x) * scaleX;
    double relY = (event->pos().y() - y) * scaleY;

    // 添加形状
    Shape shape;
    shape.type = shapeType;
    shape.xPercent = relX / originalPixmap.width();
    shape.yPercent = relY / originalPixmap.height();
    shape.sizePercent = 0.05; // 使用固定大小比例
    shapes.append(shape);

    update();
    event->acceptProposedAction();
}

class ShapeListWidget : public QListWidget
{
public:
    ShapeListWidget(QWidget *parent = nullptr) : QListWidget(parent) {}

protected:
    void startDrag(Qt::DropActions supportedActions) override
    {
        QListWidgetItem *item = currentItem();
        if (!item)
            return;

        QMimeData *mimeData = new QMimeData;
        mimeData->setText(item->text()); // 存储形状类型

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // 设置拖动时的图标
        QPixmap pixmap(40, 40);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        if (item->text() == "圆形") {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawEllipse(5, 5, 30, 30);
        } else if (item->text() == "矩形") {
            painter.setPen(QPen(Qt::blue, 2));
            painter.drawRect(5, 5, 30, 30);
        }

        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(20, 20)); // 设置热点为中心

        drag->exec(supportedActions, Qt::CopyAction);
    }
};


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

    //在标签中显示图片 - 不在这里缩放，让setOriginalPixmap处理
    dynamic_cast<DragDropImageLabel*>(imageLabel)->setOriginalPixmap(pixmap);

    //更新窗口标题以包含文件名
    setWindowTitle(tr("Qt5 Image Viewer - %1").arg(QFileInfo(fileName).fileName()));
}

void MainWindow::createShapeToolbar()
{
    // 创建停靠窗口
    shapesDock = new QDockWidget(tr("形状工具"), this);
    shapesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    shapesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // 使用自定义列表部件
    shapesListWidget = new ShapeListWidget(shapesDock);
    shapesListWidget->setDragEnabled(true);
    shapesListWidget->setViewMode(QListWidget::IconMode);
    shapesListWidget->setIconSize(QSize(40, 40));
    shapesListWidget->setSpacing(10);

    // 添加形状项
    auto *circleItem = new ShapeListItem(tr("圆形"), shapesListWidget);
    QPixmap circleIcon(40, 40);
    circleIcon.fill(Qt::transparent);
    QPainter circlePainter(&circleIcon);
    circlePainter.setRenderHint(QPainter::Antialiasing);
    circlePainter.setPen(QPen(Qt::red, 2));
    circlePainter.drawEllipse(5, 5, 30, 30);
    circleItem->setIcon(QIcon(circleIcon));

    auto *rectItem = new ShapeListItem(tr("矩形"), shapesListWidget);
    QPixmap rectIcon(40, 40);
    rectIcon.fill(Qt::transparent);
    QPainter rectPainter(&rectIcon);
    rectPainter.setPen(QPen(Qt::blue, 2));
    rectPainter.drawRect(5, 5, 30, 30);
    rectItem->setIcon(QIcon(rectIcon));

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
