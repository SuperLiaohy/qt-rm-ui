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
#include <QMouseEvent>
#include <QColorDialog>
#include <QToolBar>
#include <QSpinBox>
#include <QVBoxLayout>
// DragDropImageLabel 实现
DragDropImageLabel::DragDropImageLabel(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true); // 允许接收拖放

    // Initially all layers are visible
    for (int i = 0; i <= 9; i++) {
        visibleLayers.insert(i);
    }
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

// Add these two methods to DragDropImageLabel class:
int DragDropImageLabel::getSelectedShapeX() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].x;
    }
    return 0;
}

int DragDropImageLabel::getSelectedShapeY() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].y;
    }
    return 0;
}


// void DragDropImageLabel::paintEvent(QPaintEvent *event)
// {
//     QPainter painter(this);
//     painter.fillRect(rect(), QColor(240, 240, 240)); // 背景色
//
//     if (originalPixmap.isNull()) {
//         QLabel::paintEvent(event);
//         return;
//     }
//
//     // 计算适合窗口的缩放尺寸，保持宽高比
//     QSize scaledSize = originalPixmap.size();
//     scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
//
//     // 计算图片居中位置
//     int x = (width() - scaledSize.width()) / 2;
//     int y = (height() - scaledSize.height()) / 2;
//
//     // 绘制缩放后的图片
//     painter.drawPixmap(x, y, scaledSize.width(), scaledSize.height(), originalPixmap);
//
//     // 计算缩放比例
//     double scaleX = (double)scaledSize.width() / originalPixmap.width();
//     double scaleY = (double)scaledSize.height() / originalPixmap.height();
//
//     painter.setRenderHint(QPainter::Antialiasing);
//
//     // 绘制所有形状
//     for (const auto &shape : shapes) {
//         // 计算形状在缩放后图片上的位置
//         int imgX = x + shape.xPercent * originalPixmap.width() * scaleX;
//         int imgY = y + shape.yPercent * originalPixmap.height() * scaleY;
//
//         // 计算形状大小（也要缩放）
//         int minDimension = qMin(originalPixmap.width(), originalPixmap.height());
//         int shapeSize = shape.sizePercent * minDimension * scaleX;
//
//         QRect shapeRect(imgX - shapeSize/2, imgY - shapeSize/2, shapeSize, shapeSize);
//
//         if (shape.type == "圆形") {
//             painter.setPen(QPen(Qt::red, 2));
//             painter.drawEllipse(shapeRect);
//         } else if (shape.type == "矩形") {
//             painter.setPen(QPen(Qt::blue, 2));
//             painter.drawRect(shapeRect);
//         }
//     }
// }

void DragDropImageLabel::dropEvent(QDropEvent *event)
{
    if (originalPixmap.isNull())
        return;

    QString shapeType = event->mimeData()->text();

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);

    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    QRect imgRect(x, y, scaledSize.width(), scaledSize.height());
    if (!imgRect.contains(event->pos())) {
        return;
    }

    double scaleX = (double)originalPixmap.width() / scaledSize.width();
    double scaleY = (double)originalPixmap.height() / scaledSize.height();

    double relX = (event->pos().x() - x) * scaleX;
    double relY = (event->pos().y() - y) * scaleY;

    // Convert to absolute coordinates in 1920x1080 range
    int absX = (relX / originalPixmap.width()) * 1920;
    int absY = (relY / originalPixmap.height()) * 1080;

    Shape shape;
    shape.type = shapeType;
    shape.x = absX;
    shape.y = absY;
    shape.sizePercent = 0.05;
    shape.layer = 5; // Default to middle layer

    if (shape.type == "圆形") {
        shape.color = Qt::red;
        shape.specific.circle.radius = 50; // Default radius (in absolute coordinates)
    } else if (shape.type == "矩形") {
        shape.color = Qt::blue;
        // Set default end point to be 100 units away from starting point
        shape.specific.rect.endX = absX + 100;
        shape.specific.rect.endY = absY + 100;
    } else if (shape.type == "直线") {
        shape.color = Qt::green;
        // Set default end point to be 100 units away from starting point
        shape.specific.line.endX = absX + 100;
        shape.specific.line.endY = absY + 100;
    }
    shape.borderWidth = 2;

    shapes.append(shape);

    update();
    event->acceptProposedAction();
}

void DragDropImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(240, 240, 240));

    if (originalPixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    painter.drawPixmap(x, y, scaledSize.width(), scaledSize.height(), originalPixmap);

    double scaleX = (double)scaledSize.width() / originalPixmap.width();
    double scaleY = (double)scaledSize.height() / originalPixmap.height();

    painter.setRenderHint(QPainter::Antialiasing);

    // Create a list of indices sorted by layer
    QList<int> sortedIndices;
    for (int i = 0; i < shapes.size(); i++) {
        // Only include shapes on visible layers
        if (visibleLayers.contains(shapes[i].layer)) {
            sortedIndices.append(i);
        }
    }

    // Sort indices by layer (lower layers first)
    std::sort(sortedIndices.begin(), sortedIndices.end(),
        [this](int a, int b) { return shapes[a].layer < shapes[b].layer; });

    // Draw shapes according to layer order
    for (int idx : sortedIndices) {
        const auto &shape = shapes[idx];

        // Set pen based on selection state
        if (idx == selectedShapeIndex) {
            painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        } else {
            painter.setPen(QPen(shape.color, shape.borderWidth));
        }

        if (shape.type == "圆形") {
            // Convert from absolute coordinates to screen coordinates
            double relX = (double)shape.x / 1920 * originalPixmap.width();
            double relY = (double)shape.y / 1080 * originalPixmap.height();

            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Convert radius from absolute to screen coordinates
            double radiusRel = (double)shape.specific.circle.radius / 1920 * originalPixmap.width();
            int radiusScreen = radiusRel * scaleX;

            QRect circleRect(imgX - radiusScreen, imgY - radiusScreen,
                            radiusScreen * 2, radiusScreen * 2);

            // Draw selection rectangle if selected
            if (idx == selectedShapeIndex) {
                painter.drawRect(circleRect.adjusted(-3, -3, 3, 3));
                painter.setPen(QPen(shape.color.darker(120), shape.borderWidth));
            }

            painter.drawEllipse(circleRect);
        }
        else if (shape.type == "矩形") {
            // Convert from absolute coordinates to screen coordinates
            double startRelX = (double)shape.x / 1920 * originalPixmap.width();
            double startRelY = (double)shape.y / 1080 * originalPixmap.height();
            double endRelX = (double)shape.specific.rect.endX / 1920 * originalPixmap.width();
            double endRelY = (double)shape.specific.rect.endY / 1080 * originalPixmap.height();

            int imgStartX = x + startRelX * scaleX;
            int imgStartY = y + startRelY * scaleY;
            int imgEndX = x + endRelX * scaleX;
            int imgEndY = y + endRelY * scaleY;

            QRect rectArea(QPoint(qMin(imgStartX, imgEndX), qMin(imgStartY, imgEndY)),
                          QPoint(qMax(imgStartX, imgEndX), qMax(imgStartY, imgEndY)));

            // Draw selection rectangle if selected
            if (idx == selectedShapeIndex) {
                painter.drawRect(rectArea.adjusted(-3, -3, 3, 3));
                painter.setPen(QPen(shape.color.darker(120), shape.borderWidth));
            }

            painter.drawRect(rectArea);
        }
        else if (shape.type == "直线") {
            // Convert from absolute coordinates to screen coordinates
            double startRelX = (double)shape.x / 1920 * originalPixmap.width();
            double startRelY = (double)shape.y / 1080 * originalPixmap.height();
            double endRelX = (double)shape.specific.line.endX / 1920 * originalPixmap.width();
            double endRelY = (double)shape.specific.line.endY / 1080 * originalPixmap.height();

            int imgStartX = x + startRelX * scaleX;
            int imgStartY = y + startRelY * scaleY;
            int imgEndX = x + endRelX * scaleX;
            int imgEndY = y + endRelY * scaleY;

            // Draw selection indicators if selected
            if (idx == selectedShapeIndex) {
                // Draw start point indicator
                painter.drawRect(imgStartX - 3, imgStartY - 3, 6, 6);
                // Draw end point indicator
                painter.drawRect(imgEndX - 3, imgEndY - 3, 6, 6);

                painter.setPen(QPen(shape.color, shape.borderWidth));
            }

            painter.drawLine(imgStartX, imgStartY, imgEndX, imgEndY);
        }
    }
}

