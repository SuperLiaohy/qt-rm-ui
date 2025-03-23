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
#include <cmath>
// DragDropImageLabel 实现
DragDropImageLabel::DragDropImageLabel(QWidget *parent) : QLabel(parent) {
    setAcceptDrops(true); // 允许接收拖放

    // Initially all layers are visible
    for (int i = 0; i <= 9; i++) {
        visibleLayers.insert(i);
    }
}


void DragDropImageLabel::setOriginalPixmap(const QPixmap &pixmap) {
    // 保存原始图片
    originalPixmap = pixmap;

    // 更新显示（实际缩放在paintEvent中进行）
    update();
}

void DragDropImageLabel::dragEnterEvent(QDragEnterEvent *event) {
    // 检查拖动的数据是否包含我们需要的格式
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

// Add these two methods to DragDropImageLabel class:
int DragDropImageLabel::getSelectedShapeX() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].x;
    }
    return 0;
}

int DragDropImageLabel::getSelectedShapeY() const {
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

void DragDropImageLabel::dropEvent(QDropEvent *event) {
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

    double scaleX = (double) originalPixmap.width() / scaledSize.width();
    double scaleY = (double) originalPixmap.height() / scaledSize.height();

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
    shape.borderWidth = 2;
    shape.color = Qt::black; // Default color is black

    if (shape.type == "圆形") {
        shape.color = Qt::red;
        shape.specific.circle.radius = 50;
    } else if (shape.type == "矩形") {
        shape.color = Qt::blue;
        shape.specific.rect.endX = absX + 100;
        shape.specific.rect.endY = absY + 100;
    } else if (shape.type == "直线") {
        shape.color = Qt::green;
        shape.specific.line.endX = absX + 100;
        shape.specific.line.endY = absY + 100;
    } else if (shape.type == "椭圆") {
        shape.color = Qt::magenta;
        shape.specific.ellipse.radiusX = 50; // Default X radius
        shape.specific.ellipse.radiusY = 30; // Default Y radius
    } else if (shape.type == "圆弧") {
        shape.color = Qt::cyan;
        shape.specific.arc.radiusX = 50; // Default X radius
        shape.specific.arc.radiusY = 30; // Default Y radius
        shape.specific.arc.startAngle = 0; // Start at 12 o'clock
        shape.specific.arc.spanAngle = 180; // Default 90° arc (quarter circle)
    } else if (shape.type == "整数") {
        shape.color = Qt::black;
        shape.specific.intValue.value = 0; // Default value
        shape.specific.intValue.fontSize = 16; // Default font size
    } else if (shape.type == "浮点数") {
        shape.color = Qt::black;
        shape.specific.floatValue.value = 0; // Default value
        shape.specific.floatValue.fontSize = 16; // Default font size
    } else if (shape.type == "文本字符") {
        shape.color = Qt::black;

        // Initialize with default text
        const char *defaultText = "Text";
        int length = strlen(defaultText);
        memcpy(shape.specific.text.data, defaultText, length);
        shape.specific.text.data[length] = '\0'; // Null-terminate
        shape.specific.text.length = length;
        shape.specific.text.fontSize = 16; // Default font size
    }

    shapes.append(shape);

    update();
    event->acceptProposedAction();
}

void DragDropImageLabel::paintEvent(QPaintEvent *event) {
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

    double scaleX = (double) scaledSize.width() / originalPixmap.width();
    double scaleY = (double) scaledSize.height() / originalPixmap.height();

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
    for (int idx: sortedIndices) {
        const auto &shape = shapes[idx];

        // Set pen based on selection state
        if (idx == selectedShapeIndex) {
            painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        } else {
            painter.setPen(QPen(shape.color, shape.borderWidth));
        }

        if (shape.type == "圆形") {
            // Convert from absolute coordinates to screen coordinates
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();

            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Convert radius from absolute to screen coordinates
            double radiusRel = (double) shape.specific.circle.radius / 1920 * originalPixmap.width();
            int radiusScreen = radiusRel * scaleX;

            QRect circleRect(imgX - radiusScreen, imgY - radiusScreen,
                             radiusScreen * 2, radiusScreen * 2);

            // Draw selection rectangle if selected
            if (idx == selectedShapeIndex) {
                painter.drawRect(circleRect.adjusted(-3, -3, 3, 3));
                painter.setPen(QPen(shape.color.darker(120), shape.borderWidth));
            }

            painter.drawEllipse(circleRect);
        } else if (shape.type == "矩形") {
            // Convert from absolute coordinates to screen coordinates
            double startRelX = (double) shape.x / 1920 * originalPixmap.width();
            double startRelY = (double) shape.y / 1080 * originalPixmap.height();
            double endRelX = (double) shape.specific.rect.endX / 1920 * originalPixmap.width();
            double endRelY = (double) shape.specific.rect.endY / 1080 * originalPixmap.height();

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
        } else if (shape.type == "直线") {
            // Convert from absolute coordinates to screen coordinates
            double startRelX = (double) shape.x / 1920 * originalPixmap.width();
            double startRelY = (double) shape.y / 1080 * originalPixmap.height();
            double endRelX = (double) shape.specific.line.endX / 1920 * originalPixmap.width();
            double endRelY = (double) shape.specific.line.endY / 1080 * originalPixmap.height();

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
        } else if (shape.type == "椭圆") {
            // Convert from absolute coordinates to screen coordinates
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();

            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Convert radii from absolute to screen coordinates
            double radiusXRel = (double) shape.specific.ellipse.radiusX / 1920 * originalPixmap.width();
            double radiusYRel = (double) shape.specific.ellipse.radiusY / 1080 * originalPixmap.height();
            int radiusXScreen = radiusXRel * scaleX;
            int radiusYScreen = radiusYRel * scaleY;

            QRect ellipseRect(imgX - radiusXScreen, imgY - radiusYScreen,
                              radiusXScreen * 2, radiusYScreen * 2);

            // Draw selection rectangle if selected
            if (idx == selectedShapeIndex) {
                painter.drawRect(ellipseRect.adjusted(-3, -3, 3, 3));
                painter.setPen(QPen(shape.color.darker(120), shape.borderWidth));
            }

            painter.drawEllipse(ellipseRect);
        } else if (shape.type == "圆弧") {
            // Convert from absolute coordinates to screen coordinates
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();
            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Convert radii from absolute to screen coordinates
            double radiusXRel = (double) shape.specific.arc.radiusX / 1920 * originalPixmap.width();
            double radiusYRel = (double) shape.specific.arc.radiusY / 1080 * originalPixmap.height();
            int radiusXScreen = radiusXRel * scaleX;
            int radiusYScreen = radiusYRel * scaleY;

            QRect arcRect(imgX - radiusXScreen, imgY - radiusYScreen,
                          radiusXScreen * 2, radiusYScreen * 2);

            // Draw selection rectangle if selected
            if (idx == selectedShapeIndex) {
                // Draw bounding box
                painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
                painter.drawRect(arcRect);

                // Draw control points
                int startAngle = shape.specific.arc.startAngle;
                int spanAngle = shape.specific.arc.spanAngle;

                // Calculate points on the arc at start and end angles
                // Calculate points on the arc at start and end angles (in 12 o'clock=0° system)
                double startAngleRad = shape.specific.arc.startAngle * M_PI / 180.0;
                double endAngleRad = (shape.specific.arc.startAngle + shape.specific.arc.spanAngle) * M_PI / 180.0;

                // Convert from 12 o'clock=0° to standard math angles (0° at 3 o'clock)
                startAngleRad = M_PI / 2 - startAngleRad;
                endAngleRad = M_PI / 2 - endAngleRad;

                int startX = imgX + radiusXScreen * cos(startAngleRad);
                int startY = imgY - radiusYScreen * sin(startAngleRad);
                int endX = imgX + radiusXScreen * cos(endAngleRad);
                int endY = imgY - radiusYScreen * sin(endAngleRad);

                // Draw control points
                painter.fillRect(QRect(imgX - 4, imgY - 4, 8, 8), Qt::blue); // Center
                painter.fillRect(QRect(startX - 4, startY - 4, 8, 8), Qt::red); // Start angle
                painter.fillRect(QRect(endX - 4, endY - 4, 8, 8), Qt::green); // End angle
            }

            // Draw the arc (Note: Qt uses 16ths of a degree)
            // Draw the arc
            painter.setPen(QPen(shape.color, shape.borderWidth));
            // Convert from 12 o'clock=0°/clockwise to Qt's 3 o'clock=0°/counter-clockwise system
            int qtStartAngle = (90 - shape.specific.arc.startAngle) * 16;
            int qtSpanAngle = -shape.specific.arc.spanAngle * 16; // Negative for clockwise
            painter.drawArc(arcRect, qtStartAngle, qtSpanAngle);
        } else if (shape.type == "整数") {
            // Calculate scaled position
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();
            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Set font size and family
            QFont font("Arial", shape.specific.intValue.fontSize);
            painter.setFont(font);
            painter.setPen(QPen(shape.color, shape.borderWidth));

            // Draw the integer value as text
            QString text = QString::number(shape.specific.intValue.value);
            painter.drawText(imgX, imgY, text);
        } else if (shape.type == "浮点数") {
            // Calculate scaled position
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();
            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Set font size and family
            QFont font("Arial", shape.specific.floatValue.fontSize);
            painter.setFont(font);
            painter.setPen(QPen(shape.color, shape.borderWidth));

            // Draw the float value as text (divided by 1000)
            double displayValue = shape.specific.floatValue.value / 1000.0;
            QString text = QString::number(displayValue, 'f', 3);
            painter.drawText(imgX, imgY, text);
        } else if (shape.type == "文本字符") {
            // Calculate scaled position
            double relX = (double) shape.x / 1920 * originalPixmap.width();
            double relY = (double) shape.y / 1080 * originalPixmap.height();
            int imgX = x + relX * scaleX;
            int imgY = y + relY * scaleY;

            // Set font size and family
            QFont font("Arial", shape.specific.text.fontSize);
            painter.setFont(font);
            // Set the text color from the shape's color property
            painter.setPen(QPen(shape.color, shape.borderWidth));
            // Draw the text
            QString text = QString::fromUtf8(shape.specific.text.data, shape.specific.text.length);
            painter.drawText(imgX, imgY, text);
        }
    }
}

// Add mouse event handlers to DragDropImageLabel
void DragDropImageLabel::mousePressEvent(QMouseEvent *event) {
    if (originalPixmap.isNull())
        return;

    int oldSelection = selectedShapeIndex;
    bool clickedOnShape = false; // Declare the variable here

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    // Check if click is within image area
    QRect imageRect(x, y, scaledSize.width(), scaledSize.height());

    // Only process left mouse button events for dragging
    if (imageRect.contains(event->pos()) && event->button() == Qt::LeftButton) {
        double scaleX = (double) scaledSize.width() / originalPixmap.width();
        double scaleY = (double) scaledSize.height() / originalPixmap.height();

        for (int i = shapes.size() - 1; i >= 0; i--) {
            const auto &shape = shapes[i];
            if (!visibleLayers.contains(shape.layer)) {
                continue;
            }
            bool isHit = false;
            if (shape.type == "圆形") {
                // Convert center point to screen coordinates
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Convert radius to screen coordinates
                double radiusRel = (double) shape.specific.circle.radius / 1920 * originalPixmap.width();
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
            } else if (shape.type == "矩形") {
                // Convert rectangle coordinates to screen coordinates
                double startRelX = (double) shape.x / 1920 * originalPixmap.width();
                double startRelY = (double) shape.y / 1080 * originalPixmap.height();
                double endRelX = (double) shape.specific.rect.endX / 1920 * originalPixmap.width();
                double endRelY = (double) shape.specific.rect.endY / 1080 * originalPixmap.height();

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
            } else if (shape.type == "直线") {
                // Convert line coordinates to screen coordinates
                double startRelX = (double) shape.x / 1920 * originalPixmap.width();
                double startRelY = (double) shape.y / 1080 * originalPixmap.height();
                double endRelX = (double) shape.specific.line.endX / 1920 * originalPixmap.width();
                double endRelY = (double) shape.specific.line.endY / 1080 * originalPixmap.height();

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
            } else if (shape.type == "椭圆") {
                // Convert center point to screen coordinates
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Convert radii to screen coordinates
                double radiusXRel = (double) shape.specific.ellipse.radiusX / 1920 * originalPixmap.width();
                double radiusYRel = (double) shape.specific.ellipse.radiusY / 1080 * originalPixmap.height();
                int radiusXScreen = radiusXRel * scaleX;
                int radiusYScreen = radiusYRel * scaleY;

                // Calculate position relative to ellipse center
                int relClickX = event->pos().x() - imgX;
                int relClickY = event->pos().y() - imgY;

                // Calculate the normalized distance from click to ellipse boundary
                double normalizedDistSquared =
                        pow(relClickX / (double) radiusXScreen, 2) +
                        pow(relClickY / (double) radiusYScreen, 2);
                double normalizedDist = sqrt(normalizedDistSquared);

                // Selection tolerance - set a minimum pixel width of 5 or the border width
                double pixelTolerance = qMax(5.0, (double) shape.borderWidth);
                double toleranceNormalized = pixelTolerance / qMin(radiusXScreen, radiusYScreen);

                // Check if click is near the ellipse boundary
                bool nearEllipse = (fabs(normalizedDist - 1.0) <= toleranceNormalized);

                // Debug output
                // qDebug() << "Ellipse click: normalizedDist=" << normalizedDist
                //          << " tolerance=" << toleranceNormalized
                //          << " nearEllipse=" << nearEllipse;

                // Make ellipse selectable by the entire boundary
                if (nearEllipse) {
                    selectedShapeIndex = i;
                    dragMode = DRAG_SHAPE;

                    // Store click position relative to center for dragging
                    dragStartPos = QPoint(
                        relClickX / scaleX * 1920 / originalPixmap.width(),
                        relClickY / scaleY * 1080 / originalPixmap.height()
                    );

                    emit selectionChanged();
                    update();
                    break;
                }
            } else if (shape.type == "圆弧") {
                // Convert center point to screen coordinates
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Convert radii to screen coordinates
                double radiusXRel = (double) shape.specific.arc.radiusX / 1920 * originalPixmap.width();
                double radiusYRel = (double) shape.specific.arc.radiusY / 1080 * originalPixmap.height();
                int radiusXScreen = radiusXRel * scaleX;
                int radiusYScreen = radiusYRel * scaleY;

                // Calculate position relative to arc center
                int relClickX = event->pos().x() - imgX;
                int relClickY = event->pos().y() - imgY;

                // Calculate the normalized distance from click to ellipse boundary
                double normalizedDistSquared =
                        pow(relClickX / (double) radiusXScreen, 2) +
                        pow(relClickY / (double) radiusYScreen, 2);
                double normalizedDist = sqrt(normalizedDistSquared);

                // Calculate angle in degrees (0-360, 0° at 12 o'clock)
                double angle = atan2(relClickY, relClickX) * 180.0 / M_PI;
                // Convert from atan2 (where 0° is at 3 o'clock) to 12 o'clock system
                double angleDeg = 90 - angle;
                if (angleDeg < 0) angleDeg += 360.0;

                // Get arc parameters
                int startAngle = shape.specific.arc.startAngle;
                int spanAngle = shape.specific.arc.spanAngle;
                int endAngle = (startAngle + spanAngle) % 360;
                if (endAngle < 0) endAngle += 360;

                // Selection tolerance - set a minimum pixel width of 5 or the border width
                double pixelTolerance = qMax(5.0, (double) shape.borderWidth);
                double toleranceNormalized = pixelTolerance / qMin(radiusXScreen, radiusYScreen);

                // Check if click is near the arc path
                bool nearArc = (fabs(normalizedDist - 1.0) <= toleranceNormalized);

                // For debugging only
                bool inAngleRange = false;

                if (spanAngle > 0) {
                    // Clockwise arc
                    if (startAngle <= endAngle) {
                        inAngleRange = (angleDeg >= startAngle && angleDeg <= endAngle);
                    } else {
                        inAngleRange = (angleDeg >= startAngle || angleDeg <= endAngle);
                    }
                } else {
                    // Handle counter-clockwise case
                    if (endAngle <= startAngle) {
                        inAngleRange = (angleDeg <= startAngle && angleDeg >= endAngle);
                    } else {
                        inAngleRange = (angleDeg <= startAngle || angleDeg >= endAngle);
                    }
                }

                // Debug output
                // qDebug() << "Arc click: angle=" << angleDeg << " start=" << startAngle
                //          << " end=" << endAngle
                //          << " inRange=" << inAngleRange
                //          << " normalizedDist=" << normalizedDist
                //          << " tolerance=" << toleranceNormalized
                //          << " nearArc=" << nearArc;

                // THE FIX IS HERE: Make arc selectable by the entire border, not just in the angle range
                if (nearArc) {
                    selectedShapeIndex = i;
                    dragMode = DRAG_SHAPE;

                    // Store click position relative to center for dragging
                    dragStartPos = QPoint(
                        relClickX / scaleX * 1920 / originalPixmap.width(),
                        relClickY / scaleY * 1080 / originalPixmap.height()
                    );

                    emit selectionChanged();
                    update();
                    break;
                }
            } else if (shape.type == "整数") {
                // Calculate position for testing
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Create a hit area around the text
                QFont font("Arial", shape.specific.intValue.fontSize);
                QFontMetrics metrics(font);
                QString text = QString::number(shape.specific.intValue.value);
                QRect textRect = metrics.boundingRect(text);
                textRect.moveTo(imgX, imgY - metrics.ascent());

                // Add some padding for easier selection
                textRect.adjust(-5, -5, 5, 5);

                if (textRect.contains(event->pos())) {
                    isHit = true;
                }
            } else if (shape.type == "浮点数") {
                // Calculate position for testing
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Create a hit area around the text
                QFont font("Arial", shape.specific.floatValue.fontSize);
                QFontMetrics metrics(font);
                double displayValue = shape.specific.floatValue.value / 1000.0;
                QString text = QString::number(displayValue, 'f', 3);
                QRect textRect = metrics.boundingRect(text);
                textRect.moveTo(imgX, imgY - metrics.ascent());

                // Add some padding for easier selection
                textRect.adjust(-5, -5, 5, 5);

                if (textRect.contains(event->pos())) {
                    isHit = true;
                }
            } else if (shape.type == "文本字符") {
                // Calculate position for testing
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Create a hit area around the text
                QFont font("Arial", shape.specific.text.fontSize);
                QFontMetrics metrics(font);
                QString text = QString::fromUtf8(shape.specific.text.data, shape.specific.text.length);
                QRect textRect = metrics.boundingRect(text);
                textRect.moveTo(imgX, imgY - metrics.ascent());

                // Add some padding for easier selection
                textRect.adjust(-5, -5, 5, 5);

                if (textRect.contains(event->pos())) {
                    isHit = true;
                }
            }

            if (isHit) {
                selectedShapeIndex = i;
                dragMode = DRAG_SHAPE;

                // Store the offset between mouse position and shape position
                double relX = (double) shape.x / 1920 * originalPixmap.width();
                double relY = (double) shape.y / 1080 * originalPixmap.height();
                int imgX = x + relX * scaleX;
                int imgY = y + relY * scaleY;

                // Calculate and store the offset
                dragStartPos = QPoint(event->pos().x() - imgX, event->pos().y() - imgY);

                clickedOnShape = true;
                break;
            }
        }
    } else {
        // If right button or outside image, just update selection state
        // But don't start drag operation
        if (event->button() != Qt::LeftButton) {
            selectedShapeIndex = -1;
            dragMode = DRAG_NONE;
        }
    }
    update();
    if (oldSelection != selectedShapeIndex) {
        emit selectionChanged();
    }
}

void DragDropImageLabel::mouseMoveEvent(QMouseEvent *event) {
    if (dragMode == DRAG_NONE || !(event->buttons() & Qt::LeftButton))
        return;

    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(width(), height(), Qt::KeepAspectRatio);
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    double scaleX = (double) originalPixmap.width() / scaledSize.width();
    double scaleY = (double) originalPixmap.height() / scaledSize.height();

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
    } else if (dragMode == DRAG_RECTANGLE_CORNER && shapes[selectedShapeIndex].type == "矩形") {
        // Convert mouse position to absolute coordinates (0-1920, 0-1080)
        int mouseX = qRound((event->pos().x() - x) * scaleX * 1920 / originalPixmap.width());
        int mouseY = qRound((event->pos().y() - y) * scaleY * 1080 / originalPixmap.height());

        // Constrain to image bounds
        mouseX = qBound(0, mouseX, 1920);
        mouseY = qBound(0, mouseY, 1080);

        // Update the appropriate corner based on which one is being dragged
        if (dragCorner == 0) {
            // Top-left
            shapes[selectedShapeIndex].x = mouseX;
            shapes[selectedShapeIndex].y = mouseY;
        } else if (dragCorner == 1) {
            // Top-right
            shapes[selectedShapeIndex].specific.rect.endX = mouseX;
            shapes[selectedShapeIndex].y = mouseY;
        } else if (dragCorner == 2) {
            // Bottom-left
            shapes[selectedShapeIndex].x = mouseX;
            shapes[selectedShapeIndex].specific.rect.endY = mouseY;
        } else if (dragCorner == 3) {
            // Bottom-right
            shapes[selectedShapeIndex].specific.rect.endX = mouseX;
            shapes[selectedShapeIndex].specific.rect.endY = mouseY;
        }

        update();
        emit selectionChanged();
    } else if (dragMode == DRAG_RECTANGLE_CORNER && shapes[selectedShapeIndex].type == "直线") {
        // Convert mouse position to absolute coordinates (0-1920, 0-1080)
        int mouseX = qRound((event->pos().x() - x) * scaleX * 1920 / originalPixmap.width());
        int mouseY = qRound((event->pos().y() - y) * scaleY * 1080 / originalPixmap.height());

        // Constrain to image bounds
        mouseX = qBound(0, mouseX, 1920);
        mouseY = qBound(0, mouseY, 1080);

        if (dragCorner == 0) {
            // First endpoint (start point)
            shapes[selectedShapeIndex].x = mouseX;
            shapes[selectedShapeIndex].y = mouseY;
        } else if (dragCorner == 1) {
            // Second endpoint (end point)
            shapes[selectedShapeIndex].specific.line.endX = mouseX;
            shapes[selectedShapeIndex].specific.line.endY = mouseY;
        }

        update();
        emit selectionChanged(); // This line is critical - it notifies the UI to update
    }
}

