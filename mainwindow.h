#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDockWidget>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QColorDialog>
#include <QToolBar>
#include <QSpinBox>
#include <QCheckBox>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 自定义图片标签类，用于处理拖放事件
class DragDropImageLabel : public QLabel
{
    Q_OBJECT
    friend class MainWindow;

public:
    void setVisibleLayers(const QSet<int>& layers);

private:
    QSet<int> visibleLayers;  // Set of currently visible layers
public:
    void setShapeLayer(int layer);
    int getSelectedShapeLayer() const;
    explicit DragDropImageLabel(QWidget *parent = nullptr);
    void setOriginalPixmap(const QPixmap &pixmap);
    QPixmap getPixmap() const { return originalPixmap; }
    // Add this declaration to DragDropImageLabel class in mainwindow.h
public:
    QColor getSelectedShapeColor() const;
    int getSelectedShapeBorderWidth() const;
    bool hasSelectedShape() const { return selectedShapeIndex >= 0; }

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
    // Add this declaration to DragDropImageLabel class in `mainwindow.h`
public:

    void setShapePosition(int x, int y);
    qreal getSelectedShapeSize() const;
    void setShapeSize(qreal size);

    void deleteSelectedShape();
    void setShapeColor(const QColor &color);
    void setBorderWidth(int width);
    // Add to DragDropImageLabel class in mainwindow.h:
    signals:
        void selectionChanged();
private:
    struct Shape {
        QString type;
        int x;              // X coordinate (0-1920)
        int y;              // Y coordinate (0-1080)
        qreal sizePercent;  // Size as a percentage of the image's minimum dimension
        QColor color;       // Shape color
        int borderWidth;    // Shape border width
        int layer;          // Layer from 0-9 (0 is bottom, 9 is top)
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


private:
    QDockWidget *layersDock;
    QList<QCheckBox*> layerCheckBoxes;
    QSet<int> visibleLayers;  // Set of visible layer indices

    private slots:
        void onLayerVisibilityChanged(int);
    // Add to MainWindow class in mainwindow.h
private:

    QSpinBox *layerSpinBox;

    private slots:
        void changeShapeLayer(int layer);
private slots:
    void on_actionOpen_triggered();
    void createShapeToolbar();
    void deleteSelectedShape();
    void changeShapeColor();
    void changeBorderWidth(int width);
    void changeShapeSize(double size);

    void changeShapePosition(int);
private:

    QSpinBox *xPosSpinBox;  // Changed to QSpinBox
    QSpinBox *yPosSpinBox;  // Changed to QSpinBox
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QDockWidget *shapesDock;
    QListWidget *shapesListWidget;
    QPixmap createShapeIcon(const QString &shape);

    void createLayerPanel();

    // Add this to MainWindow class in mainwindow.h
private:
    QDoubleSpinBox *sizeSpinBox;
    QSpinBox *widthSpinBox;
    void updatePropertyControls();
};
#endif // MAINWINDOW_H