// Add mouse event handlers to DragDropImageLabel
void DragDropImageLabel::mousePressEvent(QMouseEvent *event)
{
    if (originalPixmap.isNull())
        return;

    int oldSelection = selectedShapeIndex;
    selectedShapeIndex = -1;  // Reset selection first
    dragMode = DRAG_NONE;

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    // Check if click is outside image area
    QRect imageRect(x, y, scaledSize.width(), scaledSize.height());
    if (!imageRect.contains(event->pos())) {
        if (oldSelection != selectedShapeIndex) {
            emit selectionChanged();
        }
        return;
    }

    double scaleX = (double)scaledSize.width() / originalPixmap.width();
    double scaleY = (double)scaledSize.height() / originalPixmap.height();

    // Check shapes in reverse order (top to bottom) to select the topmost shape first
    for (int i = shapes.size() - 1; i >= 0; i--) {
        const auto &shape = shapes[i];

        // Skip shapes on hidden layers
        if (!visibleLayers.contains(shape.layer)) {
            continue;
        }

        if (shape.type == "圆形") {
            // Convert center point to screen coordinates
            double relX = (double)shape.x / 1920 * originalPixmap.width();
            double relY = (double)shape.y / 1080 * originalPixmap.height();
            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Convert radius to screen coordinates
            double radiusRel = (double)shape.specific.circle.radius / 1920 * originalPixmap.width();
            int radiusScreen = radiusRel * scaleX;

            // Calculate distance from click to center
            QPoint center(imgX, imgY);
            qreal distance = QLineF(center, event->pos()).length();

            // Check if click is within circle border (use a reasonable width for selection)
            int selectionWidth = qMax(5, shape.borderWidth);
            if (qAbs(distance - radiusScreen) <= selectionWidth) {
                selectedShapeIndex = i;
                dragMode = DRAG_SHAPE;

                // Store the offset between click point and shape center
                QPoint clickOffset = event->pos() - center;

                // Calculate the offset in original coordinates
                dragStartPos = QPoint(
                    clickOffset.x() / scaleX * 1920 / originalPixmap.width(),
                    clickOffset.y() / scaleY * 1080 / originalPixmap.height()
                );
                break;
            }
        }
        else if (shape.type == "矩形") {
            // Convert rectangle coordinates to screen coordinates
            double startRelX = (double)shape.x / 1920 * originalPixmap.width();
            double startRelY = (double)shape.y / 1080 * originalPixmap.height();
            double endRelX = (double)shape.specific.rect.endX / 1920 * originalPixmap.width();
            double endRelY = (double)shape.specific.rect.endY / 1080 * originalPixmap.height();

            int imgStartX = x + startRelX * scaleX;
            int imgStartY = y + startRelY * scaleY;
            int imgEndX = x + endRelX * scaleX;
            int imgEndY = y + endRelY * scaleY;

            // Create the rectangle corners with hit area
            QPoint topLeft(qMin(imgStartX, imgEndX), qMin(imgStartY, imgEndY));
            QPoint topRight(qMax(imgStartX, imgEndX), qMin(imgStartY, imgEndY));
            QPoint bottomLeft(qMin(imgStartX, imgEndX), qMax(imgStartY, imgEndY));
            QPoint bottomRight(qMax(imgStartX, imgEndX), qMax(imgStartY, imgEndY));

            int cornerSize = 10; // Size of corner hit area

            // Check if click is on a corner
            if ((event->pos() - topLeft).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 0; // Top-left
                break;
            } else if ((event->pos() - topRight).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 1; // Top-right
                break;
            } else if ((event->pos() - bottomLeft).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 2; // Bottom-left
                break;
            } else if ((event->pos() - bottomRight).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 3; // Bottom-right
                break;
            }

            // Create the rectangle
            QRect rectOuter(topLeft, bottomRight);

            // Create inner rectangle for border detection
            int borderWidth = qMax(5, shape.borderWidth);
            QRect rectInner = rectOuter.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);

            // Check if click is within rectangle border
            if (rectOuter.contains(event->pos()) && !rectInner.contains(event->pos())) {
                selectedShapeIndex = i;
                dragMode = DRAG_SHAPE;

                // Store exact click point relative to shape (important for precise dragging)
                // We need the click offset from the shape's reference point (x,y)
                QPoint shapeTopLeft(imgStartX, imgStartY);
                QPoint clickOffset = event->pos() - shapeTopLeft;

                // Calculate the offset in original coordinates
                dragStartPos = QPoint(
                    clickOffset.x() / scaleX * 1920 / originalPixmap.width(),
                    clickOffset.y() / scaleY * 1080 / originalPixmap.height()
                );
                break;
            }
        }
                else if (shape.type == "直线") {
            // Convert line coordinates to screen coordinates
            double startRelX = (double)shape.x / 1920 * originalPixmap.width();
            double startRelY = (double)shape.y / 1080 * originalPixmap.height();
            double endRelX = (double)shape.specific.line.endX / 1920 * originalPixmap.width();
            double endRelY = (double)shape.specific.line.endY / 1080 * originalPixmap.height();

            int imgStartX = x + startRelX * scaleX;
            int imgStartY = y + startRelY * scaleY;
            int imgEndX = x + endRelX * scaleX;
            int imgEndY = y + endRelY * scaleY;

            // Create hit points for start and end points
            QPoint startPoint(imgStartX, imgStartY);
            QPoint endPoint(imgEndX, imgEndY);

            int cornerSize = 10; // Size of endpoint hit area

            // Check if click is on a line endpoint
            if ((event->pos() - startPoint).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 0; // Start point
                break;
            } else if ((event->pos() - endPoint).manhattanLength() <= cornerSize) {
                selectedShapeIndex = i;
                dragMode = DRAG_RECTANGLE_CORNER;
                dragCorner = 1; // End point
                break;
            }

            // Calculate distance from click to line
            QLineF line(startPoint, endPoint);
            QPointF clickPos(event->pos());

            // Project point onto line to find perpendicular distance
            QLineF normal = line.normalVector();
            normal.translate(clickPos - line.p1());
            QPointF intersection;
            line.intersects(normal, &intersection);

            qreal distance = QLineF(clickPos, intersection).length();

            // Check if click is close enough to line
            int selectionWidth = qMax(5, shape.borderWidth);
            if (distance <= selectionWidth &&
                QLineF(line.p1(), intersection).length() <= line.length() &&
                QLineF(line.p2(), intersection).length() <= line.length()) {
                selectedShapeIndex = i;
                dragMode = DRAG_SHAPE;

                // Store exact click point relative to shape
                QPoint clickOffset = event->pos() - startPoint;

                // Calculate the offset in original coordinates
                dragStartPos = QPoint(
                    clickOffset.x() / scaleX * 1920 / originalPixmap.width(),
                    clickOffset.y() / scaleY * 1080 / originalPixmap.height()
                );
                break;
            }
        }

    }

    update();
    if (oldSelection != selectedShapeIndex) {
        emit selectionChanged();
    }
}
void DragDropImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (dragMode == DRAG_NONE || selectedShapeIndex < 0 || selectedShapeIndex >= shapes.size())
        return;

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    double scaleX = (double)originalPixmap.width() / scaledSize.width();
    double scaleY = (double)originalPixmap.height() / scaledSize.height();

    if (dragMode == DRAG_SHAPE) {
        // Convert mouse position to absolute coordinates (0-1920, 0-1080)
        int mouseX = qRound((event->pos().x() - x) * scaleX * 1920 / originalPixmap.width());
        int mouseY = qRound((event->pos().y() - y) * scaleY * 1080 / originalPixmap.height());

        // Calculate new position based on where we clicked on the shape
        int newX = mouseX - dragStartPos.x();
        int newY = mouseY - dragStartPos.y();

        // Set shape position, keep within bounds
        shapes[selectedShapeIndex].x = qBound(0, newX, 1920);
        shapes[selectedShapeIndex].y = qBound(0, newY, 1080);

        // For rectangles, we need to move the end point too to maintain shape
        if (shapes[selectedShapeIndex].type == "矩形") {
            // Calculate the width and height of the rectangle
            int width = shapes[selectedShapeIndex].specific.rect.endX -
                        (mouseX - dragStartPos.x() - newX + shapes[selectedShapeIndex].x);
            int height = shapes[selectedShapeIndex].specific.rect.endY -
                         (mouseY - dragStartPos.y() - newY + shapes[selectedShapeIndex].y);

            // Update end point to maintain rectangle dimensions
            shapes[selectedShapeIndex].specific.rect.endX = shapes[selectedShapeIndex].x + width;
            shapes[selectedShapeIndex].specific.rect.endY = shapes[selectedShapeIndex].y + height;
        }
        // For lines, we need to move the end point too to maintain shape
        if (shapes[selectedShapeIndex].type == "直线") {
            // Calculate the width and height of the line
            int width = shapes[selectedShapeIndex].specific.line.endX -
                      (mouseX - dragStartPos.x() - newX + shapes[selectedShapeIndex].x);
            int height = shapes[selectedShapeIndex].specific.line.endY -
                       (mouseY - dragStartPos.y() - newY + shapes[selectedShapeIndex].y);

            // Update end point to maintain line dimensions
            shapes[selectedShapeIndex].specific.line.endX = shapes[selectedShapeIndex].x + width;
            shapes[selectedShapeIndex].specific.line.endY = shapes[selectedShapeIndex].y + height;
        }

        update();
        emit selectionChanged();
    }
    else if (dragMode == DRAG_RECTANGLE_CORNER && shapes[selectedShapeIndex].type == "矩形") {
        // Convert mouse position to absolute coordinates (0-1920, 0-1080)
        int mouseX = qRound((event->pos().x() - x) * scaleX * 1920 / originalPixmap.width());
        int mouseY = qRound((event->pos().y() - y) * scaleY * 1080 / originalPixmap.height());

        // Constrain to image bounds
        mouseX = qBound(0, mouseX, 1920);
        mouseY = qBound(0, mouseY, 1080);

        // Update the appropriate corner based on which one is being dragged
        if (dragCorner == 0) { // Top-left
            shapes[selectedShapeIndex].x = mouseX;
            shapes[selectedShapeIndex].y = mouseY;
        }
        else if (dragCorner == 1) { // Top-right
            shapes[selectedShapeIndex].specific.rect.endX = mouseX;
            shapes[selectedShapeIndex].y = mouseY;
        }
        else if (dragCorner == 2) { // Bottom-left
            shapes[selectedShapeIndex].x = mouseX;
            shapes[selectedShapeIndex].specific.rect.endY = mouseY;
        }
        else if (dragCorner == 3) { // Bottom-right
            shapes[selectedShapeIndex].specific.rect.endX = mouseX;
            shapes[selectedShapeIndex].specific.rect.endY = mouseY;
        }

        update();
        emit selectionChanged();
    }
    else if (dragMode == DRAG_RECTANGLE_CORNER && shapes[selectedShapeIndex].type == "直线") {
        // Convert mouse position to absolute coordinates (0-1920, 0-1080)
            int mouseX = qRound((event->pos().x() - x) * scaleX * 1920 / originalPixmap.width());
            int mouseY = qRound((event->pos().y() - y) * scaleY * 1080 / originalPixmap.height());

            // Constrain to image bounds
            mouseX = qBound(0, mouseX, 1920);
            mouseY = qBound(0, mouseY, 1080);

            if (dragCorner == 0) { // Start point
                shapes[selectedShapeIndex].x = mouseX;
                shapes[selectedShapeIndex].y = mouseY;
            }
            else if (dragCorner == 1) { // End point
                shapes[selectedShapeIndex].specific.line.endX = mouseX;
                shapes[selectedShapeIndex].specific.line.endY = mouseY;
            }

            update();
            emit selectionChanged();

    }
}
bool DragDropImageLabel::hasSelectedShape() const
{
    return selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size();
}
void DragDropImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event) return;
    dragMode = DRAG_NONE;
    dragCorner = -1;
}