bool DragDropImageLabel::hasSelectedShape() const {
    return selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size();
}

void DragDropImageLabel::mouseReleaseEvent(QMouseEvent *event) {
    // Only handle release for left button operations
    if (event->button() == Qt::LeftButton) {
        dragMode = DRAG_NONE;
        dragCorner = -1;
        update();
    }
}

void DragDropImageLabel::setShapePosition(int x, int y) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].x = qBound(0, x, 1920);
        shapes[selectedShapeIndex].y = qBound(0, y, 1080);
        update();
    }
}


qreal DragDropImageLabel::getSelectedShapeSize() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].sizePercent;
    }
    return 0.05; // Default size if no shape selected
}

void DragDropImageLabel::setShapeSize(qreal size) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].sizePercent = size;
        update();
    }
}

// Methods to manipulate selected shape
void DragDropImageLabel::deleteSelectedShape() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes.removeAt(selectedShapeIndex);
        selectedShapeIndex = -1; // Deselect after deletion
        update();
    }
}

void DragDropImageLabel::setShapeColor(const QColor &color) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].color = color;
        update();
    }
}

void DragDropImageLabel::setBorderWidth(int width) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].borderWidth = width;
        update();
    }
}

// Implementation in mainwindow.cpp
QColor DragDropImageLabel::getSelectedShapeColor() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].color;
    }
    return Qt::black; // Default color if no shape selected
}

