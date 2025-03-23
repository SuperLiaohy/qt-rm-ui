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
#include <QDrag>
#include <QMimeData>
#include <QStackedWidget>

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
    // Add this to your DragDropImageLabel class:
    enum DragMode {
        DRAG_NONE,
        DRAG_SHAPE,
        DRAG_RECTANGLE_CORNER
    };

    // Replace isDraggingShape with this:
    DragMode dragMode = DRAG_NONE;
    int dragCorner = -1; // 0=top-left, 1=top-right, 2=bottom-left, 3=bottom-right
public:
    explicit DragDropImageLabel(QWidget *parent = nullptr);
    void setOriginalPixmap(const QPixmap &pixmap);
    QPixmap getPixmap() const { return originalPixmap; }

    // Basic shape management
    QSet<int> getVisibleLayers() const;
    void setVisibleLayers(const QSet<int>& layers);
    void setShapeLayer(int layer);
    int getSelectedShapeLayer() const;



    QColor getSelectedShapeColor() const;
    int getSelectedShapeBorderWidth() const;
    bool hasSelectedShape() const;
    void setShapePosition(int x, int y);
    qreal getSelectedShapeSize() const;
    void setShapeSize(qreal size);
    void deleteSelectedShape();
    void setShapeColor(const QColor &color);
    void setBorderWidth(int width);
    int getSelectedShapeX() const;
    int getSelectedShapeY() const;


    // Shape-specific properties
    int getSelectedShapeEndX() const;
    int getSelectedShapeEndY() const;
    int getSelectedShapeRadius() const;
    void setRectangleEndPoint(int x, int y);
    void setCircleRadius(int radius);
    void setLineEndPoint(int x, int y);

    // Get shape type
    QString getSelectedShapeType() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

signals:
    void selectionChanged();

private:
    struct Shape {
        QString type;
        int x;              // X coordinate (0-1920) - Center for circle, Top-left for rectangle
        int y;              // Y coordinate (0-1080) - Center for circle, Top-left for rectangle
        qreal sizePercent;  // Size as a percentage of the image's minimum dimension
        QColor color;       // Shape color
        int borderWidth;    // Shape border width
        int layer;          // Layer from 0-9 (0 is bottom, 9 is top)

        // Shape-specific properties
        union {
            struct {
                int endX;   // End X coordinate for rectangle (bottom-right)
                int endY;   // End Y coordinate for rectangle (bottom-right)
            } rect;

            struct {
                int radius; // Radius for circle
            } circle;

            struct {
                int endX;   // End X coordinate for line
                int endY;   // End Y coordinate for line
            } line;
        } specific;
    };

    QList<Shape> shapes;
    QPixmap originalPixmap;
    int selectedShapeIndex = -1;
    bool isDraggingShape = false;
    QPoint dragStartPos;
    QSet<int> visibleLayers;
};

// 自定义列表项，用于表示可拖动的图形
class ShapeListItem : public QListWidgetItem
{
public:
    ShapeListItem(const QString &text, QListWidget *parent = nullptr);
};

class ShapeListWidget : public QListWidget
{
public:
    ShapeListWidget(QWidget *parent = nullptr) : QListWidget(parent) {}

protected:
    void startDrag(Qt::DropActions supportedActions) override;

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void onLayerVisibilityChanged(int);
    void changeShapeLayer(int layer);
    void deleteSelectedShape();
    void changeShapeColor();
    void changeBorderWidth(int width);
    void changeShapePosition(int);
    void changeRectangleEndPoint(int);
    void changeCircleRadius(int);
    void updatePropertyControls();
    void onShapeSelectionChanged();
    void changeLineEndPoint(int);

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QDockWidget *shapesDock;
    QDockWidget *layersDock;
    QListWidget *shapesListWidget;

    // Shape layer controls
    QList<QCheckBox*> layerCheckBoxes;
    QSet<int> visibleLayers;
    QSpinBox *layerSpinBox;

    // Common shape properties
    QSpinBox *xPosSpinBox;
    QSpinBox *yPosSpinBox;
    // QDoubleSpinBox *sizeSpinBox;
    QSpinBox *widthSpinBox;

    // Shape-specific property controls
    QStackedWidget *shapeSpecificControls;

    // Rectangle-specific controls
    QWidget *rectanglePropertiesWidget;
    QSpinBox *rectEndXSpinBox;
    QSpinBox *rectEndYSpinBox;

    // Circle-specific controls
    QWidget *circlePropertiesWidget;
    QSpinBox *circleRadiusSpinBox;

    // Add to MainWindow class declaration in the private section
    QWidget *linePropertiesWidget;
    QSpinBox *lineEndXSpinBox;
    QSpinBox *lineEndYSpinBox;

    // Helper methods
    void createShapeToolbar();
    void createLayerPanel();
    QPixmap createShapeIcon(const QString &shape);
};

#endif // MAINWINDOW_H