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
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap originalPixmap;
    QList<QPair<QString, QRect>> shapes; // 存储形状及其位置
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

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QDockWidget *shapesDock;
    QListWidget *shapesListWidget;
    QPixmap createShapeIcon(const QString &shape);
};
#endif // MAINWINDOW_H