int DragDropImageLabel::getSelectedShapeBorderWidth() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].borderWidth;
    }
    return 2; // Default width if no shape selected
}

void DragDropImageLabel::setShapeLayer(int layer) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        shapes[selectedShapeIndex].layer = qBound(0, layer, 9);
        update();
    }
}

int DragDropImageLabel::getSelectedShapeLayer() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].layer;
    }
    return 5; // Default layer if no shape selected
}

QSet<int> DragDropImageLabel::getVisibleLayers() const {
    return visibleLayers;
}

void DragDropImageLabel::setVisibleLayers(const QSet<int> &layers) {
    visibleLayers = layers;
    update(); // Redraw with the new visibility settings
}

QString DragDropImageLabel::getSelectedShapeType() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        return shapes[selectedShapeIndex].type;
    }
    return QString(); // Return empty string if no shape is selected
}

int DragDropImageLabel::getSelectedShapeEndX() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        const Shape &shape = shapes[selectedShapeIndex];
        if (shape.type == "矩形") {
            return shape.specific.rect.endX;
        } else if (shape.type == "直线") {
            return shape.specific.line.endX;
        }
    }
    return 0;
}

int DragDropImageLabel::getSelectedShapeEndY() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size()) {
        const Shape &shape = shapes[selectedShapeIndex];
        if (shape.type == "矩形") {
            return shape.specific.rect.endY;
        } else if (shape.type == "直线") {
            return shape.specific.line.endY;
        }
    }
    return 0;
}
int DragDropImageLabel::getSelectedShapeRadius() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆形") {
        return shapes[selectedShapeIndex].specific.circle.radius;
    }
    return 50; // Default radius if no circle is selected
}