void DragDropImageLabel::setShapePosition(int x, int y)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].x = qBound(0, x, 1920);
        shapes[selectedShapeIndex].y = qBound(0, y, 1080);
        update();
    }
}


qreal DragDropImageLabel::getSelectedShapeSize() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].sizePercent;
    }
    return 0.05; // Default size if no shape selected
}

void DragDropImageLabel::setShapeSize(qreal size)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].sizePercent = size;
        update();
    }
}

// Methods to manipulate selected shape
void DragDropImageLabel::deleteSelectedShape()
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes.removeAt(selectedShapeIndex);
        selectedShapeIndex = -1; // Deselect after deletion
        update();
    }
}

void DragDropImageLabel::setShapeColor(const QColor &color)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].color = color;
        update();
    }
}

void DragDropImageLabel::setBorderWidth(int width)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].borderWidth = width;
        update();
    }
}

// Implementation in mainwindow.cpp
QColor DragDropImageLabel::getSelectedShapeColor() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].color;
    }
    return Qt::black; // Default color if no shape selected
}

int DragDropImageLabel::getSelectedShapeBorderWidth() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].borderWidth;
    }
    return 2; // Default width if no shape selected
}

void DragDropImageLabel::setShapeLayer(int layer)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].layer = qBound(0, layer, 9);
        update();
    }
}

