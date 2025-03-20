#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDockWidget>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 自定义图片标签类，用于处理拖放事件
class DragDropImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit DragDropImageLabel(QWidget *parent = nullptr);
    void setOriginalPixmap(const QPixmap &pixmap);
    QPixmap getPixmap() const { return originalPixmap; }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *event) override;

private:
    int selectedShapeIndex = -1; // -1 means no shape selected
    bool isDraggingShape = false;
    QPoint dragStartPos;
    QColor circleColor = Qt::red;
    QColor rectangleColor = Qt::blue;
    int borderWidth = 2;

public:
    void deleteSelectedShape();
    void setShapeColor(const QColor &color);
    void setBorderWidth(int width);

private:
    struct Shape {
        QString type;
        qreal xPercent;   // 相对于图片宽度的百分比位置 (0.0-1.0)
        qreal yPercent;   // 相对于图片高度的百分比位置 (0.0-1.0)
        qreal sizePercent; // 相对于图片最小边的百分比大小 (0.0-1.0)
    };
    QList<Shape> shapes; // 存储形状及其相对位置
    QPixmap originalPixmap;
    // QList<QPair<QString, QRect>> shapes; // 存储形状及其位置
};

// 自定义列表项，用于表示可拖动的图形
class ShapeListItem : public QListWidgetItem
{
public:
    ShapeListItem(const QString &text, QListWidget *parent = nullptr);
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void createShapeToolbar();
    void deleteSelectedShape();
    void changeShapeColor();
    void changeBorderWidth(int width);

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QDockWidget *shapesDock;
    QListWidget *shapesListWidget;
    QPixmap createShapeIcon(const QString &shape);
};
#endif // MAINWINDOW_H