void DragDropImageLabel::setRectangleEndPoint(int x, int y) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "矩形") {
        shapes[selectedShapeIndex].specific.rect.endX = qBound(0, x, 1920);
        shapes[selectedShapeIndex].specific.rect.endY = qBound(0, y, 1080);
        update();
    }
}

void DragDropImageLabel::setCircleRadius(int radius) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆形") {
        shapes[selectedShapeIndex].specific.circle.radius = qBound(1, radius, 500);
        update();
    }
}

void DragDropImageLabel::setLineEndPoint(int x, int y) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "直线") {
        shapes[selectedShapeIndex].specific.line.endX = x;
        shapes[selectedShapeIndex].specific.line.endY = y;
        update();
    }
}

void DragDropImageLabel::setEllipseRadius(int radiusX, int radiusY) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "椭圆") {
        shapes[selectedShapeIndex].specific.ellipse.radiusX = qBound(1, radiusX, 500);
        shapes[selectedShapeIndex].specific.ellipse.radiusY = qBound(1, radiusY, 500);
        update();
    }
}

int DragDropImageLabel::getSelectedShapeRadiusX() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "椭圆") {
        return shapes[selectedShapeIndex].specific.ellipse.radiusX;
    }
    return 50; // Default X radius if no ellipse is selected
}

int DragDropImageLabel::getSelectedShapeRadiusY() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "椭圆") {
        return shapes[selectedShapeIndex].specific.ellipse.radiusY;
    }
    return 30; // Default Y radius if no ellipse is selected
}

void DragDropImageLabel::setArcProperties(int radiusX, int radiusY, int startAngle, int spanAngle) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆弧") {
        shapes[selectedShapeIndex].specific.arc.radiusX = qBound(1, radiusX, 500);
        shapes[selectedShapeIndex].specific.arc.radiusY = qBound(1, radiusY, 500);
        shapes[selectedShapeIndex].specific.arc.startAngle = qBound(0, startAngle, 359);
        shapes[selectedShapeIndex].specific.arc.spanAngle = qBound(-359, spanAngle, 359);
        update();
    }
}

int DragDropImageLabel::getSelectedShapeArcRadiusX() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆弧") {
        return shapes[selectedShapeIndex].specific.arc.radiusX;
    }
    return 50;
}

int DragDropImageLabel::getSelectedShapeArcRadiusY() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆弧") {
        return shapes[selectedShapeIndex].specific.arc.radiusY;
    }
    return 30;
}

int DragDropImageLabel::getSelectedShapeArcStartAngle() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆弧") {
        return shapes[selectedShapeIndex].specific.arc.startAngle;
    }
    return 0;
}

int DragDropImageLabel::getSelectedShapeArcSpanAngle() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "圆弧") {
        return shapes[selectedShapeIndex].specific.arc.spanAngle;
    }
    return 90;
}

int32_t DragDropImageLabel::getSelectedShapeIntValue() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "整数") {
        return shapes[selectedShapeIndex].specific.intValue.value;
    }
    return 0; // Default value if no int shape is selected
}

int DragDropImageLabel::getSelectedShapeIntFontSize() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "整数") {
        return shapes[selectedShapeIndex].specific.intValue.fontSize;
    }
    return 12; // Default font size if no int shape is selected
}

void DragDropImageLabel::setIntValue(int32_t value) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "整数") {
        shapes[selectedShapeIndex].specific.intValue.value = value;
        update();
    }
}

void DragDropImageLabel::setIntFontSize(int fontSize) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "整数") {
        shapes[selectedShapeIndex].specific.intValue.fontSize = qBound(8, fontSize, 72);
        update();
    }
}


int32_t DragDropImageLabel::getSelectedShapeFloatValue() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "浮点数") {
        return shapes[selectedShapeIndex].specific.floatValue.value;
    }
    return 0; // Default value if no float shape is selected
}

int DragDropImageLabel::getSelectedShapeFloatFontSize() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "浮点数") {
        return shapes[selectedShapeIndex].specific.floatValue.fontSize;
    }
    return 12; // Default font size if no float shape is selected
}

void DragDropImageLabel::setFloatValue(int32_t value) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "浮点数") {
        shapes[selectedShapeIndex].specific.floatValue.value = value;
        update();
    }
}

void DragDropImageLabel::setFloatFontSize(int fontSize) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "浮点数") {
        shapes[selectedShapeIndex].specific.floatValue.fontSize = qBound(8, fontSize, 72);
        update();
    }
}

QString DragDropImageLabel::getSelectedShapeText() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "文本字符") {
        return QString::fromUtf8(shapes[selectedShapeIndex].specific.text.data,
                                 shapes[selectedShapeIndex].specific.text.length);
    }
    return QString(); // Empty string if no text shape is selected
}

int DragDropImageLabel::getSelectedShapeTextLength() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "文本字符") {
        return shapes[selectedShapeIndex].specific.text.length;
    }
    return 0; // Default length if no text shape is selected
}

int DragDropImageLabel::getSelectedShapeTextFontSize() const {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "文本字符") {
        return shapes[selectedShapeIndex].specific.text.fontSize;
    }
    return 12; // Default font size if no text shape is selected
}