int DragDropImageLabel::getSelectedShapeLayer() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].layer;
    }
    return 5; // Default layer if no shape selected
}
QSet<int> DragDropImageLabel::getVisibleLayers() const
{
    return visibleLayers;
}
void DragDropImageLabel::setVisibleLayers(const QSet<int>& layers)
{
    visibleLayers = layers;
    update();  // Redraw with the new visibility settings
}

QString DragDropImageLabel::getSelectedShapeType() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].type;
    }
    return QString(); // Return empty string if no shape is selected
}

int DragDropImageLabel::getSelectedShapeEndX() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "矩形") {
        return shapes[selectedShapeIndex].specific.rect.endX;
        }
    return 0; // Default value if no rectangle is selected
}

int DragDropImageLabel::getSelectedShapeEndY() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "矩形") {
        return shapes[selectedShapeIndex].specific.rect.endY;
        }
    return 0; // Default value if no rectangle is selected
}

int DragDropImageLabel::getSelectedShapeRadius() const
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆形") {
        return shapes[selectedShapeIndex].specific.circle.radius;
        }
    return 50; // Default radius if no circle is selected
}

void DragDropImageLabel::setRectangleEndPoint(int x, int y)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "矩形") {
        shapes[selectedShapeIndex].specific.rect.endX = qBound(0, x, 1920);
        shapes[selectedShapeIndex].specific.rect.endY = qBound(0, y, 1080);
        update();
        }
}

void DragDropImageLabel::setCircleRadius(int radius)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆形") {
        shapes[selectedShapeIndex].specific.circle.radius = qBound(1, radius, 500);
        update();
        }
}

void DragDropImageLabel::setLineEndPoint(int x, int y)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "直线") {
        shapes[selectedShapeIndex].specific.line.endX = x;
        shapes[selectedShapeIndex].specific.line.endY = y;
        update();
        }
}

// ShapeListItem 实现
ShapeListItem::ShapeListItem(const QString &text, QListWidget *parent)
    : QListWidgetItem(text, parent)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}




void MainWindow::onShapeSelectionChanged()
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (!imageLabel || !imageLabel->hasSelectedShape())
        return;

    // Don't call updatePropertyControls() from here
    // Only handle shape-specific UI changes
    QString shapeType = imageLabel->getSelectedShapeType();

    if (shapeType == "矩形") {
        // Set up rectangle-specific controls
        rectEndXSpinBox->blockSignals(true);
        rectEndYSpinBox->blockSignals(true);

        rectEndXSpinBox->setValue(imageLabel->getSelectedShapeEndX());
        rectEndYSpinBox->setValue(imageLabel->getSelectedShapeEndY());

        rectEndXSpinBox->blockSignals(false);
        rectEndYSpinBox->blockSignals(false);

        // Show rectangle properties panel
        shapeSpecificControls->setCurrentWidget(rectanglePropertiesWidget);
    }
    else if (shapeType == "圆形") {
        // Set up circle-specific controls
        circleRadiusSpinBox->blockSignals(true);
        circleRadiusSpinBox->setValue(imageLabel->getSelectedShapeRadius());
        circleRadiusSpinBox->blockSignals(false);

        // Show circle properties panel
        shapeSpecificControls->setCurrentWidget(circlePropertiesWidget);
    }
}

