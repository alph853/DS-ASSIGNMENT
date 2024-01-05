#ifndef CUSTOMGRAPHICS_H
#define CUSTOMGRAPHICS_H

#include <QTransform>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QScrollEvent>
#include <QDebug>
#include <QToolTip>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPushButton>


class customMap : public QGraphicsView
{
    Q_OBJECT

public:
    customMap(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};


class customPopupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit customPopupWidget(const QString &text, const QString& label, bool ra, QWidget *parent = nullptr);
    explicit customPopupWidget(const QString& text, const QString& label, QWidget* parent = nullptr);

protected:

private:
    QTextEdit * m_text;
    QLabel * m_label;
    bool ra;
};


class customEllipse : public QGraphicsEllipseItem
{
public:
    customEllipse(qreal x, qreal y, qreal width, qreal height, const QString& name, const QString& label,
                  bool ra, qreal sX, qreal sY,
                  QGraphicsItem *parent = nullptr );

    QString get_name() const { return m_name; }
    QPointF get_pos() const {return {m_x, m_y}; }
    QString get_label() const { return m_label; }
    void setSize(qreal x, qreal y)
    {
        setRect(m_x, m_y, x, y);
    }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:

private:
    QString m_name;
    customPopupWidget *m_popupWidget = nullptr;
    QString m_label;
    bool ra;
    qreal m_x, m_y;
};


class customScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit customScene(QObject *parent = nullptr);
    void set_startButton() { startPress = 1; }
    void set_endButton() { endPress = 1; }

    void reset_startButton() { startPress = 0; }
    void reset_endButton() { endPress = 0; }
    void addNodes(customEllipse* );
    void reserve(int n) { nodes.reserve(n); }
    std::vector<customEllipse*> get_nodes() { return nodes; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override; //emit signals to set_startText / set_endText

signals:
    void startClick(const QString &); //signals for mousePressEvent
    void endClick(const QString &);
private:
    bool startPress = 0;
    bool endPress = 0;
    std::vector<customEllipse* > nodes;
};


class customButton : public QPushButton
{
    Q_OBJECT
public:
    customButton(QWidget *parent = nullptr) : QPushButton(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void enterKeyPressed(customButton* );  // Custom signal to indicate Enter key press
};


class customPath : public QGraphicsPathItem
{
public:
    customPath(const QPainterPath& path, const QString& name, const QString& label)
        : QGraphicsPathItem(path), m_name(name), m_label(label)
    {
        setAcceptHoverEvents(true);
    }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    customPopupWidget* m_popupWidget = nullptr;
    QString m_name;
    QString m_label;
};
#endif // CUSTOMGRAPHICS_H