void DragDropImageLabel::setText(const QString &text) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "文本字符") {
        // Ensure we don't exceed the array size
        QByteArray utf8Data = text.toUtf8();
        int length = qMin(utf8Data.length(), 29); // Max 29 chars to leave room for null terminator

        // Copy the string to the data array
        memcpy(shapes[selectedShapeIndex].specific.text.data, utf8Data.constData(), length);
        shapes[selectedShapeIndex].specific.text.data[length] = '\0'; // Null-terminate
        shapes[selectedShapeIndex].specific.text.length = length;

        update();
    }
}

void DragDropImageLabel::setTextFontSize(int fontSize) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size() &&
        shapes[selectedShapeIndex].type == "文本字符") {
        shapes[selectedShapeIndex].specific.text.fontSize = qBound(8, fontSize, 72);
        update();
    }
}

// ShapeListItem 实现
ShapeListItem::ShapeListItem(const QString &text, QListWidget *parent)
    : QListWidgetItem(text, parent) {
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}


void MainWindow::onShapeSelectionChanged() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
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
    } else if (shapeType == "圆形") {
        // Set up circle-specific controls
        circleRadiusSpinBox->blockSignals(true);
        circleRadiusSpinBox->setValue(imageLabel->getSelectedShapeRadius());
        circleRadiusSpinBox->blockSignals(false);

        // Show circle properties panel
        shapeSpecificControls->setCurrentWidget(circlePropertiesWidget);
    }
}

void MainWindow::changeRectangleEndPoint(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "矩形") {
        imageLabel->setRectangleEndPoint(
            rectEndXSpinBox->value(),
            rectEndYSpinBox->value()
        );
    }
}

void MainWindow::changeCircleRadius(int radius) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "圆形") {
        imageLabel->setCircleRadius(radius);
    }
}

void MainWindow::changeLineEndPoint(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "直线") {
        imageLabel->setLineEndPoint(lineEndXSpinBox->value(), lineEndYSpinBox->value());
    }
}

void MainWindow::changeEllipseRadius(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "椭圆") {
        imageLabel->setEllipseRadius(
            ellipseRadiusXSpinBox->value(),
            ellipseRadiusYSpinBox->value()
        );
    }
}

void MainWindow::changeArcProperties(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "圆弧") {
        imageLabel->setArcProperties(
            arcRadiusXSpinBox->value(),
            arcRadiusYSpinBox->value(),
            arcStartAngleSpinBox->value(),
            arcSpanAngleSpinBox->value()
        );
    }
}


void MainWindow::changeIntProperties(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "整数") {
        imageLabel->setIntValue(intValueSpinBox->value());
        imageLabel->setIntFontSize(intFontSizeSpinBox->value());
    }
}

void MainWindow::changeFloatProperties(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "浮点数") {
        imageLabel->setFloatValue(floatValueSpinBox->value());
        imageLabel->setFloatFontSize(floatFontSizeSpinBox->value());
    }
}

void MainWindow::changeTextProperties() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape() &&
        imageLabel->getSelectedShapeType() == "文本字符") {
        imageLabel->setText(textLineEdit->text());
        imageLabel->setTextFontSize(textFontSizeSpinBox->value());
    }
}

void MainWindow::saveShapesToFile() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (!imageLabel) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("保存形状数据"),
                                                   QString(), tr("形状数据文件 (*.shapes)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("无法保存文件"),
                           tr("无法打开文件 %1 进行写入。\n%2.")
                           .arg(fileName, file.errorString()));
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);

    // Write number of shapes
    out << imageLabel->shapes.size();

    // Write shapes data
    for (const auto &shape : imageLabel->shapes) {
        out << shape.type;
        out << shape.x;
        out << shape.y;
        out << shape.sizePercent;
        out << shape.color;
        out << shape.borderWidth;
        out << shape.layer;

        // Write shape-specific data based on type
        if (shape.type == "圆形") {
            out << shape.specific.circle.radius;
        } else if (shape.type == "矩形") {
            out << shape.specific.rect.endX;
            out << shape.specific.rect.endY;
        } else if (shape.type == "直线") {
            out << shape.specific.line.endX;
            out << shape.specific.line.endY;
        } else if (shape.type == "椭圆") {
            out << shape.specific.ellipse.radiusX;
            out << shape.specific.ellipse.radiusY;
        } else if (shape.type == "圆弧") {
            out << shape.specific.arc.radiusX;
            out << shape.specific.arc.radiusY;
            out << shape.specific.arc.startAngle;
            out << shape.specific.arc.spanAngle;
        } else if (shape.type == "整数") {
            out << shape.specific.intValue.value;
            out << shape.specific.intValue.fontSize;
        } else if (shape.type == "浮点数") {
            out << shape.specific.floatValue.value;
            out << shape.specific.floatValue.fontSize;
        } else if (shape.type == "文本字符") {
            QString text = QString::fromUtf8(shape.specific.text.data, shape.specific.text.length);
            out << text;
            out << shape.specific.text.fontSize;
        }
    }

    file.close();
    QMessageBox::information(this, tr("保存成功"), tr("形状数据已成功保存到文件。"));
}

void MainWindow::loadShapesFromFile() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (!imageLabel) return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("加载形状数据"),
                                                  QString(), tr("形状数据文件 (*.shapes)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("无法加载文件"),
                           tr("无法打开文件 %1 进行读取。\n%2.")
                           .arg(fileName, file.errorString()));
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);

    // Clear existing shapes
    imageLabel->shapes.clear();

    // Read number of shapes
    int shapeCount;
    in >> shapeCount;

    // Read shapes data
    for (int i = 0; i < shapeCount; i++) {
        DragDropImageLabel::Shape shape;

        in >> shape.type;
        in >> shape.x;
        in >> shape.y;
        in >> shape.sizePercent;
        in >> shape.color;
        in >> shape.borderWidth;
        in >> shape.layer;

        // Read shape-specific data based on type
        if (shape.type == "圆形") {
            in >> shape.specific.circle.radius;
        } else if (shape.type == "矩形") {
            in >> shape.specific.rect.endX;
            in >> shape.specific.rect.endY;
        } else if (shape.type == "直线") {
            in >> shape.specific.line.endX;
            in >> shape.specific.line.endY;
        } else if (shape.type == "椭圆") {
            in >> shape.specific.ellipse.radiusX;
            in >> shape.specific.ellipse.radiusY;
        } else if (shape.type == "圆弧") {
            in >> shape.specific.arc.radiusX;
            in >> shape.specific.arc.radiusY;
            in >> shape.specific.arc.startAngle;
            in >> shape.specific.arc.spanAngle;
        } else if (shape.type == "整数") {
            in >> shape.specific.intValue.value;
            in >> shape.specific.intValue.fontSize;
        } else if (shape.type == "浮点数") {
            in >> shape.specific.floatValue.value;
            in >> shape.specific.floatValue.fontSize;
        } else if (shape.type == "文本字符") {
            QString text;
            in >> text;
            in >> shape.specific.text.fontSize;

            // Copy text to the shape data structure
            QByteArray textBytes = text.toUtf8();
            int length = qMin(textBytes.size(), 29); // Ensure it fits in our 30-char buffer
            memcpy(shape.specific.text.data, textBytes.constData(), length);
            shape.specific.text.data[length] = '\0'; // Null-terminate
            shape.specific.text.length = length;
        }

        imageLabel->shapes.append(shape);
    }

    file.close();
    imageLabel->update(); // Redraw with new shapes
    QMessageBox::information(this, tr("加载成功"), tr("形状数据已成功从文件加载。"));
}