void MainWindow::changeRectangleEndPoint(int)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "矩形") {
        imageLabel->setRectangleEndPoint(
            rectEndXSpinBox->value(),
            rectEndYSpinBox->value()
        );
    }
}

void MainWindow::changeCircleRadius(int radius)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "圆形") {
        imageLabel->setCircleRadius(radius);
    }
}

void MainWindow::changeLineEndPoint(int)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "直线") {
        imageLabel->setLineEndPoint(lineEndXSpinBox->value(), lineEndYSpinBox->value());
        }
}

void ShapeListWidget::startDrag(Qt::DropActions supportedActions)
{
    QListWidgetItem *item = currentItem();
    if (item) {
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(item->text());

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // Create a pixmap of the item for the drag cursor
        QPixmap pixmap = item->icon().pixmap(32, 32);
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

        drag->exec(supportedActions, Qt::CopyAction);
    }
}

// This overrides the updatePropertyControls method to include shape-specific properties
void MainWindow::updatePropertyControls()
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (!imageLabel || !imageLabel->hasSelectedShape()) {
        // Disable all controls if no shape is selected
        widthSpinBox->setEnabled(false);
        xPosSpinBox->setEnabled(false);
        yPosSpinBox->setEnabled(false);
        layerSpinBox->setEnabled(false);
        shapeSpecificControls->setEnabled(false);
        return;
    }

    // Enable controls
    widthSpinBox->setEnabled(true);
    xPosSpinBox->setEnabled(true);
    yPosSpinBox->setEnabled(true);
    layerSpinBox->setEnabled(true);
    shapeSpecificControls->setEnabled(true);

    // Block signals to prevent feedback loop
    widthSpinBox->blockSignals(true);
    xPosSpinBox->blockSignals(true);
    yPosSpinBox->blockSignals(true);
    layerSpinBox->blockSignals(true);

    // Update controls with current values - using accessor methods
    widthSpinBox->setValue(imageLabel->getSelectedShapeBorderWidth());
    xPosSpinBox->setValue(imageLabel->getSelectedShapeX());
    yPosSpinBox->setValue(imageLabel->getSelectedShapeY());
    layerSpinBox->setValue(imageLabel->getSelectedShapeLayer());

    // Re-enable signals
    widthSpinBox->blockSignals(false);
    xPosSpinBox->blockSignals(false);
    yPosSpinBox->blockSignals(false);
    layerSpinBox->blockSignals(false);

    // Handle shape-specific properties separately
    QString shapeType = imageLabel->getSelectedShapeType();

    if (shapeType == "矩形") {
        // Set up rectangle-specific controls
        rectEndXSpinBox->blockSignals(true);
        rectEndYSpinBox->blockSignals(true);

        rectEndXSpinBox->setValue(imageLabel->getSelectedShapeEndX());
        rectEndYSpinBox->setValue(imageLabel->getSelectedShapeEndY());

        rectEndXSpinBox->blockSignals(false);
        rectEndYSpinBox->blockSignals(false);

        // Show rectangle properties panel
        shapeSpecificControls->setCurrentWidget(rectanglePropertiesWidget);
    }
    else if (shapeType == "圆形") {
        // Set up circle-specific controls
        circleRadiusSpinBox->blockSignals(true);
        circleRadiusSpinBox->setValue(imageLabel->getSelectedShapeRadius());
        circleRadiusSpinBox->blockSignals(false);

        // Show circle properties panel
        shapeSpecificControls->setCurrentWidget(circlePropertiesWidget);
    }
    else if (shapeType == "直线") {
        // Set up line-specific controls
        lineEndXSpinBox->blockSignals(true);
        lineEndYSpinBox->blockSignals(true);

        lineEndXSpinBox->setValue(imageLabel->getSelectedShapeEndX());
        lineEndYSpinBox->setValue(imageLabel->getSelectedShapeEndY());

        lineEndXSpinBox->blockSignals(false);
        lineEndYSpinBox->blockSignals(false);

        // Show line properties panel
        shapeSpecificControls->setCurrentWidget(linePropertiesWidget);
    }
}
// Modified version of createShapeToolbar to include shape-specific property controls
void MainWindow::createShapeToolbar()
{
    // Create the shapes dock widget and list (your existing code)
    shapesDock = new QDockWidget(tr("形状工具"), this);
    shapesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    shapesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    shapesListWidget = new ShapeListWidget(shapesDock);
    shapesListWidget->setDragEnabled(true);
    shapesListWidget->setViewMode(QListWidget::IconMode);
    shapesListWidget->setIconSize(QSize(40, 40));
    shapesListWidget->setSpacing(10);

    // Add shape items
    auto *circleItem = new ShapeListItem(tr("圆形"), shapesListWidget);
    circleItem->setIcon(QIcon(createShapeIcon("圆形")));

    auto *rectItem = new ShapeListItem(tr("矩形"), shapesListWidget);
    rectItem->setIcon(QIcon(createShapeIcon("矩形")));

    // Add this to your createShapeToolbar() method after the rectangle item
    auto *lineItem = new ShapeListItem(tr("直线"), shapesListWidget);
    lineItem->setIcon(QIcon(createShapeIcon("直线")));

    shapesDock->setWidget(shapesListWidget);
    addDockWidget(Qt::RightDockWidgetArea, shapesDock);

    // Create toolbar for common properties
    QToolBar *propertiesToolbar = addToolBar(tr("形状属性"));

    QAction *deleteAction = propertiesToolbar->addAction(tr("删除"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedShape);

    QAction *colorAction = propertiesToolbar->addAction(tr("更改颜色"));
    connect(colorAction, &QAction::triggered, this, &MainWindow::changeShapeColor);

    propertiesToolbar->addSeparator();

    // Common properties
    QLabel *widthLabel = new QLabel(tr("线宽: "));
    propertiesToolbar->addWidget(widthLabel);

    widthSpinBox = new QSpinBox();
    widthSpinBox->setRange(1, 10);
    widthSpinBox->setValue(2);
    propertiesToolbar->addWidget(widthSpinBox);
    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeBorderWidth);



    propertiesToolbar->addSeparator();

    // Position controls
    QLabel *xPosLabel = new QLabel(tr("X坐标: "));
    propertiesToolbar->addWidget(xPosLabel);

    xPosSpinBox = new QSpinBox();
    xPosSpinBox->setRange(0, 1920);
    xPosSpinBox->setSingleStep(1);
    propertiesToolbar->addWidget(xPosSpinBox);
    connect(xPosSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeShapePosition);

    QLabel *yPosLabel = new QLabel(tr("Y坐标: "));
    propertiesToolbar->addWidget(yPosLabel);

    yPosSpinBox = new QSpinBox();
    yPosSpinBox->setRange(0, 1080);
    yPosSpinBox->setSingleStep(1);
    propertiesToolbar->addWidget(yPosSpinBox);
    connect(yPosSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeShapePosition);

    // Layer control
    propertiesToolbar->addSeparator();
    QLabel *layerLabel = new QLabel(tr("图层: "));
    propertiesToolbar->addWidget(layerLabel);

    layerSpinBox = new QSpinBox();
    layerSpinBox->setRange(0, 9);
    layerSpinBox->setValue(5);
    layerSpinBox->setToolTip(tr("0为最底层，9为最顶层"));
    propertiesToolbar->addWidget(layerSpinBox);
    connect(layerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeShapeLayer);

    // Create a new dock widget for shape-specific properties
    QDockWidget *specificPropDock = new QDockWidget(tr("形状特有属性"), this);
    specificPropDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    specificPropDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Create stacked widget to switch between different shape property panels
    shapeSpecificControls = new QStackedWidget(specificPropDock);

    // Rectangle properties
    rectanglePropertiesWidget = new QWidget();
    QVBoxLayout *rectLayout = new QVBoxLayout(rectanglePropertiesWidget);

    QLabel *rectEndXLabel = new QLabel(tr("终点X坐标:"));
    rectEndXSpinBox = new QSpinBox();
    rectEndXSpinBox->setRange(0, 1920);
    rectEndXSpinBox->setSingleStep(1);
    connect(rectEndXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeRectangleEndPoint);

    QLabel *rectEndYLabel = new QLabel(tr("终点Y坐标:"));
    rectEndYSpinBox = new QSpinBox();
    rectEndYSpinBox->setRange(0, 1080);
    rectEndYSpinBox->setSingleStep(1);
    connect(rectEndYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeRectangleEndPoint);

    rectLayout->addWidget(rectEndXLabel);
    rectLayout->addWidget(rectEndXSpinBox);
    rectLayout->addWidget(rectEndYLabel);
    rectLayout->addWidget(rectEndYSpinBox);
    rectLayout->addStretch();

    // Circle properties
    circlePropertiesWidget = new QWidget();
    QVBoxLayout *circleLayout = new QVBoxLayout(circlePropertiesWidget);

    QLabel *radiusLabel = new QLabel(tr("半径:"));
    circleRadiusSpinBox = new QSpinBox();
    circleRadiusSpinBox->setRange(1, 500);
    circleRadiusSpinBox->setSingleStep(1);
    connect(circleRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeCircleRadius);

    circleLayout->addWidget(radiusLabel);
    circleLayout->addWidget(circleRadiusSpinBox);
    circleLayout->addStretch();


    // Add after circle properties widget setup
    // Line properties
    linePropertiesWidget = new QWidget();
    QVBoxLayout *lineLayout = new QVBoxLayout(linePropertiesWidget);

    QLabel *lineEndXLabel = new QLabel(tr("终点X坐标:"));
    lineEndXSpinBox = new QSpinBox();
    lineEndXSpinBox->setRange(0, 1920);
    lineEndXSpinBox->setSingleStep(1);
    connect(lineEndXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeLineEndPoint);

    QLabel *lineEndYLabel = new QLabel(tr("终点Y坐标:"));
    lineEndYSpinBox = new QSpinBox();
    lineEndYSpinBox->setRange(0, 1080);
    lineEndYSpinBox->setSingleStep(1);
    connect(lineEndYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeLineEndPoint);

    lineLayout->addWidget(lineEndXLabel);
    lineLayout->addWidget(lineEndXSpinBox);
    lineLayout->addWidget(lineEndYLabel);
    lineLayout->addWidget(lineEndYSpinBox);
    lineLayout->addStretch();

    // Add widgets to stacked widget
    shapeSpecificControls->addWidget(linePropertiesWidget);

    // Add widgets to stacked widget
    shapeSpecificControls->addWidget(rectanglePropertiesWidget);
    shapeSpecificControls->addWidget(circlePropertiesWidget);

    // Set the stacked widget as the dock widget's content
    specificPropDock->setWidget(shapeSpecificControls);
    addDockWidget(Qt::RightDockWidgetArea, specificPropDock);

    // Connect selection changes to update property controls
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    auto *imgLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imgLabel) {
        connect(imgLabel, &DragDropImageLabel::selectionChanged,
                this, &MainWindow::updatePropertyControls);
    }

    // Initially disable all controls until a shape is selected
    widthSpinBox->setEnabled(false);
    xPosSpinBox->setEnabled(false);
    yPosSpinBox->setEnabled(false);
    layerSpinBox->setEnabled(false);
    shapeSpecificControls->setEnabled(false);
}







MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create central image display area
    imageLabel = new DragDropImageLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCentralWidget(imageLabel);

    // Create shape and layer toolbars
    createShapeToolbar();
    createLayerPanel();

    // Set up initial window properties
    resize(800, 600);
    setWindowTitle(tr("图形编辑器"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图片"),
                                                  "", tr("图片文件 (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QPixmap newImage = QPixmap::fromImageReader(&reader);

    if (newImage.isNull()) {
        QMessageBox::information(this, tr("无法加载图片"),
                                tr("无法加载图片：%1").arg(reader.errorString()));
        return;
    }

    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel) {
        imageLabel->setOriginalPixmap(newImage);
    }
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
    } else if (shape == "直线") {
        painter.setPen(QPen(Qt::green, 2));
        painter.drawLine(5, 5, 35, 35);
    }

    return pixmap;
}
void MainWindow::onLayerVisibilityChanged(int layer)
{
    // This method is called when a layer visibility checkbox is toggled
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (!checkbox) return;

    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (!imageLabel) return;

    // Get current visible layers
    QSet<int> visibleLayers = imageLabel->getVisibleLayers();

    if (checkbox->isChecked()) {
        // Add this layer to visible layers
        visibleLayers.insert(layer);
    } else {
        // Remove this layer from visible layers
        visibleLayers.remove(layer);
    }

    // Update the visible layers in the image label
    imageLabel->setVisibleLayers(visibleLayers);
}
void MainWindow::createLayerPanel()
{
    layersDock = new QDockWidget(tr("图层"), this);
    layersDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* layersWidget = new QWidget(layersDock);
    QVBoxLayout* layout = new QVBoxLayout(layersWidget);

    // Create checkboxes for each layer
    for (int i = 9; i >= 0; i--) {  // 9 is top layer, 0 is bottom
        QCheckBox* layerCheckbox = new QCheckBox(tr("图层 %1").arg(i));
        layerCheckbox->setChecked(true); // Initially all layers are visible

        // Use a lambda to capture the layer index
        connect(layerCheckbox, &QCheckBox::toggled, this, [this, i]() {
            this->onLayerVisibilityChanged(i);
        });

        layout->addWidget(layerCheckbox);
    }

    layersDock->setWidget(layersWidget);
    addDockWidget(Qt::RightDockWidgetArea, layersDock);
}


void MainWindow::changeShapeColor()
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (!imageLabel || !imageLabel->hasSelectedShape())
        return;

    QColor currentColor = imageLabel->getSelectedShapeColor();
    QColor newColor = QColorDialog::getColor(currentColor, this, tr("选择形状颜色"));

    if (newColor.isValid()) {
        imageLabel->setShapeColor(newColor);
    }
}

void MainWindow::deleteSelectedShape()
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->deleteSelectedShape();
        updatePropertyControls(); // Update UI after deletion
    }
}

void MainWindow::changeBorderWidth(int width)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->setBorderWidth(width);
    }
}

// void MainWindow::changeShapeSize(double size)
// {
//     auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
//     if (imageLabel && imageLabel->hasSelectedShape()) {
//         imageLabel->setShapeSize(size);
//     }
// }

void MainWindow::changeShapePosition(int)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->setShapePosition(xPosSpinBox->value(), yPosSpinBox->value());
    }
}

void MainWindow::changeShapeLayer(int layer)
{
    auto *imageLabel = dynamic_cast<DragDropImageLabel*>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->setShapeLayer(layer);
    }
}