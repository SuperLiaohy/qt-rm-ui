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
#include <QLineEdit>

#include <QComboBox>

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

// 自定义图片标签类，用于处理拖放事件
class DragDropImageLabel : public QLabel {
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

    void setVisibleLayers(const QSet<int> &layers);

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

    void setEllipseRadius(int radiusX, int radiusY);

    int getSelectedShapeRadiusX() const;

    int getSelectedShapeRadiusY() const;

    void setArcProperties(int radiusX, int radiusY, int startAngle, int spanAngle);

    int getSelectedShapeArcRadiusX() const;

    int getSelectedShapeArcRadiusY() const;

    int getSelectedShapeArcStartAngle() const;

    int getSelectedShapeArcSpanAngle() const;

    int32_t getSelectedShapeIntValue() const;

    int getSelectedShapeIntFontSize() const;

    void setIntValue(int32_t value);

    void setIntFontSize(int fontSize);

    int32_t getSelectedShapeFloatValue() const;

    int getSelectedShapeFloatFontSize() const;

    void setFloatValue(int32_t value);

    void setFloatFontSize(int fontSize);

    QString getSelectedShapeText() const;

    int getSelectedShapeTextLength() const;

    int getSelectedShapeTextFontSize() const;

    void setText(const QString &text);

    void setTextFontSize(int fontSize);

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
        int x; // X coordinate (0-1920) - Center for circle, Top-left for rectangle
        int y; // Y coordinate (0-1080) - Center for circle, Top-left for rectangle
        qreal sizePercent; // Size as a percentage of the image's minimum dimension
        QColor color; // Shape color
        int borderWidth; // Shape border width
        int layer; // Layer from 0-9 (0 is bottom, 9 is top)

        // Shape-specific properties
        union {
            struct {
                int endX; // End X coordinate for rectangle (bottom-right)
                int endY; // End Y coordinate for rectangle (bottom-right)
            } rect;

            struct {
                int radius; // Radius for circle
            } circle;

            struct {
                int endX; // End X coordinate for line
                int endY; // End Y coordinate for line
            } line;

            struct {
                int radiusX; // X-axis radius for ellipse
                int radiusY; // Y-axis radius for ellipse
            } ellipse;

            struct {
                int radiusX; // X-axis radius for arc
                int radiusY; // Y-axis radius for arc
                int startAngle; // Start angle in degrees (0° is 12 o'clock, clockwise)
                int spanAngle; // Span angle in degrees
            } arc;

            struct {
                int32_t value; // The int32_t value to display
                int fontSize; // Font size for displaying the value
            } intValue;

            struct {
                int32_t value; // The int32_t value to display (will be divided by 1000 for display)
                int fontSize; // Font size for displaying the value
            } floatValue;

            struct {
                char data[30]; // Character array to store text (max 30 chars)
                int length; // Actual length of text
                int fontSize; // Font size for displaying the text
            } text;
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
class ShapeListItem : public QListWidgetItem {
public:
    ShapeListItem(const QString &text, QListWidget *parent = nullptr);
};

class ShapeListWidget : public QListWidget {
public:
    ShapeListWidget(QWidget *parent = nullptr) : QListWidget(parent) {
    }

protected:
    void startDrag(Qt::DropActions supportedActions) override;
};

class MainWindow : public QMainWindow {
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

    void changeEllipseRadius(int);

    void changeArcProperties(int);

    void changeIntProperties(int);

    void changeFloatProperties(int);

    void changeTextProperties();

    void saveShapesToFile();

    void loadShapesFromFile();

    void exportControlsInfo();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QDockWidget *shapesDock;
    QDockWidget *layersDock;
    QListWidget *shapesListWidget;

    QComboBox *colorComboBox;

    // Shape layer controls
    QList<QCheckBox *> layerCheckBoxes;
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

    // Line-specific controls
    QWidget *linePropertiesWidget;
    QSpinBox *lineEndXSpinBox;
    QSpinBox *lineEndYSpinBox;

    // Ellipse-specific controls
    QWidget *ellipsePropertiesWidget;
    QSpinBox *ellipseRadiusXSpinBox;
    QSpinBox *ellipseRadiusYSpinBox;

    // Arc-specific controls
    QWidget *arcPropertiesWidget;
    QSpinBox *arcRadiusXSpinBox;
    QSpinBox *arcRadiusYSpinBox;
    QSpinBox *arcStartAngleSpinBox;
    QSpinBox *arcSpanAngleSpinBox;

    // Int-specific controls
    QWidget *intValuePropertiesWidget;
    QSpinBox *intValueSpinBox;
    QSpinBox *intFontSizeSpinBox;

    // Float-specific controls
    QWidget *floatValuePropertiesWidget;
    QSpinBox *floatValueSpinBox;
    QSpinBox *floatFontSizeSpinBox;

    // Text-specific controls
    QWidget *textPropertiesWidget;
    QLineEdit *textLineEdit;
    QSpinBox *textFontSizeSpinBox;

    // Helper methods for color management
    QColor getColorFromName(const QString &colorName) const;

    QString getNameFromColor(const QColor &color) const;

    QList<QString> getAvailableColors() const;

    // Helper methods
    void createShapeToolbar();

    void createLayerPanel();

    QPixmap createShapeIcon(const QString &shape);
};

#endif // MAINWINDOW_H