void MainWindow::exportControlsInfo() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (!imageLabel || imageLabel->shapes.isEmpty()) {
        QMessageBox::information(this, tr("没有控件"), tr("当前画布上没有控件信息可以导出。"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("导出控件信息"),
                                                  QString(), tr("文本文件 (*.txt)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("无法保存文件"),
                           tr("无法打开文件 %1 进行写入。\n%2.")
                           .arg(fileName, file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // Write header
    out << "控件信息表\n";
    out << "==========\n\n";
    out << "生成时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";

    // Summary information
    out << "控件总数：" << imageLabel->shapes.size() << "\n";

    // Count by type
    QMap<QString, int> typeCount;
    for (const auto &shape : imageLabel->shapes) {
        typeCount[shape.type]++;
    }

    out << "控件类型统计：\n";
    for (auto it = typeCount.constBegin(); it != typeCount.constEnd(); ++it) {
        out << "  " << it.key() << ": " << it.value() << " 个\n";
    }
    out << "\n";

    // Table header
    out << "控件详细信息\n";
    out << "============\n\n";
    out << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
           .arg("序号", 4)
           .arg("类型", 10)
           .arg("位置(X,Y)", 15)
           .arg("图层", 5)
           .arg("颜色", 10)
           .arg("边框宽度", 8)
           .arg("特定属性");

    out << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
           .arg("----", 4)
           .arg("----------", 10)
           .arg("---------------", 15)
           .arg("-----", 5)
           .arg("----------", 10)
           .arg("--------", 8)
           .arg("-------------------------------------------");

    // List all controls
    int index = 1;
    for (const auto &shape : imageLabel->shapes) {
        QString position = QString("(%1,%2)").arg(shape.x).arg(shape.y);
        QString colorName = getNameFromColor(shape.color);
        QString specificProps;

        if (shape.type == "圆形") {
            specificProps = QString("半径: %1").arg(shape.specific.circle.radius);
        } else if (shape.type == "矩形") {
            specificProps = QString("右下角: (%1,%2)")
                             .arg(shape.specific.rect.endX)
                             .arg(shape.specific.rect.endY);
        } else if (shape.type == "直线") {
            specificProps = QString("终点: (%1,%2)")
                             .arg(shape.specific.line.endX)
                             .arg(shape.specific.line.endY);
        } else if (shape.type == "椭圆") {
            specificProps = QString("半径X: %1, 半径Y: %2")
                             .arg(shape.specific.ellipse.radiusX)
                             .arg(shape.specific.ellipse.radiusY);
        } else if (shape.type == "圆弧") {
            specificProps = QString("半径X: %1, 半径Y: %2, 起始角度: %3°, 张角: %4°")
                             .arg(shape.specific.arc.radiusX)
                             .arg(shape.specific.arc.radiusY)
                             .arg(shape.specific.arc.startAngle)
                             .arg(shape.specific.arc.spanAngle);
        } else if (shape.type == "整数") {
            specificProps = QString("值: %1, 字体大小: %2")
                             .arg(shape.specific.intValue.value)
                             .arg(shape.specific.intValue.fontSize);
        } else if (shape.type == "浮点数") {
            double displayValue = shape.specific.floatValue.value / 1000.0;
            specificProps = QString("值: %1, 字体大小: %2")
                             .arg(displayValue, 0, 'f', 3)
                             .arg(shape.specific.floatValue.fontSize);
        } else if (shape.type == "文本字符") {
            QString text = QString::fromUtf8(shape.specific.text.data, shape.specific.text.length);
            specificProps = QString("文本: \"%1\", 字体大小: %2")
                             .arg(text)
                             .arg(shape.specific.text.fontSize);
        }

        out << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
               .arg(index, 4)
               .arg(shape.type, 10)
               .arg(position, 15)
               .arg(shape.layer, 5)
               .arg(colorName, 10)
               .arg(shape.borderWidth, 8)
               .arg(specificProps);

        index++;
    }

    file.close();
    QMessageBox::information(this, tr("导出成功"), tr("控件信息已成功导出到文件。"));
}


void ShapeListWidget::startDrag(Qt::DropActions supportedActions) {
    QListWidgetItem *item = currentItem();
    if (item) {
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(item->text());

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // Create a pixmap of the item for the drag cursor
        QPixmap pixmap = item->icon().pixmap(32, 32);
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));

        drag->exec(supportedActions, Qt::CopyAction);
    }
}

// This overrides the updatePropertyControls method to include shape-specific properties
void MainWindow::updatePropertyControls() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
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

    // Update color combo box
    colorComboBox->blockSignals(true);
    QString colorName = getNameFromColor(imageLabel->getSelectedShapeColor());
    int colorIndex = colorComboBox->findText(colorName);
    if (colorIndex != -1) {
        colorComboBox->setCurrentIndex(colorIndex);
    }
    colorComboBox->blockSignals(false);

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
    } else if (shapeType == "圆形") {
        // Set up circle-specific controls
        circleRadiusSpinBox->blockSignals(true);
        circleRadiusSpinBox->setValue(imageLabel->getSelectedShapeRadius());
        circleRadiusSpinBox->blockSignals(false);

        // Show circle properties panel
        shapeSpecificControls->setCurrentWidget(circlePropertiesWidget);
    } else if (shapeType == "直线") {
        // Set up line-specific controls
        lineEndXSpinBox->blockSignals(true);
        lineEndYSpinBox->blockSignals(true);

        lineEndXSpinBox->setValue(imageLabel->getSelectedShapeEndX());
        lineEndYSpinBox->setValue(imageLabel->getSelectedShapeEndY());

        lineEndXSpinBox->blockSignals(false);
        lineEndYSpinBox->blockSignals(false);

        // Show line properties panel
        shapeSpecificControls->setCurrentWidget(linePropertiesWidget);
    } else if (shapeType == "椭圆") {
        // Set up ellipse-specific controls
        ellipseRadiusXSpinBox->blockSignals(true);
        ellipseRadiusYSpinBox->blockSignals(true);

        ellipseRadiusXSpinBox->setValue(imageLabel->getSelectedShapeRadiusX());
        ellipseRadiusYSpinBox->setValue(imageLabel->getSelectedShapeRadiusY());

        ellipseRadiusXSpinBox->blockSignals(false);
        ellipseRadiusYSpinBox->blockSignals(false);

        // Show ellipse properties panel
        shapeSpecificControls->setCurrentWidget(ellipsePropertiesWidget);
    } else if (shapeType == "圆弧") {
        // Set up arc-specific controls
        arcRadiusXSpinBox->blockSignals(true);
        arcRadiusYSpinBox->blockSignals(true);
        arcStartAngleSpinBox->blockSignals(true);
        arcSpanAngleSpinBox->blockSignals(true);

        arcRadiusXSpinBox->setValue(imageLabel->getSelectedShapeArcRadiusX());
        arcRadiusYSpinBox->setValue(imageLabel->getSelectedShapeArcRadiusY());
        arcStartAngleSpinBox->setValue(imageLabel->getSelectedShapeArcStartAngle());
        arcSpanAngleSpinBox->setValue(imageLabel->getSelectedShapeArcSpanAngle());

        arcRadiusXSpinBox->blockSignals(false);
        arcRadiusYSpinBox->blockSignals(false);
        arcStartAngleSpinBox->blockSignals(false);
        arcSpanAngleSpinBox->blockSignals(false);

        // Show arc properties panel
        shapeSpecificControls->setCurrentWidget(arcPropertiesWidget);
    } else if (shapeType == "整数") {
        shapeSpecificControls->setCurrentWidget(intValuePropertiesWidget);

        intValueSpinBox->blockSignals(true);
        intFontSizeSpinBox->blockSignals(true);

        intValueSpinBox->setValue(imageLabel->getSelectedShapeIntValue());
        intFontSizeSpinBox->setValue(imageLabel->getSelectedShapeIntFontSize());

        intValueSpinBox->blockSignals(false);
        intFontSizeSpinBox->blockSignals(false);
    } else if (shapeType == "浮点数") {
        shapeSpecificControls->setCurrentWidget(floatValuePropertiesWidget);

        floatValueSpinBox->blockSignals(true);
        floatFontSizeSpinBox->blockSignals(true);

        floatValueSpinBox->setValue(imageLabel->getSelectedShapeFloatValue());
        floatFontSizeSpinBox->setValue(imageLabel->getSelectedShapeFloatFontSize());

        floatValueSpinBox->blockSignals(false);
        floatFontSizeSpinBox->blockSignals(false);
    }     else if (shapeType == "文本字符") {
        shapeSpecificControls->setCurrentWidget(textPropertiesWidget);

        textLineEdit->blockSignals(true);
        textFontSizeSpinBox->blockSignals(true);

        textLineEdit->setText(imageLabel->getSelectedShapeText());
        textFontSizeSpinBox->setValue(imageLabel->getSelectedShapeTextFontSize());

        textLineEdit->blockSignals(false);
        textFontSizeSpinBox->blockSignals(false);
    }
}
QColor MainWindow::getColorFromName(const QString &colorName) const {
    if (colorName == "红色") return Qt::red;
    if (colorName == "黄色") return Qt::yellow;
    if (colorName == "绿色") return Qt::green;
    if (colorName == "橙色") return QColor(255, 165, 0);
    if (colorName == "紫红色") return Qt::magenta;
    if (colorName == "粉色") return QColor(255, 192, 203);
    if (colorName == "青色") return Qt::cyan;
    if (colorName == "黑色") return Qt::black;
    if (colorName == "白色") return Qt::white;
    return Qt::black; // Default
}

QString MainWindow::getNameFromColor(const QColor &color) const {
    // Define some tolerance for color comparison
    auto isSimilarColor = [](const QColor &a, const QColor &b, int tolerance = 30) {
        return qAbs(a.red() - b.red()) <= tolerance &&
               qAbs(a.green() - b.green()) <= tolerance &&
               qAbs(a.blue() - b.blue()) <= tolerance;
    };

    if (isSimilarColor(color, Qt::red)) return "红色";
    if (isSimilarColor(color, Qt::yellow)) return "黄色";
    if (isSimilarColor(color, Qt::green)) return "绿色";
    if (isSimilarColor(color, QColor(255, 165, 0))) return "橙色";
    if (isSimilarColor(color, Qt::magenta)) return "紫红色";
    if (isSimilarColor(color, QColor(255, 192, 203))) return "粉色";
    if (isSimilarColor(color, Qt::cyan)) return "青色";
    if (isSimilarColor(color, Qt::black)) return "黑色";
    if (isSimilarColor(color, Qt::white)) return "白色";
    return "黑色"; // Default
}

QList<QString> MainWindow::getAvailableColors() const {
    return {"红色", "黄色", "绿色", "橙色", "紫红色", "粉色", "青色", "黑色", "白色"};
}
// Modified version of createShapeToolbar to include shape-specific property controls
void MainWindow::createShapeToolbar() {

    // Create toolbar for save/load functionality
    QToolBar *fileToolbar = addToolBar(tr("文件操作"));

    QAction *saveAction = fileToolbar->addAction(tr("保存"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveShapesToFile);

    QAction *loadAction = fileToolbar->addAction(tr("加载"));
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadShapesFromFile);

    QAction *exportInfoAction = fileToolbar->addAction(tr("导出控件信息"));
    connect(exportInfoAction, &QAction::triggered, this, &MainWindow::exportControlsInfo);

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

    // Add this to your createShapeToolbar() method after the line item
    auto *ellipseItem = new ShapeListItem(tr("椭圆"), shapesListWidget);
    ellipseItem->setIcon(QIcon(createShapeIcon("椭圆")));

    // Add this to your createShapeToolbar() method
    auto *arcItem = new ShapeListItem(tr("圆弧"), shapesListWidget);
    arcItem->setIcon(QIcon(createShapeIcon("圆弧")));

    auto *intValueItem = new ShapeListItem(tr("整数"), shapesListWidget);
    intValueItem->setIcon(QIcon(createShapeIcon("整数")));

    auto *floatValueItem = new ShapeListItem(tr("浮点数"), shapesListWidget);
    floatValueItem->setIcon(QIcon(createShapeIcon("浮点数")));

    auto *textItem = new ShapeListItem(tr("文本字符"), shapesListWidget);
    textItem->setIcon(QIcon(createShapeIcon("文本字符")));

    shapesDock->setWidget(shapesListWidget);
    addDockWidget(Qt::RightDockWidgetArea, shapesDock);

    // Create toolbar for common properties
    QToolBar *propertiesToolbar = addToolBar(tr("形状属性"));

    QAction *deleteAction = propertiesToolbar->addAction(tr("删除"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedShape);

    // QAction *colorAction = propertiesToolbar->addAction(tr("更改颜色"));
    // connect(colorAction, &QAction::triggered, this, &MainWindow::changeShapeColor);
    //
    // propertiesToolbar->addSeparator();

    // Create a widget to hold the color combo box
    QWidget *colorWidget = new QWidget();
    QHBoxLayout *colorLayout = new QHBoxLayout(colorWidget);
    colorLayout->setContentsMargins(5, 0, 5, 0);

    QLabel *colorLabel = new QLabel(tr("颜色:"));
    colorComboBox = new QComboBox();

    // Populate with available colors
    for (const QString &colorName : getAvailableColors()) {
        QPixmap colorBox(16, 16);
        colorBox.fill(getColorFromName(colorName));
        colorComboBox->addItem(QIcon(colorBox), colorName);
    }

    connect(colorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeShapeColor);

    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorComboBox);

    // Add the widget to the toolbar
    propertiesToolbar->addWidget(colorWidget);


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


    // Ellipse properties
    ellipsePropertiesWidget = new QWidget();
    QVBoxLayout *ellipseLayout = new QVBoxLayout(ellipsePropertiesWidget);

    QLabel *ellipseRadiusXLabel = new QLabel(tr("X轴半径:"));
    ellipseRadiusXSpinBox = new QSpinBox();
    ellipseRadiusXSpinBox->setRange(1, 500);
    ellipseRadiusXSpinBox->setSingleStep(1);
    connect(ellipseRadiusXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeEllipseRadius);

    QLabel *ellipseRadiusYLabel = new QLabel(tr("Y轴半径:"));
    ellipseRadiusYSpinBox = new QSpinBox();
    ellipseRadiusYSpinBox->setRange(1, 500);
    ellipseRadiusYSpinBox->setSingleStep(1);
    connect(ellipseRadiusYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeEllipseRadius);

    ellipseLayout->addWidget(ellipseRadiusXLabel);
    ellipseLayout->addWidget(ellipseRadiusXSpinBox);
    ellipseLayout->addWidget(ellipseRadiusYLabel);
    ellipseLayout->addWidget(ellipseRadiusYSpinBox);
    ellipseLayout->addStretch();


    // Arc properties widget
    arcPropertiesWidget = new QWidget();
    QVBoxLayout *arcLayout = new QVBoxLayout(arcPropertiesWidget);

    QLabel *arcRadiusXLabel = new QLabel(tr("X轴半径:"));
    arcRadiusXSpinBox = new QSpinBox();
    arcRadiusXSpinBox->setRange(1, 500);
    arcRadiusXSpinBox->setSingleStep(1);
    connect(arcRadiusXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeArcProperties);

    QLabel *arcRadiusYLabel = new QLabel(tr("Y轴半径:"));
    arcRadiusYSpinBox = new QSpinBox();
    arcRadiusYSpinBox->setRange(1, 500);
    arcRadiusYSpinBox->setSingleStep(1);
    connect(arcRadiusYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeArcProperties);

    QLabel *arcStartAngleLabel = new QLabel(tr("起始角度(°):"));
    arcStartAngleSpinBox = new QSpinBox();
    arcStartAngleSpinBox->setRange(0, 359);
    arcStartAngleSpinBox->setSingleStep(5);
    connect(arcStartAngleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeArcProperties);

    QLabel *arcSpanAngleLabel = new QLabel(tr("扫过角度(°):"));
    arcSpanAngleSpinBox = new QSpinBox();
    arcSpanAngleSpinBox->setRange(-359, 359);
    arcSpanAngleSpinBox->setSingleStep(5);
    connect(arcSpanAngleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeArcProperties);

    arcLayout->addWidget(arcRadiusXLabel);
    arcLayout->addWidget(arcRadiusXSpinBox);
    arcLayout->addWidget(arcRadiusYLabel);
    arcLayout->addWidget(arcRadiusYSpinBox);
    arcLayout->addWidget(arcStartAngleLabel);
    arcLayout->addWidget(arcStartAngleSpinBox);
    arcLayout->addWidget(arcSpanAngleLabel);
    arcLayout->addWidget(arcSpanAngleSpinBox);
    arcLayout->addStretch();


    // Create int value properties widget
    intValuePropertiesWidget = new QWidget();
    QVBoxLayout *intValueLayout = new QVBoxLayout(intValuePropertiesWidget);

    QLabel *intValueLabel = new QLabel(tr("整数值:"));
    intValueSpinBox = new QSpinBox();
    intValueSpinBox->setRange(INT_MIN, INT_MAX);
    intValueSpinBox->setSingleStep(1);
    connect(intValueSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeIntProperties);

    QLabel *intFontSizeLabel = new QLabel(tr("字体大小:"));
    intFontSizeSpinBox = new QSpinBox();
    intFontSizeSpinBox->setRange(8, 72);
    intFontSizeSpinBox->setSingleStep(1);
    connect(intFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeIntProperties);

    intValueLayout->addWidget(intValueLabel);
    intValueLayout->addWidget(intValueSpinBox);
    intValueLayout->addWidget(intFontSizeLabel);
    intValueLayout->addWidget(intFontSizeSpinBox);
    intValueLayout->addStretch();

    // Create float value properties widget
    floatValuePropertiesWidget = new QWidget();
    QVBoxLayout *floatValueLayout = new QVBoxLayout(floatValuePropertiesWidget);

    QLabel *floatValueLabel = new QLabel(tr("浮点数值 (x/1000):"));
    floatValueSpinBox = new QSpinBox();
    floatValueSpinBox->setRange(INT_MIN, INT_MAX);
    floatValueSpinBox->setSingleStep(1);
    connect(floatValueSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeFloatProperties);

    QLabel *floatFontSizeLabel = new QLabel(tr("字体大小:"));
    floatFontSizeSpinBox = new QSpinBox();
    floatFontSizeSpinBox->setRange(8, 72);
    floatFontSizeSpinBox->setSingleStep(1);
    connect(floatFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeFloatProperties);

    floatValueLayout->addWidget(floatValueLabel);
    floatValueLayout->addWidget(floatValueSpinBox);
    floatValueLayout->addWidget(floatFontSizeLabel);
    floatValueLayout->addWidget(floatFontSizeSpinBox);
    floatValueLayout->addStretch();

    // Create text properties widget
    textPropertiesWidget = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textPropertiesWidget);

    QLabel *textValueLabel = new QLabel(tr("文本内容:"));
    textLineEdit = new QLineEdit();
    textLineEdit->setMaxLength(30); // Limit to 30 characters
    connect(textLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::changeTextProperties);

    QLabel *textFontSizeLabel = new QLabel(tr("字体大小:"));
    textFontSizeSpinBox = new QSpinBox();
    textFontSizeSpinBox->setRange(8, 72);
    textFontSizeSpinBox->setSingleStep(1);
    connect(textFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeTextProperties);

    textLayout->addWidget(textValueLabel);
    textLayout->addWidget(textLineEdit);
    textLayout->addWidget(textFontSizeLabel);
    textLayout->addWidget(textFontSizeSpinBox);
    textLayout->addStretch();

    // Add to your stacked widget:
    shapeSpecificControls->addWidget(textPropertiesWidget);
    shapeSpecificControls->addWidget(floatValuePropertiesWidget);
    shapeSpecificControls->addWidget(intValuePropertiesWidget);
    shapeSpecificControls->addWidget(arcPropertiesWidget);
    shapeSpecificControls->addWidget(ellipsePropertiesWidget);
    shapeSpecificControls->addWidget(linePropertiesWidget);
    shapeSpecificControls->addWidget(rectanglePropertiesWidget);
    shapeSpecificControls->addWidget(circlePropertiesWidget);

    // Set the stacked widget as the dock widget's content
    specificPropDock->setWidget(shapeSpecificControls);
    addDockWidget(Qt::RightDockWidgetArea, specificPropDock);

    // Connect selection changes to update property controls
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    auto *imgLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
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
      , ui(new Ui::MainWindow) {
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

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionOpen_triggered() {
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

    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel) {
        imageLabel->setOriginalPixmap(newImage);
    }
}


// 创建形状图标的辅助函数
QPixmap MainWindow::createShapeIcon(const QString &shape) {
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
    } else if (shape == "椭圆") {
        painter.setPen(QPen(Qt::magenta, 2));
        painter.drawEllipse(5, 10, 30, 20); // Draw wider than tall for ellipse icon
    } else if (shape == "圆弧") {
        painter.setPen(QPen(Qt::cyan, 2));
        painter.drawArc(5, 5, 30, 30, 30 * 16, 120 * 16); // Draw a 120° arc starting at 30°
    } else if (shape == "整数") {
        painter.setPen(QPen(Qt::black, 2));
        painter.setFont(QFont("Arial", 12));
        painter.drawText(5, 25, "123");
    } else if (shape == "浮点数") {
        painter.setPen(QPen(Qt::black, 2));
        painter.setFont(QFont("Arial", 12));
        painter.drawText(5, 25, "1.234");
    } else if (shape == "文本字符") {
        painter.setPen(QPen(Qt::black, 2));
        painter.setFont(QFont("Arial", 12));
        painter.drawText(5, 25, "ABC");
    }

    return pixmap;
}

void MainWindow::onLayerVisibilityChanged(int layer) {
    // This method is called when a layer visibility checkbox is toggled
    QCheckBox *checkbox = qobject_cast<QCheckBox *>(sender());
    if (!checkbox) return;

    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
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

void MainWindow::createLayerPanel() {
    layersDock = new QDockWidget(tr("图层"), this);
    layersDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *layersWidget = new QWidget(layersDock);
    QVBoxLayout *layout = new QVBoxLayout(layersWidget);

    // Create checkboxes for each layer
    for (int i = 9; i >= 0; i--) {
        // 9 is top layer, 0 is bottom
        QCheckBox *layerCheckbox = new QCheckBox(tr("图层 %1").arg(i));
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


void MainWindow::changeShapeColor() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        QString colorName = colorComboBox->currentText();
        QColor color = getColorFromName(colorName);
        imageLabel->setShapeColor(color);
    }
}

void MainWindow::deleteSelectedShape() {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->deleteSelectedShape();
        updatePropertyControls(); // Update UI after deletion
    }
}

void MainWindow::changeBorderWidth(int width) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
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

void MainWindow::changeShapePosition(int) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->setShapePosition(xPosSpinBox->value(), yPosSpinBox->value());
    }
}

void MainWindow::changeShapeLayer(int layer) {
    auto *imageLabel = dynamic_cast<DragDropImageLabel *>(this->imageLabel);
    if (imageLabel && imageLabel->hasSelectedShape()) {
        imageLabel->setShapeLayer(layer);
    }
}
